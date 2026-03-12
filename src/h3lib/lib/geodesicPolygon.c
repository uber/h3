/*
 * Copyright 2025 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file geodesicPolygon.c
 * @brief Implements geodesic polygon conversions and spatial predicates.
 */

#include <math.h>

#include "alloc.h"
#include "constants.h"
#include "geodesicPolygonInternal.h"
#include "h3api.h"

/**
 * Determine whether two geodesic edges intersect on the unit sphere.
 *
 * The test classifies edge endpoints against each other's great-circle planes
 * to cull obvious misses, handles near-parallel arcs with a 1-D projection
 * fallback, and otherwise checks that the great-circle intersection lies
 * between both segments.
 */
static bool _geodesicEdgesCross(const Vec3d *a1, const Vec3d *a2,
                                const Vec3d *b1, const Vec3d *b2,
                                const Vec3d *normalB) {
    Vec3d normalA;
    vec3Cross(a1, a2, &normalA);

    const double b1Side = vec3Dot(&normalA, b1);
    const double b2Side = vec3Dot(&normalA, b2);
    const double a1Side = vec3Dot(normalB, a1);
    const double a2Side = vec3Dot(normalB, a2);

    // For coincident arcs (shared cell/polygon boundary edges), the triple
    // scalar products b1Side/b2Side are mathematically zero but ~1e-16 in FP.
    // When both errors share the same sign, their product is positive and
    // this early-out would incorrectly reject the overlap.  Use an epsilon
    // guard so near-zero products fall through to the collinear-overlap check.
    const double sideEps = EPSILON * EPSILON;
    if ((b1Side * b2Side > sideEps) || (a1Side * a2Side > sideEps)) {
        return false;
    }

    Vec3d intersectionLine;
    vec3Cross(&normalA, normalB, &intersectionLine);

    if (vec3MagSq(&intersectionLine) < EPSILON * EPSILON) {
        const Vec3d refDir = {a2->x - a1->x, a2->y - a1->y, a2->z - a1->z};
        if (vec3MagSq(&refDir) < EPSILON * EPSILON) {
            return false;
        }

        double projA1 = vec3Dot(a1, &refDir);
        double projA2 = vec3Dot(a2, &refDir);
        double projB1 = vec3Dot(b1, &refDir);
        double projB2 = vec3Dot(b2, &refDir);

        if (projA1 > projA2) {
            double tmp = projA1;
            projA1 = projA2;
            projA2 = tmp;
        }
        if (projB1 > projB2) {
            double tmp = projB1;
            projB1 = projB2;
            projB2 = tmp;
        }

        return (projA1 <= projB2) && (projB1 <= projA2);
    }

    const Vec3d midASum = {a1->x + a2->x, a1->y + a2->y, a1->z + a2->z};
    const Vec3d midBSum = {b1->x + b2->x, b1->y + b2->y, b1->z + b2->z};

    if (vec3MagSq(&midASum) < EPSILON * EPSILON ||
        vec3MagSq(&midBSum) < EPSILON * EPSILON) {
        return true;
    }

    double dotA = vec3Dot(&intersectionLine, &midASum);
    double dotB = vec3Dot(&intersectionLine, &midBSum);

    return (dotA * dotB >= -EPSILON);
}

/**
 * Test whether a point lies inside a geodesic loop using spherical winding.
 *
 * The algorithm walks the loop once, accumulating the signed angle subtended by
 * consecutive vertices as seen from the query point. When the total winding
 * exceeds pi in magnitude the point is inside; otherwise it is outside.
 *
 * A hemisphere-pole early rejection ensures that points on the far side of the
 * sphere (where the winding formula can give spurious +-2pi results) are
 * correctly classified as outside. This is safe because the polygon must fit
 * within a hemisphere; any point in the opposite hemisphere is guaranteed to be
 * outside.
 */
static bool _geodesicLoopContainsPoint(const GeodesicLoop *loop,
                                       const Vec3d *pointVec) {
    // Early rejection: if the loop has a known hemisphere pole and the point
    // lies in the opposite hemisphere, it cannot be inside the polygon.
    // This also prevents spurious winding-number results for antipodal points.
    if (loop->hasHemispherePole &&
        vec3Dot(&loop->hemispherePole, pointVec) < -1e-10) {
        return false;
    }

    double totalAngle = 0;
    double dotPV1 = vec3Dot(pointVec, &loop->edges[0].vert);

    for (int i = 0; i < loop->numEdges; i++) {
        Vec3d *vert2 = &loop->edges[(i + 1) % loop->numEdges].vert;
        double dotPV2 = vec3Dot(pointVec, vert2);

        double y = vec3Dot(pointVec, &loop->edges[i].edgeCross);
        double x = loop->edges[i].edgeDot - dotPV1 * dotPV2;

        totalAngle += atan2(y, x);

        dotPV1 = dotPV2;
    }

    return fabs(totalAngle) > M_PI;
}

/**
 * Quickly reject a sphere cap and AABB that cannot intersect.
 * Returns false ONLY when intersection is definitively impossible.
 */
static bool _geodesicSphereCapOverlapsAABB(const SphereCap *cap,
                                           const AABB *aabb) {
    // Cos comparisons require more accuracy - use bigger epsilon
    const double epsilon = 1e-8;

    // 1. Far point test - checks if the farthest AABB corner
    // falls outside the cap
    Vec3d farPoint;
    farPoint.x = cap->center.x >= 0 ? aabb->max.x : aabb->min.x;
    farPoint.y = cap->center.y >= 0 ? aabb->max.y : aabb->min.y;
    farPoint.z = cap->center.z >= 0 ? aabb->max.z : aabb->min.z;

    double farDot = vec3Dot(&farPoint, &cap->center);
    if (farDot < cap->cosRadius - epsilon) {
        return false;
    }

    // 2. Near origin test - checks if the closest AABB point
    // is outside the unit sphere
    Vec3d nearOrigin;
    nearOrigin.x = fmax(aabb->min.x, fmin(aabb->max.x, 0.0));
    nearOrigin.y = fmax(aabb->min.y, fmin(aabb->max.y, 0.0));
    nearOrigin.z = fmax(aabb->min.z, fmin(aabb->max.z, 0.0));

    double distSq = vec3Dot(&nearOrigin, &nearOrigin);
    if (distSq > 1.0 + epsilon) {
        return false;
    }

    return true;
}

static void _geodesicLoopToAABB(const GeodesicLoop *loop, AABB *out) {
    aabbEmptyInverted(out);

    for (int i = 0; i < loop->numEdges; i++) {
        aabbUpdateWithVec3d(out, &loop->edges[i].vert);

        aabbUpdateWithArcExtrema(out, &loop->edges[i].vert,
                                 &loop->edges[(i + 1) % loop->numEdges].vert,
                                 &loop->edges[i].edgeCross);
    }

    // Probe cardinal axes and expand the box if needed
    Vec3d testVecs[] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                        {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};
    for (int i = 0; i < 6; i++) {
        if (_geodesicLoopContainsPoint(loop, &testVecs[i])) {
            aabbUpdateWithVec3d(out, &testVecs[i]);
        }
    }
}

static bool _candidateContainsVerts(const Vec3d *candidate,
                                    const GeodesicEdge *edges, int numEdges) {
    if (vec3MagSq(candidate) < EPSILON * EPSILON) {
        return false;
    }

    Vec3d normalized = *candidate;
    vec3Normalize(&normalized);

    const double hemisphereEpsilon = -1e-12;
    for (int i = 0; i < numEdges; i++) {
        if (vec3Dot(&normalized, &edges[i].vert) < hemisphereEpsilon) {
            return false;
        }
    }
    return true;
}

/**
 * Test whether all loop vertices fit within some closed hemisphere.
 *
 * A hemisphere can be described by a pole vector c such that dot(c, v) >= 0
 * for every vertex v in the loop.
 *
 * This checks a deterministic set of candidate poles and returns true once
 * one contains all vertices:
 * - The vertex centroid
 * - Each vertex
 * - cross(v_i, v_j) and its negation for each vertex pair
 *
 * In the common case, the centroid check succeeds in O(n). If not, the
 * fallback pairwise search is more expensive (O(n^3) worst case).
 */
static bool _geoLoopVerticesFitHemisphere(const GeodesicEdge *edges,
                                          int numEdges, Vec3d *poleOut) {
    Vec3d centroid = {0};
    for (int i = 0; i < numEdges; i++) {
        centroid.x += edges[i].vert.x;
        centroid.y += edges[i].vert.y;
        centroid.z += edges[i].vert.z;
    }

    // Near-zero centroid direction is numerically inconclusive, so skip it.
    // More tight epsilon check for more conclusive result
    if (vec3MagSq(&centroid) >= 1e-12) {
        if (_candidateContainsVerts(&centroid, edges, numEdges)) {
            if (poleOut) {
                *poleOut = centroid;
                vec3Normalize(poleOut);
            }
            return true;
        }
    }

    for (int i = 0; i < numEdges; i++) {
        if (_candidateContainsVerts(&edges[i].vert, edges, numEdges)) {
            if (poleOut) {
                *poleOut = edges[i].vert;
            }
            return true;
        }
    }

    for (int i = 0; i < numEdges; i++) {
        for (int j = i + 1; j < numEdges; j++) {
            Vec3d candidate;
            vec3Cross(&edges[i].vert, &edges[j].vert, &candidate);

            if (_candidateContainsVerts(&candidate, edges, numEdges)) {
                if (poleOut) {
                    *poleOut = candidate;
                    vec3Normalize(poleOut);
                }
                return true;
            }

            Vec3d opposite = {
                .x = -candidate.x, .y = -candidate.y, .z = -candidate.z};
            if (_candidateContainsVerts(&opposite, edges, numEdges)) {
                if (poleOut) {
                    *poleOut = opposite;
                    vec3Normalize(poleOut);
                }
                return true;
            }
        }
    }

    return false;
}

/**
 * Test whether all loop vertices lie on a single great circle.
 */
static bool _geoLoopVerticesOnGreatCircle(const GeodesicEdge *edges,
                                          int numEdges) {
    // Keep legacy handling for point/line/triangle degeneracies and only
    // reject larger boundary-only loops that produce ambiguous containment.
    if (numEdges < 4) {
        return false;
    }

    Vec3d normal = {0};
    bool foundNormal = false;
    for (int i = 0; i < numEdges && !foundNormal; i++) {
        for (int j = i + 1; j < numEdges; j++) {
            vec3Cross(&edges[i].vert, &edges[j].vert, &normal);
            if (vec3MagSq(&normal) > 1e-12) {
                foundNormal = true;
                break;
            }
        }
    }

    if (!foundNormal) {
        return false;
    }

    vec3Normalize(&normal);
    const double greatCircleEpsilon = 1e-14;
    for (int i = 0; i < numEdges; i++) {
        if (fabs(vec3Dot(&normal, &edges[i].vert)) > greatCircleEpsilon) {
            return false;
        }
    }

    return true;
}

static H3Error _geodesicLoopFromGeo(const GeoLoop *loop, GeodesicLoop *out,
                                    bool rejectLargeLoop) {
    if (!out || !loop || loop->numVerts <= 0) {
        return E_DOMAIN;
    }
    *out = (GeodesicLoop){0};

    const int n = loop->numVerts;
    GeodesicEdge *edges = H3_MEMORY(calloc)(n, sizeof(GeodesicEdge));
    if (!edges) {
        return E_MEMORY_ALLOC;
    }

    out->edges = edges;
    out->numEdges = n;
    out->centroid.x = 0;
    out->centroid.y = 0;
    out->centroid.z = 0;

    for (int i = 0; i < n; i++) {
        if (!isfinite(loop->verts[i].lat) || !isfinite(loop->verts[i].lng)) {
            H3_MEMORY(free)(edges);
            out->edges = NULL;
            out->numEdges = 0;
            return E_DOMAIN;
        }
        _geoToVec3d(&loop->verts[i], &edges[i].vert);
    }

    // Great-circle boundary loops are always ambiguous (zero enclosed area)
    // regardless of polygon size, so reject them unconditionally.
    if (_geoLoopVerticesOnGreatCircle(edges, n)) {
        H3_MEMORY(free)(edges);
        out->edges = NULL;
        out->numEdges = 0;
        return E_DOMAIN;
    }

    if (rejectLargeLoop) {
        Vec3d pole;
        if (!_geoLoopVerticesFitHemisphere(edges, n, &pole)) {
            H3_MEMORY(free)(edges);
            out->edges = NULL;
            out->numEdges = 0;
            return E_DOMAIN;
        }
        out->hemispherePole = pole;
        out->hasHemispherePole = true;
    }

    for (int i = 0; i < n; i++) {
        const Vec3d *v1 = &edges[i].vert;
        const Vec3d *v2 = &edges[(i + 1) % n].vert;

        out->centroid.x += v1->x;
        out->centroid.y += v1->y;
        out->centroid.z += v1->z;

        vec3Cross(v1, v2, &edges[i].edgeCross);
        edges[i].edgeDot = vec3Dot(v1, v2);

        AABB *box = &edges[i].aabb;
        aabbEmptyInverted(box);
        aabbUpdateWithVec3d(box, v1);
        aabbUpdateWithVec3d(box, v2);
        aabbUpdateWithArcExtrema(box, v1, v2, &edges[i].edgeCross);
    }

    // Normalize the centroid to a unit vector for use in AABB construction
    // and other geometric queries.
    vec3Normalize(&out->centroid);

    return E_SUCCESS;
}

static void _geodesicLoopDestroy(GeodesicLoop *loop) {
    if (!loop || !loop->edges) {
        return;
    }

    H3_MEMORY(free)(loop->edges);
    loop->edges = NULL;
    loop->numEdges = 0;
}

H3Error geodesicPolygonCreate(const GeoPolygon *polygon,
                              GeodesicPolygon **out) {
    if (!polygon || !out || polygon->geoloop.numVerts <= 0) {
        return E_DOMAIN;
    }

    GeodesicPolygon *result = H3_MEMORY(calloc)(1, sizeof(GeodesicPolygon));
    if (!result) {
        return E_MEMORY_ALLOC;
    }

    H3Error loopErr =
        _geodesicLoopFromGeo(&polygon->geoloop, &result->geoloop, true);
    if (loopErr != E_SUCCESS) {
        _geodesicLoopDestroy(&result->geoloop);
        H3_MEMORY(free)(result);
        return loopErr;
    }

    const int holeCount = polygon->numHoles;
    result->numHoles = holeCount;
    if (holeCount > 0) {
        if (!polygon->holes) {
            _geodesicLoopDestroy(&result->geoloop);
            H3_MEMORY(free)(result);
            return E_DOMAIN;
        }
        result->holes = H3_MEMORY(calloc)(holeCount, sizeof(GeodesicLoop));
        if (!result->holes) {
            _geodesicLoopDestroy(&result->geoloop);
            H3_MEMORY(free)(result);
            return E_MEMORY_ALLOC;
        }
        for (int i = 0; i < holeCount; i++) {
            if (polygon->holes[i].numVerts <= 0) {
                for (int j = 0; j < i; j++) {
                    _geodesicLoopDestroy(&result->holes[j]);
                }
                H3_MEMORY(free)(result->holes);
                _geodesicLoopDestroy(&result->geoloop);
                H3_MEMORY(free)(result);
                return E_DOMAIN;
            }
            H3Error holeErr = _geodesicLoopFromGeo(&polygon->holes[i],
                                                   &result->holes[i], false);
            if (holeErr != E_SUCCESS) {
                for (int j = 0; j < i; j++) {
                    _geodesicLoopDestroy(&result->holes[j]);
                }
                H3_MEMORY(free)(result->holes);
                _geodesicLoopDestroy(&result->geoloop);
                H3_MEMORY(free)(result);
                return holeErr;
            }
        }
    }

    _geodesicLoopToAABB(&result->geoloop, &result->aabb);

    *out = result;
    return E_SUCCESS;
}

void geodesicPolygonDestroy(GeodesicPolygon *polygon) {
    if (!polygon) {
        return;
    }

    _geodesicLoopDestroy(&polygon->geoloop);

    if (polygon->holes) {
        for (int i = 0; i < polygon->numHoles; i++) {
            _geodesicLoopDestroy(&polygon->holes[i]);
        }
        H3_MEMORY(free)(polygon->holes);
    }

    H3_MEMORY(free)(polygon);
}

bool geodesicPolygonCapIntersects(const GeodesicPolygon *polygon,
                                  const SphereCap *cap) {
    if (!polygon || !cap) {
        return false;
    }

    return _geodesicSphereCapOverlapsAABB(cap, &polygon->aabb);
}

/**
 * Check whether a geodesic polygon boundary intersects a cell boundary.
 *
 * For each polygon loop (outer shell and holes) we prune arcs whose bounding
 * boxes do not overlap the query cap. Remaining arcs are tested pairwise
 * against the cell boundary segments with `_geodesicEdgesCross`. Early returns
 * avoid unnecessary great-circle computations.
 */
bool geodesicPolygonBoundaryIntersects(const GeodesicPolygon *polygon,
                                       const GeodesicCellBoundary *boundary,
                                       const SphereCap *cap) {
    if (!polygon || !boundary || !cap) {
        return false;
    }

    const GeodesicLoop *loops = &polygon->geoloop;
    const int loopCount = 1 + polygon->numHoles;

    for (int loopIdx = 0; loopIdx < loopCount; loopIdx++) {
        const GeodesicLoop *loop =
            (loopIdx == 0) ? loops : &polygon->holes[loopIdx - 1];

        for (int i = 0; i < loop->numEdges; i++) {
            if (!_geodesicSphereCapOverlapsAABB(cap, &loop->edges[i].aabb)) {
                continue;
            }

            int nextI = (i + 1) % loop->numEdges;
            for (int j = 0; j < boundary->numVerts; j++) {
                int nextJ = (j + 1) % boundary->numVerts;
                if (_geodesicEdgesCross(
                        &boundary->verts[j], &boundary->verts[nextJ],
                        &loop->edges[i].vert, &loop->edges[nextI].vert,
                        &loop->edges[i].edgeCross)) {
                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * Evaluate whether a 3D unit vector lies inside a polygon with optional holes.
 *
 * The outer loop is tested for containment first; if it fails we can reject the
 * point immediately. When the point is inside the shell we ensure it does not
 * land inside any hole loop, mirroring the typical point-in-polygon predicate
 * on the sphere.
 */
bool geodesicPolygonContainsPoint(const GeodesicPolygon *polygon,
                                  const Vec3d *point) {
    if (!polygon || !point) {
        return false;
    }

    if (!_geodesicLoopContainsPoint(&polygon->geoloop, point)) {
        return false;
    }

    for (int i = 0; i < polygon->numHoles; i++) {
        if (_geodesicLoopContainsPoint(&polygon->holes[i], point)) {
            return false;
        }
    }

    return true;
}

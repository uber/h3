/*
 * Copyright 2026 Uber Technologies, Inc.
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
/** @file
 * @brief Measures how far cellToLatLng and cellToBoundary land from the exact
 * value of the projection they implement.
 *
 * The reference is an arbitrary-precision (MPFR) evaluation of the algebraic
 * form of the face-to-sphere projection: the atan2/cos/sin round trip on the
 * azimuth becomes x/r and y/r, the Class III rotation and the face axis
 * subtraction become angle-addition identities, and atan/cos/sin on the
 * gnomonic radius become 1/sqrt(1+r^2) and r/sqrt(1+r^2). The tables in
 * faceijk.c and RES0_U_GNOMONIC are taken as exact definitions; sqrt(3)/2,
 * 1/sqrt(7), 1/3 and the aperture 7 rotation are the exact numbers their
 * constants approximate.
 *
 * Errors are measured as the chord between the library's answer, converted
 * back to a unit vector at full precision, and the exact unit vector. This is
 * uniform over the sphere, unlike a per-coordinate comparison in lat/lng.
 *
 * Every cell at resolutions 0 through H3_EXACT_PROJECTION_RES (default 2) is
 * checked, plus one deterministic descendant of each base cell at every finer
 * resolution (the same cells as h3-go's exact_projection_test.go).
 *
 * Set H3_EXACT_PROJECTION_REPORT=1 to print the worst separation per
 * resolution and a histogram in units in the last place, instead of asserting
 * the bound.
 *
 *  usage: `testExactProjectionExhaustive`
 */

#include <float.h>
#include <math.h>
#include <mpfr.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "coordijk.h"
#include "faceijk.h"
#include "h3Index.h"
#include "h3api.h"
#include "iterators.h"
#include "test.h"

/** Precision of the reference, in bits */
#define PREC 256
#define RND MPFR_RNDN

/**
 * Bound on the separation from the exact projection, in degrees of arc, about
 * 8 units in the last place of a unit vector component. The worst separation
 * observed is 5.3e-14 degrees (4.1 ulp) on macOS arm64 over every cell to
 * resolution 4 and samples to 15; see the report mode output.
 */
#define BOUND_DEG 1e-13

/** One unit in the last place of a unit vector component, in degrees */
#define ULP_DEG (DBL_EPSILON * 180.0 / M_PI)

#define NUM_HIST_BUCKETS 8

typedef struct {
    mpfr_t x, y;
} MVec2;

typedef struct {
    mpfr_t x, y, z;
} MVec3;

/** Worst case and histogram for one kind of point at one resolution */
typedef struct {
    long count;
    double worstDeg;
    H3Index worstCell;
    int worstVert;
    long hist[NUM_HIST_BUCKETS];
} Stats;

static Stats vec3Stats[MAX_H3_RES + 1];
static Stats centerStats[MAX_H3_RES + 1];
static Stats vertStats[MAX_H3_RES + 1];

// constants
static mpfr_t sqrt3Half, rSqrt7, oneThird, gnomonic, rotCos, rotSin, fltEps;
static MVec3 center[NUM_ICOSA_FACES], north[NUM_ICOSA_FACES],
    east[NUM_ICOSA_FACES];
static mpfr_t axisCos[NUM_ICOSA_FACES], axisSin[NUM_ICOSA_FACES];

// scratch values, initialized once and reused for every projection
static mpfr_t r, cosT, sinT, cosAz, sinAz, cosR, sinR, ta, tb, tc;
static MVec2 hex, orig0, orig1, edge0, edge1, inter;
static MVec3 dir, got;
static MVec3 expected[MAX_CELL_BNDRY_VERTS];

static void mvec2Init(MVec2 *v) { mpfr_inits2(PREC, v->x, v->y, (mpfr_ptr)0); }

static void mvec2Clear(MVec2 *v) { mpfr_clears(v->x, v->y, (mpfr_ptr)0); }

static void mvec3Init(MVec3 *v) {
    mpfr_inits2(PREC, v->x, v->y, v->z, (mpfr_ptr)0);
}

static void mvec3Clear(MVec3 *v) { mpfr_clears(v->x, v->y, v->z, (mpfr_ptr)0); }

static void mvec3Set(MVec3 *out, const MVec3 *v) {
    mpfr_set(out->x, v->x, RND);
    mpfr_set(out->y, v->y, RND);
    mpfr_set(out->z, v->z, RND);
}

/** out = a * v1 + b * v2; out may not alias v1 or v2 */
static void mvec3LinComb(MVec3 *out, mpfr_t a, const MVec3 *v1, mpfr_t b,
                         const MVec3 *v2) {
    mpfr_fmma(out->x, a, v1->x, b, v2->x, RND);
    mpfr_fmma(out->y, a, v1->y, b, v2->y, RND);
    mpfr_fmma(out->z, a, v1->z, b, v2->z, RND);
}

static void mvec3Normalize(MVec3 *v) {
    mpfr_sqr(ta, v->x, RND);
    mpfr_fma(ta, v->y, v->y, ta, RND);
    mpfr_fma(ta, v->z, v->z, ta, RND);
    mpfr_rec_sqrt(ta, ta, RND);
    mpfr_mul(v->x, v->x, ta, RND);
    mpfr_mul(v->y, v->y, ta, RND);
    mpfr_mul(v->z, v->z, ta, RND);
}

/** Exact equivalent of _ijkToHex2d */
static void exactHex2d(const CoordIJK *ijk, MVec2 *out) {
    long i = (long)ijk->i - ijk->k;
    long j = (long)ijk->j - ijk->k;
    // x = i - j / 2, which is exact at this precision
    mpfr_set_si(out->x, j, RND);
    mpfr_div_2ui(out->x, out->x, 1, RND);
    mpfr_si_sub(out->x, i, out->x, RND);
    mpfr_mul_si(out->y, sqrt3Half, j, RND);
}

/** Exact equivalent of _v2dIntersect */
static void exactIntersect(const MVec2 *p0, const MVec2 *p1, const MVec2 *p2,
                           const MVec2 *p3, MVec2 *out) {
    MVec2 s1, s2;
    mvec2Init(&s1);
    mvec2Init(&s2);
    mpfr_sub(s1.x, p1->x, p0->x, RND);
    mpfr_sub(s1.y, p1->y, p0->y, RND);
    mpfr_sub(s2.x, p3->x, p2->x, RND);
    mpfr_sub(s2.y, p3->y, p2->y, RND);

    // t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) /
    //     (-s2.x * s1.y + s1.x * s2.y)
    mpfr_sub(ta, p0->y, p2->y, RND);
    mpfr_sub(tb, p0->x, p2->x, RND);
    mpfr_fmms(tc, s2.x, ta, s2.y, tb, RND);
    mpfr_fmms(ta, s1.x, s2.y, s2.x, s1.y, RND);
    mpfr_div(tc, tc, ta, RND);

    mpfr_fma(out->x, tc, s1.x, p0->x, RND);
    mpfr_fma(out->y, tc, s1.y, p0->y, RND);
    mvec2Clear(&s1);
    mvec2Clear(&s2);
}

/** Exact equivalent of _v2dAlmostEquals */
static bool exactAlmostEquals(const MVec2 *v1, const MVec2 *v2) {
    mpfr_sub(ta, v1->x, v2->x, RND);
    mpfr_sub(tb, v1->y, v2->y, RND);
    return mpfr_cmpabs(ta, fltEps) < 0 && mpfr_cmpabs(tb, fltEps) < 0;
}

/** 7^(res / 2), the unitScaleByCIIres table for a Class II resolution */
static long unitScale(int res) {
    long scale = 1;
    for (int i = 0; i < res / 2; i++) scale *= 7;
    return scale;
}

/**
 * The adjacentFaceDir table, derived from faceNeighbors: the direction from
 * face to other, 0 if they are the same face, -1 if they are not adjacent.
 */
static int adjacentFaceDir(int face, int other) {
    for (int d = 0; d < 4; d++) {
        if (faceNeighbors[face][d].face == other) return d;
    }
    return -1;
}

/** Vertex n (0, 1 or 2) of the icosahedron face, scaled for a Class II res */
static void exactFaceVertex(int n, int res, MVec2 *out) {
    // maxDimByCIIres
    long maxDim = 2 * unitScale(res);
    // v0 = (3 maxDim, 0), v1 = (-1.5 maxDim, 3 sqrt(3)/2 maxDim),
    // v2 = (-1.5 maxDim, -3 sqrt(3)/2 maxDim)
    if (n == 0) {
        mpfr_set_si(out->x, 3 * maxDim, RND);
        mpfr_set_zero(out->y, 1);
    } else {
        mpfr_set_si(out->x, -3 * maxDim, RND);
        mpfr_div_2ui(out->x, out->x, 1, RND);
        mpfr_mul_si(out->y, sqrt3Half, 3 * maxDim, RND);
        if (n == 2) mpfr_neg(out->y, out->y, RND);
    }
}

/** The icosahedron face edge in direction d, scaled for a Class II res */
static void exactFaceEdge(int d, int res, MVec2 *e0, MVec2 *e1) {
    switch (d) {
        case IJ:
            exactFaceVertex(0, res, e0);
            exactFaceVertex(1, res, e1);
            break;
        case JK:
            exactFaceVertex(1, res, e0);
            exactFaceVertex(2, res, e1);
            break;
        case KI:
            exactFaceVertex(2, res, e0);
            exactFaceVertex(0, res, e1);
            break;
        default:
            t_assert(false, "faces of a crossing edge are adjacent");
    }
}

/**
 * Exact equivalent of _hex2dToVec3, in the algebraic form described in the
 * file comment.
 */
static void exactToVec3(const MVec2 *v, int face, int res, int substrate,
                        MVec3 *out) {
    mpfr_hypot(r, v->x, v->y, RND);

    // with integer inputs r is zero only at the face center
    if (mpfr_zero_p(r)) {
        mvec3Set(out, &center[face]);
        return;
    }

    // cos and sin of theta = atan2(y, x)
    mpfr_div(cosT, v->x, r, RND);
    mpfr_div(sinT, v->y, r, RND);

    // scale for current resolution length u
    for (int i = 0; i < res; i++) mpfr_mul(r, r, rSqrt7, RND);

    // substrate resolutions are always Class II (see _hex2dToVec3)
    if (substrate) mpfr_mul(r, r, oneThird, RND);

    mpfr_mul(r, r, gnomonic, RND);

    // cos and sin of atan(r)
    mpfr_sqr(cosR, r, RND);
    mpfr_add_ui(cosR, cosR, 1, RND);
    mpfr_rec_sqrt(cosR, cosR, RND);
    mpfr_mul(sinR, r, cosR, RND);

    // theta += M_AP7_ROT_RADS for Class III
    if (!substrate && isResolutionClassIII(res)) {
        mpfr_fmms(ta, cosT, rotCos, sinT, rotSin, RND);
        mpfr_fmma(sinT, sinT, rotCos, cosT, rotSin, RND);
        mpfr_set(cosT, ta, RND);
    }

    // azimuth = faceAxesAzRadsCII[face][0] - theta
    mpfr_fmma(cosAz, axisCos[face], cosT, axisSin[face], sinT, RND);
    mpfr_fmms(sinAz, axisSin[face], cosT, axisCos[face], sinT, RND);

    mvec3LinComb(&dir, cosAz, &north[face], sinAz, &east[face]);
    mvec3LinComb(out, cosR, &center[face], sinR, &dir);
    mvec3Normalize(out);
}

/**
 * The expected boundary of a hexagon, following _faceIjkToCellBoundary for
 * the full loop step for step.
 *
 * @return number of vertexes written to expected
 */
static int exactHexBoundary(const FaceIJK *h, int res) {
    int adjRes = res;
    FaceIJK centerIJK = *h;
    FaceIJK fijkVerts[NUM_HEX_VERTS];
    _faceIjkToVerts(&centerIJK, &adjRes, fijkVerts);

    int numVerts = 0;
    int lastFace = -1;
    Overage lastOverage = NO_OVERAGE;
    // one more iteration for a crossing on the last edge
    for (int vert = 0; vert < NUM_HEX_VERTS + 1; vert++) {
        int v = vert % NUM_HEX_VERTS;
        FaceIJK fijk = fijkVerts[v];
        Overage overage = _adjustOverageClassII(&fijk, adjRes, 0, 1);

        if (isResolutionClassIII(res) && vert > 0 && fijk.face != lastFace &&
            lastOverage != FACE_EDGE) {
            int lastV = (v + 5) % NUM_HEX_VERTS;
            exactHex2d(&fijkVerts[lastV].coord, &orig0);
            exactHex2d(&fijkVerts[v].coord, &orig1);

            int face2 = (lastFace == centerIJK.face) ? fijk.face : lastFace;
            exactFaceEdge(adjacentFaceDir(centerIJK.face, face2), adjRes,
                          &edge0, &edge1);
            exactIntersect(&orig0, &orig1, &edge0, &edge1, &inter);

            if (!exactAlmostEquals(&orig0, &inter) &&
                !exactAlmostEquals(&orig1, &inter)) {
                exactToVec3(&inter, centerIJK.face, adjRes, 1,
                            &expected[numVerts++]);
            }
        }

        if (vert < NUM_HEX_VERTS) {
            exactHex2d(&fijk.coord, &hex);
            exactToVec3(&hex, fijk.face, adjRes, 1, &expected[numVerts++]);
        }

        lastFace = fijk.face;
        lastOverage = overage;
    }
    return numVerts;
}

/**
 * The expected boundary of a pentagon, following _faceIjkPentToCellBoundary
 * for the full loop step for step.
 *
 * @return number of vertexes written to expected
 */
static int exactPentBoundary(const FaceIJK *h, int res) {
    int adjRes = res;
    FaceIJK centerIJK = *h;
    FaceIJK fijkVerts[NUM_PENT_VERTS];
    _faceIjkPentToVerts(&centerIJK, &adjRes, fijkVerts);

    int numVerts = 0;
    FaceIJK lastFijk = {0};
    // one more iteration for a crossing on the last edge
    for (int vert = 0; vert < NUM_PENT_VERTS + 1; vert++) {
        int v = vert % NUM_PENT_VERTS;
        FaceIJK fijk = fijkVerts[v];
        _adjustPentVertOverage(&fijk, adjRes);

        // all Class III pentagon edges cross icosa edges
        if (isResolutionClassIII(res) && vert > 0) {
            exactHex2d(&lastFijk.coord, &orig0);

            // the current vertex in the coordinates of the last face
            FaceIJK tmpFijk = fijk;
            int currentToLastDir = adjacentFaceDir(tmpFijk.face, lastFijk.face);
            if (currentToLastDir < 0) {
                t_assert(false, "consecutive pentagon faces are adjacent");
            }
            const FaceOrientIJK *fijkOrient =
                &faceNeighbors[tmpFijk.face][currentToLastDir];
            tmpFijk.face = fijkOrient->face;
            CoordIJK *ijk = &tmpFijk.coord;
            for (int i = 0; i < fijkOrient->ccwRot60; i++) _ijkRotate60ccw(ijk);
            CoordIJK transVec = fijkOrient->translate;
            _ijkScale(&transVec, (int)unitScale(adjRes) * 3);
            _ijkAdd(ijk, &transVec, ijk);
            _ijkNormalize(ijk);
            exactHex2d(ijk, &orig1);

            exactFaceEdge(adjacentFaceDir(tmpFijk.face, fijk.face), adjRes,
                          &edge0, &edge1);
            exactIntersect(&orig0, &orig1, &edge0, &edge1, &inter);
            exactToVec3(&inter, tmpFijk.face, adjRes, 1, &expected[numVerts++]);
        }

        if (vert < NUM_PENT_VERTS) {
            exactHex2d(&fijk.coord, &hex);
            exactToVec3(&hex, fijk.face, adjRes, 1, &expected[numVerts++]);
        }

        lastFijk = fijk;
    }
    return numVerts;
}

/**
 * Separation in degrees of arc between a library answer and the exact unit
 * vector. The answer is converted back to a unit vector at full precision, so
 * the conversion adds nothing to the measured error.
 */
static double separationDeg(LatLng ll, const MVec3 *want) {
    mpfr_set_d(ta, ll.lat, RND);
    mpfr_sin_cos(sinT, cosT, ta, RND);
    mpfr_set_d(ta, ll.lng, RND);
    mpfr_sin_cos(sinAz, cosAz, ta, RND);
    mpfr_mul(got.x, cosT, cosAz, RND);
    mpfr_mul(got.y, cosT, sinAz, RND);
    mpfr_set(got.z, sinT, RND);

    mpfr_sub(got.x, got.x, want->x, RND);
    mpfr_sub(got.y, got.y, want->y, RND);
    mpfr_sub(got.z, got.z, want->z, RND);
    mpfr_hypot(ta, got.x, got.y, RND);
    mpfr_hypot(ta, ta, got.z, RND);

    // for separations this small the chord is the angle in radians
    return mpfr_get_d(ta, RND) * 180.0 / M_PI;
}

/** Separation in degrees of arc between a library unit vector and the exact
 * one */
static double separationVec3Deg(Vec3d v, const MVec3 *want) {
    mpfr_set_d(got.x, v.x, RND);
    mpfr_set_d(got.y, v.y, RND);
    mpfr_set_d(got.z, v.z, RND);
    mpfr_sub(got.x, got.x, want->x, RND);
    mpfr_sub(got.y, got.y, want->y, RND);
    mpfr_sub(got.z, got.z, want->z, RND);
    mpfr_hypot(ta, got.x, got.y, RND);
    mpfr_hypot(ta, ta, got.z, RND);
    return mpfr_get_d(ta, RND) * 180.0 / M_PI;
}

static void record(Stats *s, double sepDeg, H3Index cell, int vert) {
    s->count++;
    if (sepDeg > s->worstDeg) {
        s->worstDeg = sepDeg;
        s->worstCell = cell;
        s->worstVert = vert;
    }
    // buckets: < 0.5, < 1, < 2, < 4, ... ulps
    double ulps = sepDeg / ULP_DEG;
    int bucket = 0;
    for (double limit = 0.5; bucket < NUM_HIST_BUCKETS - 1 && ulps >= limit;
         limit *= 2) {
        bucket++;
    }
    s->hist[bucket]++;
}

static void checkCell(H3Index cell) {
    char msg[128];
    int res = H3_GET_RESOLUTION(cell);
    FaceIJK fijk;
    if (_h3ToFaceIjk(cell, &fijk)) {
        t_assertSuccess(_h3ToFaceIjk(cell, &fijk));
    }

    LatLng ll;
    if (H3_EXPORT(cellToLatLng)(cell, &ll)) {
        t_assertSuccess(H3_EXPORT(cellToLatLng)(cell, &ll));
    }
    exactHex2d(&fijk.coord, &hex);
    exactToVec3(&hex, fijk.face, res, 0, &expected[0]);
    record(&centerStats[res], separationDeg(ll, &expected[0]), cell, -1);
    Vec3d v3;
    _faceIjkToVec3(&fijk, res, &v3);
    record(&vec3Stats[res], separationVec3Deg(v3, &expected[0]), cell, -1);

    CellBoundary cb;
    if (H3_EXPORT(cellToBoundary)(cell, &cb)) {
        t_assertSuccess(H3_EXPORT(cellToBoundary)(cell, &cb));
    }
    int numVerts = H3_EXPORT(isPentagon)(cell) ? exactPentBoundary(&fijk, res)
                                               : exactHexBoundary(&fijk, res);
    if (numVerts != cb.numVerts) {
        snprintf(msg, sizeof(msg), "%llx: expected %d vertexes, got %d",
                 (unsigned long long)cell, numVerts, cb.numVerts);
        t_assert(numVerts == cb.numVerts, msg);
    }
    for (int v = 0; v < numVerts; v++) {
        record(&vertStats[res], separationDeg(cb.verts[v], &expected[v]), cell,
               v);
    }
}

static void init(void) {
    mpfr_inits2(PREC, sqrt3Half, rSqrt7, oneThird, gnomonic, rotCos, rotSin,
                fltEps, r, cosT, sinT, cosAz, sinAz, cosR, sinR, ta, tb, tc,
                (mpfr_ptr)0);
    mvec2Init(&hex);
    mvec2Init(&orig0);
    mvec2Init(&orig1);
    mvec2Init(&edge0);
    mvec2Init(&edge1);
    mvec2Init(&inter);
    mvec3Init(&dir);
    mvec3Init(&got);
    for (int i = 0; i < MAX_CELL_BNDRY_VERTS; i++) mvec3Init(&expected[i]);

    mpfr_sqrt_ui(sqrt3Half, 3, RND);
    mpfr_div_2ui(sqrt3Half, sqrt3Half, 1, RND);
    mpfr_set_ui(rSqrt7, 7, RND);
    mpfr_rec_sqrt(rSqrt7, rSqrt7, RND);
    mpfr_set_ui(oneThird, 1, RND);
    mpfr_div_ui(oneThird, oneThird, 3, RND);
    mpfr_set_d(gnomonic, RES0_U_GNOMONIC, RND);
    // M_AP7_ROT_RADS is asin(sqrt(3/28))
    mpfr_set_ui(rotSin, 3, RND);
    mpfr_div_ui(rotSin, rotSin, 28, RND);
    mpfr_sqrt(rotSin, rotSin, RND);
    mpfr_set_ui(rotCos, 25, RND);
    mpfr_div_ui(rotCos, rotCos, 28, RND);
    mpfr_sqrt(rotCos, rotCos, RND);
    mpfr_set_d(fltEps, FLT_EPSILON, RND);

    for (int f = 0; f < NUM_ICOSA_FACES; f++) {
        MVec3 *p = &center[f];
        mvec3Init(p);
        mvec3Init(&north[f]);
        mvec3Init(&east[f]);
        mpfr_inits2(PREC, axisCos[f], axisSin[f], (mpfr_ptr)0);

        mpfr_set_d(p->x, faceCenterPoint[f].x, RND);
        mpfr_set_d(p->y, faceCenterPoint[f].y, RND);
        mpfr_set_d(p->z, faceCenterPoint[f].z, RND);

        mpfr_set_d(ta, faceAxesAzRadsCII[f][0], RND);
        mpfr_sin_cos(axisSin[f], axisCos[f], ta, RND);

        // _vec3TangentBasis: north = normalize(pole - (pole . p) p),
        // east = north x p
        MVec3 *n = &north[f];
        mpfr_mul(n->x, p->z, p->x, RND);
        mpfr_neg(n->x, n->x, RND);
        mpfr_mul(n->y, p->z, p->y, RND);
        mpfr_neg(n->y, n->y, RND);
        mpfr_sqr(n->z, p->z, RND);
        mpfr_ui_sub(n->z, 1, n->z, RND);
        mvec3Normalize(n);

        MVec3 *e = &east[f];
        mpfr_fmms(e->x, n->y, p->z, n->z, p->y, RND);
        mpfr_fmms(e->y, n->z, p->x, n->x, p->z, RND);
        mpfr_fmms(e->z, n->x, p->y, n->y, p->x, RND);
    }
}

static void clear(void) {
    mpfr_clears(sqrt3Half, rSqrt7, oneThird, gnomonic, rotCos, rotSin, fltEps,
                r, cosT, sinT, cosAz, sinAz, cosR, sinR, ta, tb, tc,
                (mpfr_ptr)0);
    mvec2Clear(&hex);
    mvec2Clear(&orig0);
    mvec2Clear(&orig1);
    mvec2Clear(&edge0);
    mvec2Clear(&edge1);
    mvec2Clear(&inter);
    mvec3Clear(&dir);
    mvec3Clear(&got);
    for (int i = 0; i < MAX_CELL_BNDRY_VERTS; i++) mvec3Clear(&expected[i]);
    for (int f = 0; f < NUM_ICOSA_FACES; f++) {
        mvec3Clear(&center[f]);
        mvec3Clear(&north[f]);
        mvec3Clear(&east[f]);
        mpfr_clears(axisCos[f], axisSin[f], (mpfr_ptr)0);
    }
    mpfr_free_cache();
}

static void printStats(const char *kind, int res, const Stats *s) {
    printf("res %2d %-6s n=%-8ld worst %.3e deg (%5.2f ulp) cell %llx", res,
           kind, s->count, s->worstDeg, s->worstDeg / ULP_DEG,
           (unsigned long long)s->worstCell);
    if (s->worstVert >= 0) printf(" vert %d", s->worstVert);
    printf("\n       ulps:");
    double limit = 0.5;
    for (int b = 0; b < NUM_HIST_BUCKETS; b++, limit *= 2) {
        if (b < NUM_HIST_BUCKETS - 1) {
            printf(" <%g:%ld", limit, s->hist[b]);
        } else {
            printf(" >=%g:%ld", limit / 2, s->hist[b]);
        }
    }
    printf("\n");
}

static void assertStats(const char *kind, int res, const Stats *s) {
    char msg[160];
    snprintf(msg, sizeof(msg),
             "res %d %s: %.3e deg from exact at %llx vert %d exceeds bound",
             res, kind, s->worstDeg, (unsigned long long)s->worstCell,
             s->worstVert);
    t_assert(s->worstDeg <= BOUND_DEG, msg);
}

SUITE(exactProjection) {
    init();

    int fullRes = 2;
    const char *resEnv = getenv("H3_EXACT_PROJECTION_RES");
    if (resEnv != NULL) fullRes = atoi(resEnv);
    if (fullRes < 0) fullRes = 0;
    if (fullRes > MAX_H3_RES) fullRes = MAX_H3_RES;
    const char *reportEnv = getenv("H3_EXACT_PROJECTION_REPORT");
    bool report = reportEnv != NULL && strcmp(reportEnv, "0") != 0;

    // every cell at the coarse resolutions
    for (int res = 0; res <= fullRes; res++) {
        for (IterCellsResolution iter = iterInitRes(res); iter.h;
             iterStepRes(&iter)) {
            checkCell(iter.h);
        }
    }

    // one descendant per base cell at each finer resolution, choosing a
    // different child at each step and varying with the base cell so the
    // sampled paths differ across the icosahedron
    H3Index baseCells[NUM_BASE_CELLS];
    t_assertSuccess(H3_EXPORT(getRes0Cells)(baseCells));
    for (int b = 0; b < NUM_BASE_CELLS; b++) {
        int baseCellNumber = H3_EXPORT(getBaseCellNumber)(baseCells[b]);
        H3Index cell = baseCells[b];
        for (int childRes = 1; childRes <= MAX_H3_RES; childRes++) {
            int64_t numChildren;
            H3Index children[7];
            t_assertSuccess(
                H3_EXPORT(cellToChildrenSize)(cell, childRes, &numChildren));
            t_assertSuccess(
                H3_EXPORT(cellToChildren)(cell, childRes, children));
            cell = children[(baseCellNumber * 31 + childRes * 7) % numChildren];
            if (childRes > fullRes) checkCell(cell);
        }
    }

    for (int res = 0; res <= MAX_H3_RES; res++) {
        if (report) {
            printStats("vec3", res, &vec3Stats[res]);
            printStats("center", res, &centerStats[res]);
            printStats("vertex", res, &vertStats[res]);
        } else {
            assertStats("vec3", res, &vec3Stats[res]);
            assertStats("center", res, &centerStats[res]);
            assertStats("vertex", res, &vertStats[res]);
        }
    }

    clear();
}

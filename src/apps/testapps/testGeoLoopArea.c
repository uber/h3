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

/** @file
 * @brief tests H3 GeoLoop area calculation
 *
 *  usage: `testGeoLoopArea`
 */

#include <math.h>

#include "area.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

static void _compareArea(LatLng *verts, int numVerts, double target_area) {
    double tol = 1e-14;
    GeoLoop loop = {.verts = verts, .numVerts = numVerts};

    double out;
    t_assertSuccess(geoLoopAreaRads2(loop, &out));
    t_assert(fabs(out - target_area) < tol, "area should match");
}

SUITE(geoLoopArea) {
    TEST(triangle_basic) {
        /*
        GeoLoop representing a triangle covering 1/8 of the globe, with
        points ordered according to right-hand rule (counter-clockwise).

        The triangle starts at the north pole, moves down 90 degrees to the
        equator, and then sweeps out 90 degrees along the equator
        before returning to the north pole.

        The globe has an area of 4*pi radians, so this 1/8 triangle piece of
        the globe should have area pi/2.
        */
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, 0.0},
            {0.0, M_PI_2},
        };

        _compareArea(verts, ARRAY_SIZE(verts), M_PI / 2);
    }

    TEST(triangle_reversed) {
        /*
        Reverse the order of the points in the triangle from the previous test,
        so that they are in clockwise order.

        Since the points are in clockwise order, GeoLoop represents the whole
        globe minus the triangle above.
        */
        LatLng verts[] = {
            {0.0, M_PI_2},
            {0.0, 0.0},
            {M_PI_2, 0.0},
        };

        _compareArea(verts, ARRAY_SIZE(verts), 7 * M_PI / 2);
    }

    TEST(slice) {
        /*
        Stitch two 1/8 triangles together, sharing an edge along the equator
        to create a 1/4 slice of the globe, with vertices at the north and south
        pole.
        */
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, 0.0},
            {-M_PI_2, 0.0},
            {0.0, M_PI_2},
        };

        _compareArea(verts, ARRAY_SIZE(verts), M_PI);
    }

    TEST(slice_reversed) {
        /*
        3/4 slice of the globe, from north to south pole, formed by reversing
        order of points from example above.
        */
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, M_PI_2},
            {-M_PI_2, 0.0},
            {0.0, 0.0},
        };

        _compareArea(verts, ARRAY_SIZE(verts), 3 * M_PI);
    }

    TEST(hemisphere_east) {
        /*
        Stitch 4 1/8 triangles together to cover the eastern hemisphere.
        */
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, 0},
            {-M_PI_2, 0.0},
            {0.0, M_PI},
        };

        _compareArea(verts, ARRAY_SIZE(verts), 2 * M_PI);
    }

    TEST(hemisphere_north) {
        /*
        Stitch 4 1/8 triangles together to cover the northern hemisphere.
        */
        LatLng verts[] = {
            {0.0, -M_PI},
            {0.0, -M_PI_2},
            {0.0, 0.0},
            {0.0, M_PI_2},
        };

        _compareArea(verts, ARRAY_SIZE(verts), 2 * M_PI);
    }

    TEST(percentageSlice) {
        /*
        Demonstrate that edge arcs between points in a polygon or geoloop
        should be less than 180 degrees (M_PI radians).

        Create a triangle from north pole to equator and back to the north pole
        that sweeps out an edge arc of t * M_PI radians along the equator,
        so it should have an area of t*M_PI for t \in [0,1].

        However, there is a discontinuity at t = 1 (i.e., M_PI radians or 180
        degrees), where expected area goes to (2 + t) * M_PI for 1 < t < 2.

        Recall that the area in stradians of the entire globe is 4*M_PI.
        */
        for (double t = 0; t <= 1.2; t += 0.01) {
            LatLng verts[] = {
                {M_PI_2, 0.0},
                {0.0, -M_PI_2},
                {0.0, t * M_PI - M_PI_2},
            };
            GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

            double out;
            t_assertSuccess(geoLoopAreaRads2(loop, &out));

            double tol = 1e-13;
            if (t < 0.99) {
                // When t < 1, the largest angle in the triangle is less than
                // 180 degrees
                t_assert(fabs(out - t * M_PI) <= tol, "expected area");
            } else if (t > 1.01) {
                /*
                Discontinuity at t == 1. For t > 1, the triangle "flips",
                because the shortest geodesic path is on the other side of
                the globe. The triangle is now oriented in clockwise order,
                and the area computed is the area *outside* of the triangle,
                which starts at 3*pi.
                */
                t_assert(fabs(out - (2 + t) * M_PI) <= tol, "expected area");
            }
            /*
            Note that we avoid testing t == 1, since the triangle
            isn't well defined because there are many possible geodesic
            shortest paths when consecutive points are antipodal
            (180 degrees apart).
            */
        }
    }

    TEST(percentageSlice_large) {
        /*
        Continuing from the test above, note that a large polygon with
        t > 1 is *still* representable and we can compute its area accurately;
        we just need to add intermediate vertices so that
        no edge arc is greater than 180 degrees.
        */
        double t = 1.2;
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, -M_PI_2},
            {0.0, 0.0},  // Extra vertex so every angle is < 180 degrees
            {0.0, t * M_PI - M_PI_2},
        };

        _compareArea(verts, ARRAY_SIZE(verts), t * M_PI);
    }

    TEST(degenerateLoop2) {
        // Note that `geoLoopAreaRads2()` works without error on degenerate
        // loops, returning 0 area.
        LatLng verts[] = {
            {M_PI_2, 0.0},
            {0.0, -M_PI_2},
        };
        _compareArea(verts, ARRAY_SIZE(verts), 0.0);
    }

    TEST(degenerateLoop1) {
        // Note that `geoLoopAreaRads2()` works without error on degenerate
        // loops, returning 0 area.
        LatLng verts[] = {
            {0.0, 0.0},
        };
        _compareArea(verts, ARRAY_SIZE(verts), 0.0);
    }

    TEST(degenerateLoop0) {
        // Note that `geoLoopAreaRads2()` works without error on degenerate
        // loops, returning 0 area.
        _compareArea(NULL, 0, 0.0);
    }
}

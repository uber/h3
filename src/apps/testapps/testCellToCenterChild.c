/*
 * Copyright 2019-2020 Uber Technologies, Inc.
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

#include <inttypes.h>
#include <stdlib.h>

#include "h3Index.h"
#include "test.h"

SUITE(cellToCenterChild) {
    H3Index baseHex;
    GeoPoint baseCentroid;
    setH3Index(&baseHex, 8, 4, 2);
    H3_EXPORT(cellToPoint)(baseHex, &baseCentroid);

    TEST(propertyTests) {
        for (int res = 0; res <= MAX_H3_RES - 1; res++) {
            for (int childRes = res + 1; childRes <= MAX_H3_RES; childRes++) {
                GeoPoint centroid;
                H3Index h3Index;
                t_assertSuccess(
                    H3_EXPORT(pointToCell)(&baseCentroid, res, &h3Index));
                H3_EXPORT(cellToPoint)(h3Index, &centroid);

                H3Index geoChild;
                t_assertSuccess(
                    H3_EXPORT(pointToCell)(&centroid, childRes, &geoChild));
                H3Index centerChild =
                    H3_EXPORT(cellToCenterChild)(h3Index, childRes);

                t_assert(
                    centerChild == geoChild,
                    "center child should be same as indexed centroid at child "
                    "resolution");
                t_assert(H3_EXPORT(getResolution)(centerChild) == childRes,
                         "center child should have correct resolution");
                t_assert(
                    H3_EXPORT(cellToParent)(centerChild, res) == h3Index,
                    "parent at original resolution should be initial index");
            }
        }
    }

    TEST(sameRes) {
        int res = H3_EXPORT(getResolution)(baseHex);
        t_assert(H3_EXPORT(cellToCenterChild)(baseHex, res) == baseHex,
                 "center child at same resolution should return self");
    }

    TEST(invalidInputs) {
        int res = H3_EXPORT(getResolution)(baseHex);
        t_assert(H3_EXPORT(cellToCenterChild)(baseHex, res - 1) == 0,
                 "should fail at coarser resolution");
        t_assert(H3_EXPORT(cellToCenterChild)(baseHex, -1) == 0,
                 "should fail for negative resolution");
        t_assert(H3_EXPORT(cellToCenterChild)(baseHex, MAX_H3_RES + 1) == 0,
                 "should fail beyond finest resolution");
    }
}

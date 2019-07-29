/*
 * Copyright 2017-2019 Uber Technologies, Inc.
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

#include <stdlib.h>
#include "h3Index.h"
#include "test.h"

SUITE(h3ToCenterChild) {
    H3Index hex8;
    GeoCoord centroid;
    setH3Index(&hex8, 8, 4, 2);
    H3_EXPORT(h3ToGeo)(hex8, &centroid);

    TEST(twoStepRes) {
        H3Index centerChild = H3_EXPORT(h3ToCenterChild)(hex8, 10);
        H3Index geoChild = H3_EXPORT(geoToH3)(&centroid, 10);

        t_assert(centerChild == geoChild, "Found expected center child");
    }

    TEST(sameRes) {
        t_assert(H3_EXPORT(h3ToCenterChild)(hex8, 8) == hex8,
                 "Same resolution returns self");
    }

    TEST(invalidInputs) {
        t_assert(H3_EXPORT(h3ToCenterChild)(hex8, 6) == 0,
                 "Lower resolution fails");
        t_assert(H3_EXPORT(h3ToCenterChild)(hex8, -1) == 0,
                 "Invalid resolution fails");
        t_assert(H3_EXPORT(h3ToCenterChild)(hex8, 16) == 0,
                 "Invalid resolution fails");
    }
}

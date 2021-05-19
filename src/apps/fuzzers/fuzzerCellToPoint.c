/*
 * Copyright 2021 Uber Technologies, Inc.
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
 * @brief Fuzzer program for cellToPoint and cellToBoundary
 */

#include "h3api.h"
#include "utility.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error("Should have one argument (test case file)\n");
    }
    const char* filename = argv[1];
    FILE* fp = fopen(filename, "rb");
    H3Index index;
    if (fread(&index, sizeof(H3Index), 1, fp) != 1) {
        error("Error reading\n");
    }
    fclose(fp);

    GeoPoint geo;
    H3_EXPORT(cellToLatLng)(index, &geo);
    printf("%lf %lf\n", geo.lat, geo.lon);
    CellBoundary cellBoundary;
    H3_EXPORT(cellToBoundary)(index, &cellBoundary);
    printf("%d\n", cellBoundary.numVerts);
}

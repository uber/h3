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
#include "benchmark.h"
#include "h3api.h"

H3Index h = 0x89283080ddbffff;
H3Index p = 0x89080000003ffff;

DECLSPEC int64_t H3_EXPORT(cellToChildrenSize)(H3Index h, int childRes);

/** @brief provides the children (or grandchildren, etc) of the given cell */
DECLSPEC void H3_EXPORT(cellToChildren)(H3Index h, int childRes,
                                        H3Index *children);

BEGIN_BENCHMARKS();

int childRes = 14;
int64_t numChildren = H3_EXPORT(cellToChildrenSize)(h, childRes);

H3Index *children = calloc(numChildren, sizeof(H3Index));
H3_EXPORT(cellToChildren)(h, childRes, children);

BENCHMARK(hexagonChildren, 10, {
    for (int64_t i = 0; i < numChildren; i++) {
        H3_EXPORT(isValidCell)(children[i]);
    }
});

free(children);

END_BENCHMARKS();

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
#include "benchmark.h"
#include "iterators.h"

H3Index hex2 = 0x820887fffffffff;   // res 2 hexagon
H3Index pent2 = 0x820807fffffffff;  // res 2 pentagon

void exhaustGosper(H3Index cell, int childRes) {
    IterEdgesGosper iter = iterInitGosper(cell, childRes);
    while (iter.e) iterStepGosper(&iter);
}

BEGIN_BENCHMARKS();

// res: 2 -> 2
BENCHMARK(hex_plus0, 50000000, { exhaustGosper(hex2, 2); });
BENCHMARK(pent_plus0, 50000000, { exhaustGosper(pent2, 2); });

// res: 2 -> 3
BENCHMARK(hex_plus1, 500000, { exhaustGosper(hex2, 3); });
BENCHMARK(pent_plus1, 500000, { exhaustGosper(pent2, 3); });

// res: 2 -> 4
BENCHMARK(hex_plus2, 100000, { exhaustGosper(hex2, 4); });
BENCHMARK(pent_plus2, 100000, { exhaustGosper(pent2, 4); });

// res: 2 -> 7
BENCHMARK(hex_plus5, 1000, { exhaustGosper(hex2, 7); });
BENCHMARK(pent_plus5, 1000, { exhaustGosper(pent2, 7); });

// res: 2 -> 10
BENCHMARK(hex_plus8, 100, { exhaustGosper(hex2, 10); });
BENCHMARK(pent_plus8, 100, { exhaustGosper(pent2, 10); });

// res: 2 -> 13
BENCHMARK(hex_plus11, 10, { exhaustGosper(hex2, 13); });
BENCHMARK(pent_plus11, 10, { exhaustGosper(pent2, 13); });

END_BENCHMARKS();

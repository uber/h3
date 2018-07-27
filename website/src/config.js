/*
 * Copyright 2018 Uber Technologies, Inc.
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

export const PROJECT_TYPE = 'github';
export const PROJECT_NAME = 'h3';
export const PROJECT_ORG = 'uber';
export const PROJECT_URL = `https://github.com/${PROJECT_ORG}/${PROJECT_NAME}`;
export const PROJECT_DESC = 'A hexagonal hierarchical geospatial indexing system';

export const PROJECTS = {
  'H3': 'https://uber.github.io/h3'
};

export const HOME_HEADING =
  'A hexagonal hierarchical geospatial indexing system.';

/* eslint-disable quotes */
export const HOME_BULLETS = [{
  text: 'Partition data into a hexagonal grid',
  desc: `H3 has an easy API for indexing coordinates into a hexagonal, global grid.`,
  img: 'images/icon-high-precision.svg'
}, {
  text: 'Hierarchical grid system',
  desc: `Easy, bitwise truncation to coarser, approximate cells, along with area compression/decompression \
algorithms.`,
  img: 'images/icon-layers.svg'
}, {
  text: 'Fully global',
  desc: `Along with twelve pentagons, the entire world is addressable in H3, down to square meter resolution.`,
  img: 'images/icon-chart.svg'
}];

export const ADDITIONAL_LINKS = [];

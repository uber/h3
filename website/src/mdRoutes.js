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

import mainpage from '../../docs/overview/mainpage.md';
import usecases from '../../docs/overview/usecases.md';

import overview from '../../docs/core-library/overview.md';
import usage from '../../docs/core-library/usage.md';
import coordsystems from '../../docs/core-library/coordsystems.md';
import filters from '../../docs/core-library/filters.md';
import geoToH3desc from '../../docs/core-library/geoToH3desc.md';
import h3Indexing from '../../docs/core-library/h3indexing.md';
import h3ToGeoBoundaryDesc from '../../docs/core-library/h3ToGeoBoundaryDesc.md';
import h3ToGeoDesc from '../../docs/core-library/h3ToGeoDesc.md';
import restable from '../../docs/core-library/restable.md';

import api_hierarchy from '../../docs/api/hierarchy.md';
import api_indexing from '../../docs/api/indexing.md';
import api_inspection from '../../docs/api/inspection.md';
import api_misc from '../../docs/api/misc.md';
import api_traversal from '../../docs/api/traversal.md';
import api_regions from '../../docs/api/regions.md';
import api_uniedge from '../../docs/api/uniedge.md';

import community_bindings from '../../docs/community/bindings.md';
import community_libraries from '../../docs/community/libraries.md';
import community_applications from '../../docs/community/applications.md';
import community_tutorials from '../../docs/community/tutorials.md';

export default [{
    name: 'Documentation',
    path: '/documentation',
    data: [{
        name: 'Overview',
        children: [
            {
                name: 'Introduction',
                markdown: mainpage
            },
            {
                name: 'Use Cases',
                markdown: usecases
            }
        ]
    }, {
        name: 'Core Library',
        children: [
            {
                name: 'Overview',
                markdown: overview
            },
            {
                name: 'Coordinate Systems',
                markdown: coordsystems
            },
            {
                name: 'Unix-Style Filters',
                markdown: filters
            },
            {
                name: 'H3 Index Representations',
                markdown: h3Indexing
            },
            {
                name: 'Walkthrough of geoToH3',
                markdown: geoToH3desc
            },
            {
                name: 'Walkthrough of h3ToGeoBoundary',
                markdown: h3ToGeoBoundaryDesc
            },
            {
                name: 'Walkthrough of h3ToGeo',
                markdown: h3ToGeoDesc
            },
            {
                name: 'Using H3',
                markdown: usage
            },
            {
                name: 'Resolution Table',
                markdown: restable
            }
        ]
    }, {
        name: 'API Reference',
        children: [
            {
                name: 'Indexing',
                markdown: api_indexing
            },
            {
                name: 'Inspection',
                markdown: api_inspection
            },
            {
                name: 'Traversal',
                markdown: api_traversal
            },
            {
                name: 'Hierarchy',
                markdown: api_hierarchy
            },
            {
                name: 'Regions',
                markdown: api_regions
            },
            {
                name: 'Unidirectional Edges',
                markdown: api_uniedge
            },
            {
                name: 'Miscellaneous',
                markdown: api_misc
            }
        ]
    }, {
        name: 'Community',
        children: [
            {
                name: 'Bindings',
                markdown: community_bindings
            },
            {
                name: 'Libraries Using H3',
                markdown: community_libraries
            },
            {
                name: 'Applications Using H3',
                markdown: community_applications
            },
            {
                name: 'Tutorials',
                markdown: community_tutorials
            }
        ]
    }]
}];

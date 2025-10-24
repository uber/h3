/*
 * Copyright 2017, 2020-2021 Uber Technologies, Inc.
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
/** @file benchmarkPolygonToCellsExperimentalGeodesic.c
 * @brief Benchmarks the experimental polygon-to-cells traversal (geodesic).
 */
#include <stdbool.h>
#include <stdlib.h>

#include "algos.h"
#include "benchmark.h"
#include "h3api.h"
#include "polyfill.h"
#include "polygon.h"

// Colorado state
LatLng stateVerts[] = {
    {0.645778804554910, -1.903190792178713},
    {0.645682811446050, -1.780975856637062},
    {0.715595465293187, -1.781167842854781},
    {0.715578012000667, -1.903262350678044},
    {0.645778804554910, -1.903190792178713}
};
GeoLoop stateGeoLoop;
GeoPolygon stateGeoPolygon;

// Large ellipse approx 1800x1100km
LatLng largeEllipseVerts[] = {
    {0.698131700797732, -1.658062789394613},  {0.697959500367918, -1.647103776346370},
    {0.697443578674904, -1.636188013515931},  {0.696585971825909, -1.625358580432251},
    {0.695390064403362, -1.614658215920214},  {0.693860576107503, -1.604129149430017},
    {0.692003543129878, -1.593812934376823},  {0.689826294331253, -1.583750284148419},
    {0.687337422317945, -1.573980911428067},  {0.684546749530733, -1.564543371466692},
    {0.681465289480172, -1.555474909922921},  {0.678105203281295, -1.546811315871481},
    {0.674479751659253, -1.538586780560073},  {0.670603242615285, -1.530833762472139},
    {0.666490974959581, -1.523582859228054},  {0.662159177933861, -1.516862686830300},
    {0.657624947161976, -1.510699766729177},  {0.652906177181289, -1.505118421154754},
    {0.648021490821112, -1.500140677128138},  {0.642990165706910, -1.495786179530887},
    {0.637832058180313, -1.492072113575638},  {0.632567524935215, -1.489013136983919},
    {0.627217342679196, -1.486621322138826},  {0.621802626137356, -1.484906108440836},
    {0.616344744722137, -1.483874265054807},  {0.610865238198015, -1.483529864195180},
    {0.605385731673894, -1.483874265054807},  {0.599927850258674, -1.484906108440836},
    {0.594513133716834, -1.486621322138826},  {0.589162951460816, -1.489013136983919},
    {0.583898418215717, -1.492072113575638},  {0.578740310689120, -1.495786179530887},
    {0.573708985574918, -1.500140677128138},  {0.568824299214742, -1.505118421154754},
    {0.564105529234055, -1.510699766729177},  {0.559571298462169, -1.516862686830300},
    {0.555239501436449, -1.523582859228054},  {0.551127233780745, -1.530833762472139},
    {0.547250724736778, -1.538586780560073},  {0.543625273114736, -1.546811315871481},
    {0.540265186915859, -1.555474909922921},  {0.537183726865297, -1.564543371466692},
    {0.534393054078086, -1.573980911428067},  {0.531904182064778, -1.583750284148419},
    {0.529726933266152, -1.593812934376823},  {0.527869900288528, -1.604129149430017},
    {0.526340411992668, -1.614658215920214},  {0.525144504570122, -1.625358580432251},
    {0.524286897721127, -1.636188013515931},  {0.523770976028112, -1.647103776346370},
    {0.523598775598299, -1.658062789394613},  {0.523770976028112, -1.669021802442856},
    {0.524286897721127, -1.679937565273295},  {0.525144504570122, -1.690766998356975},
    {0.526340411992668, -1.701467362869012},  {0.527869900288528, -1.711996429359209},
    {0.529726933266152, -1.722312644412403},  {0.531904182064778, -1.732375294640808},
    {0.534393054078086, -1.742144667361160},  {0.537183726865297, -1.751582207322534},
    {0.540265186915859, -1.760650668866305},  {0.543625273114736, -1.769314262917745},
    {0.547250724736778, -1.777538798229153},  {0.551127233780745, -1.785291816317088},
    {0.555239501436449, -1.792542719561173},  {0.559571298462169, -1.799262891958926},
    {0.564105529234055, -1.805425812060049},  {0.568824299214742, -1.811007157634472},
    {0.573708985574918, -1.815984901661088},  {0.578740310689120, -1.820339399258339},
    {0.583898418215717, -1.824053465213588},  {0.589162951460816, -1.827112441805307},
    {0.594513133716834, -1.829504256650400},  {0.599927850258674, -1.831219470348391},
    {0.605385731673894, -1.832251313734419},  {0.610865238198015, -1.832595714594046},
    {0.616344744722137, -1.832251313734419},  {0.621802626137356, -1.831219470348391},
    {0.627217342679196, -1.829504256650400},  {0.632567524935215, -1.827112441805307},
    {0.637832058180313, -1.824053465213588},  {0.642990165706910, -1.820339399258339},
    {0.648021490821112, -1.815984901661088},  {0.652906177181289, -1.811007157634472},
    {0.657624947161976, -1.805425812060049},  {0.662159177933861, -1.799262891958926},
    {0.666490974959581, -1.792542719561173},  {0.670603242615285, -1.785291816317088},
    {0.674479751659253, -1.777538798229153},  {0.678105203281295, -1.769314262917745},
    {0.681465289480172, -1.760650668866305},  {0.684546749530733, -1.751582207322534},
    {0.687337422317945, -1.742144667361160},  {0.689826294331253, -1.732375294640808},
    {0.692003543129878, -1.722312644412403},  {0.693860576107503, -1.711996429359210},
    {0.695390064403362, -1.701467362869012},  {0.696585971825909, -1.690766998356975},
    {0.697443578674904, -1.679937565273295},  {0.697959500367918, -1.669021802442856}};
GeoLoop largeEllipseGeoLoop;
GeoPolygon largeEllipseGeoPolygon;

// London to NY flight
LatLng londonNyVerts[] = {{-0.0022305307840, 0.8989737191417},
                          {-1.2916483662309, 0.7105724077059},
                          {-0.0022305307840, 0.8989737191417}};
GeoLoop londonNyGeoLoop;
GeoPolygon londonNyGeoPolygon;

BEGIN_BENCHMARKS();

stateGeoLoop.numVerts = 5;
stateGeoLoop.verts = stateVerts;
stateGeoPolygon.geoloop = stateGeoLoop;

largeEllipseGeoLoop.numVerts = 100;
largeEllipseGeoLoop.verts = largeEllipseVerts;
largeEllipseGeoPolygon.geoloop = largeEllipseGeoLoop;

londonNyGeoLoop.numVerts = 6;
londonNyGeoLoop.verts = londonNyVerts;
londonNyGeoPolygon.geoloop = londonNyGeoLoop;

void polygonToCellsBenchmark(const GeoPolygon *polygon, int resolution,
                                    bool geodesic) {
    uint32_t flags = CONTAINMENT_OVERLAPPING;
    if (geodesic) {
        FLAG_SET_GEODESIC(flags);
    }

    int64_t numHexagons = 0;
    H3_EXPORT(maxPolygonToCellsSizeExperimental)
    (polygon, resolution, flags, &numHexagons);
    H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));
    H3_EXPORT(polygonToCellsExperimental)
    (polygon, resolution, flags, numHexagons, hexagons);
    free(hexagons);
}

const int stateResolution = 5;
const int largeEllipseResolution = 4;
const int londonNyResolution = 3;

BENCHMARK(polygonToCellsState_PlanarOverlapping, 30, {
    polygonToCellsBenchmark(&stateGeoPolygon, stateResolution, false);
});

BENCHMARK(polygonToCellsState_GeodesicOverlapping, 30, {
    polygonToCellsBenchmark(&stateGeoPolygon, stateResolution, true);
});

BENCHMARK(polygonToCellsLargeEllipse_PlanarOverlapping, 50, {
    polygonToCellsBenchmark(&largeEllipseGeoPolygon, largeEllipseResolution,
                            false);
});

BENCHMARK(polygonToCellsLargeEllipse_GeodesicOverlapping, 50, {
    polygonToCellsBenchmark(&largeEllipseGeoPolygon, largeEllipseResolution,
                            true);
});

BENCHMARK(polygonToCellsLondonNY_PlanarOverlapping, 100, {
    polygonToCellsBenchmark(&londonNyGeoPolygon, londonNyResolution, false);
});

BENCHMARK(polygonToCellsLondonNY_GeodesicOverlapping, 100, {
    polygonToCellsBenchmark(&londonNyGeoPolygon, londonNyResolution, true);
});

END_BENCHMARKS();

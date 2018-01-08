/*
 * Copyright 2017 Uber Technologies, Inc.
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
#include "algos.h"
#include "benchmark.h"
#include "h3api.h"

// Fixtures
GeoCoord sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
Geofence sfGeofence;
GeoPolygon sfGeoPolygon;

GeoCoord alamedaVerts[] = {{0.6597959342671712, -2.133241848488897},
                           {0.6597959348850178, -2.133241848495878},
                           {0.6598352639563587, -2.1331688423977755},
                           {0.6601346536539207, -2.13270417124178},
                           {0.6601594763880223, -2.1326680320633344},
                           {0.6601512007732382, -2.1326594176574534},
                           {0.6598535076212304, -2.1323049630593562},
                           {0.6596565748646488, -2.132069889917591},
                           {0.6594645035394391, -2.131843148468039},
                           {0.6593438094209757, -2.1316994860539844},
                           {0.6591174422311021, -2.131429776816562},
                           {0.658849344286881, -2.1311111485483867},
                           {0.6588348862079956, -2.1310988536794455},
                           {0.6586273138317915, -2.131668420800747},
                           {0.6583729538174264, -2.132370426573979},
                           {0.6582479206289285, -2.132718691911663},
                           {0.6582322393220743, -2.1327614200082317},
                           {0.6583003647098981, -2.132837478687196},
                           {0.6584457274847966, -2.132827956758973},
                           {0.6585526679060995, -2.1330231566043203},
                           {0.6587379099516777, -2.1331602726234538},
                           {0.6587273684736642, -2.1332676321559063},
                           {0.6584638025857692, -2.133305719954319},
                           {0.6583545950288919, -2.1334323622944993},
                           {0.6584427148370682, -2.1335885223323947},
                           {0.6584715236640714, -2.133649780409862},
                           {0.6584715242505019, -2.133649780481421},
                           {0.658474662092443, -2.1336459234695804},
                           {0.6591666596433436, -2.1348354004882926},
                           {0.6591809355063646, -2.1348424115474565},
                           {0.6593477498700266, -2.1351460576998926},
                           {0.6597155087395117, -2.1351049454274},
                           {0.6597337410387994, -2.135113899444683},
                           {0.6598277083823935, -2.1351065432309517},
                           {0.659837290351688, -2.1350919904836627},
                           {0.6598391300107502, -2.1350911731005957},
                           {0.6598335712627461, -2.1350732321630828},
                           {0.6597162034032434, -2.134664026354221},
                           {0.6596785831942451, -2.134651647657116},
                           {0.6596627824684727, -2.13458880305965},
                           {0.6596785832500957, -2.134530719130462},
                           {0.6596093592822273, -2.13428052987356},
                           {0.6596116166352313, -2.134221493755564},
                           {0.6595973199434513, -2.134146270344056},
                           {0.6595536764042369, -2.1340805688066653},
                           {0.6594611172376618, -2.133753252031165},
                           {0.6594829406269346, -2.1337342082305697},
                           {0.6594897134102581, -2.1337104032834757},
                           {0.6597920983773051, -2.1332343063312775},
                           {0.6597959342671712, -2.133241848488897}};
Geofence alamedaGeofence;
GeoPolygon alamedaGeoPolygon;

GeoCoord southernVerts[] = {{0.6367481147484843, -2.1290865397798906},
                            {0.6367481152301953, -2.129086539469222},
                            {0.6367550754426818, -2.128887436716856},
                            {0.6367816002113981, -2.1273204058681094},
                            {0.6380814125349741, -2.127201274803692},
                            {0.6388614350074809, -2.12552061082428},
                            {0.6393520289210095, -2.124274316938293},
                            {0.639524834205869, -2.122168447308359},
                            {0.6405714857447717, -2.122083222593005},
                            {0.640769478635285, -2.120979885974894},
                            {0.6418936996869471, -2.1147667448862255},
                            {0.6419094141707652, -2.1146521242709584},
                            {0.6269997808948107, -2.1038647304637257},
                            {0.6252080524974937, -2.1195521728170457},
                            {0.626379700264057, -2.1203708632511162},
                            {0.6282200029232767, -2.1210412050690723},
                            {0.6283657301211779, -2.1219496416754393},
                            {0.6305651783819565, -2.123628532238016},
                            {0.6308259852882764, -2.124225549648211},
                            {0.6317049665784865, -2.124887756638367},
                            {0.6323403882676475, -2.1266205835454053},
                            {0.6334397909415498, -2.1277211741619553},
                            {0.6367481147484843, -2.1290865397798906}};
Geofence southernGeofence;
GeoPolygon southernGeoPolygon;

BEGIN_BENCHMARKS();

sfGeofence.numVerts = 6;
sfGeofence.verts = sfVerts;
sfGeoPolygon.geofence = sfGeofence;

alamedaGeofence.numVerts = 50;
alamedaGeofence.verts = alamedaVerts;
alamedaGeoPolygon.geofence = alamedaGeofence;

southernGeofence.numVerts = 23;
southernGeofence.verts = southernVerts;
southernGeoPolygon.geofence = southernGeofence;

int numHexagons;
H3Index* hexagons;

BENCHMARK(polyfillSF, 500, {
    numHexagons = H3_EXPORT(maxPolyfillSize)(&sfGeoPolygon, 9);
    hexagons = calloc(numHexagons, sizeof(H3Index));
    H3_EXPORT(polyfill)(&sfGeoPolygon, 9, hexagons);
    free(hexagons);
});

BENCHMARK(polyfillAlameda, 500, {
    numHexagons = H3_EXPORT(maxPolyfillSize)(&alamedaGeoPolygon, 9);
    hexagons = calloc(numHexagons, sizeof(H3Index));
    H3_EXPORT(polyfill)(&alamedaGeoPolygon, 9, hexagons);
    free(hexagons);
});

BENCHMARK(polyfillSouthernExpansion, 10, {
    numHexagons = H3_EXPORT(maxPolyfillSize)(&southernGeoPolygon, 9);
    hexagons = calloc(numHexagons, sizeof(H3Index));
    H3_EXPORT(polyfill)(&southernGeoPolygon, 9, hexagons);
    free(hexagons);
});

END_BENCHMARKS();

/*
 * Copyright 2018, 2020-2021 Uber Technologies, Inc.
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
/**
 * Example program that calculates the distance in hexagons and in kilometers
 * between two hexagon indices.
 */

// mean Earth radius
#define R 6371.0088

#include <assert.h>
#include <h3/h3api.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

/**
 * @brief haversineDistance finds the
 * [great-circle distance](https://en.wikipedia.org/wiki/Great-circle_distance)
 * between two points on a sphere.
 * @see https://en.wikipedia.org/wiki/Haversine_formula.
 *
 * Parameters are the latitude and longitude of the first and second point in
 * radians, respectively.
 *
 * @return the great-circle distance in kilometers.
 */
double haversineDistance(double th1, double ph1, double th2, double ph2) {
    double dx, dy, dz;
    ph1 -= ph2;

    dz = sin(th1) - sin(th2);
    dx = cos(ph1) * cos(th1) - cos(th2);
    dy = sin(ph1) * cos(th1);
    return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

int main(int argc, char *argv[]) {
    // 1455 Market St @ resolution 15
    H3Index h3HQ1 = 0;
    stringToH3("8f2830828052d25", &h3HQ1);
    // 555 Market St @ resolution 15
    H3Index h3HQ2 = 0;
    stringToH3("8f283082a30e623", &h3HQ2);

    LatLng geoHQ1 = {0};
    LatLng geoHQ2 = {0};
    cellToLatLng(h3HQ1, &geoHQ1);
    cellToLatLng(h3HQ2, &geoHQ2);

    int64_t distance = 0;
    assert(gridDistance(h3HQ1, h3HQ2, &distance) == E_SUCCESS);

    printf(
        "origin: (%lf, %lf)\n"
        "destination: (%lf, %lf)\n"
        "grid distance: %" PRId64
        "\n"
        "distance in km: %lfkm\n",
        radsToDegs(geoHQ1.lat), radsToDegs(geoHQ1.lng), radsToDegs(geoHQ2.lat),
        radsToDegs(geoHQ2.lng), distance,
        haversineDistance(geoHQ1.lat, geoHQ1.lng, geoHQ2.lat, geoHQ2.lng));
    // Output:
    // origin: (37.775236, -122.419755)
    // destination: (37.789991, -122.402121)
    // grid distance: 2340
    // distance in km: 2.256853km
}
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "adder.h"
#include "algos.h"
#include "area.h"
#include "h3api.h"
#include "iterators.h"
#include "test.h"
#include "utility.h"

static inline double relative_diff(double a, double b) {
    double diff = fabs(a - b);
    double denom = fmax(fabs(a), fabs(b));

    // If both numbers are extremely close to zero, fall back to absolute diff
    if (denom < DBL_EPSILON) {
        return diff;
    }

    return diff / denom;
}

static double get_loop_area(GeoLoop loop) {
    double area;
    t_assertSuccess(geoLoopAreaRads2(loop, &area));

    t_assert(area >= 0, "Area should be nonnegative");
    t_assert(area < 4.0 * M_PI, "Area should be less than entire globe");

    t_assert(loop.numVerts >= 3, "Every loop should have at least 3 vertexes.");

    return area;
}

static double get_outer_loop_area(GeoPolygon poly) {
    return get_loop_area(poly.geoloop);
}

static void check_poly(GeoPolygon poly) {
    double area;
    t_assertSuccess(geoPolygonAreaRads2(poly, &area));

    t_assert(area >= 0, "Area should be nonnegative");
    t_assert(area < 4.0 * M_PI, "Area should be less than entire globe");
    t_assert(area <= get_outer_loop_area(poly),
             "Total area should be less than the outer loop area");

    // The outer ring and holes should be ordered in 'increasing' order; that
    // is, since the holes are oriented clockwise, they will naively enclose
    // more area than the outer ring, which is oriented counterclockwise.
    if (poly.numHoles > 0) {
        t_assert(get_loop_area(poly.geoloop) <= get_loop_area(poly.holes[0]),
                 "Outer loop should have 'less' area than first hole.");
    }
    for (int i = 0; i < poly.numHoles - 1; i++) {
        t_assert(
            get_loop_area(poly.holes[i]) <= get_loop_area(poly.holes[i + 1]),
            "Polygon holes should be listed in 'increasing' order.");
    }
}

static GeoMultiPolygon get_mpoly(H3Index *cells, uint64_t num_cells) {
    double rel_tol = 1e-7;
    GeoMultiPolygon mpoly;
    t_assertSuccess(H3_EXPORT(cellsToMultiPolygon)(cells, num_cells, &mpoly));

    for (int i = 0; i < mpoly.numPolygons - 1; i++) {
        t_assert(get_outer_loop_area(mpoly.polygons[i]) >=
                     get_outer_loop_area(mpoly.polygons[i + 1]),
                 "Polygons should ordered by area enclosed by outer loop; "
                 "decreasing.");
    }

    for (int i = 0; i < mpoly.numPolygons; i++) {
        check_poly(mpoly.polygons[i]);
    }

    {
        // Check that area matches sum of areas of cells
        double poly_area;
        geoMultiPolygonAreaRads2(mpoly, &poly_area);

        Adder k = {};
        for (int i = 0; i < num_cells; i++) {
            double temp;
            H3_EXPORT(cellAreaRads2)(cells[i], &temp);
            kadd(&k, temp);
        }
        double cell_area = k.sum;
        t_assert(relative_diff(cell_area, poly_area) <= rel_tol,
                 "Polygon area should match summing area of cells");
    }

    return mpoly;
}

static void check_cell(H3Index cell) {
    GeoMultiPolygon mpoly = get_mpoly(&cell, 1);
    t_assert(mpoly.numPolygons == 1, "Exactly one polygon.");
    t_assert(mpoly.polygons[0].numHoles == 0,
             "Individual cells should have zero holes.");
    t_assert(mpoly.polygons[0].geoloop.numVerts >= 5,
             "Individual cells should have at least 5 verices");
    t_assert(mpoly.polygons[0].geoloop.numVerts <= 10,
             "Individual cells should never have more than 10 verices");
    H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
}

static void check_global_poly(GeoMultiPolygon mpoly) {
    t_assert(mpoly.numPolygons == 8, "Expecting 8 polygons");
    for (int i = 0; i < 8; i++) {
        t_assert((mpoly.polygons[i].numHoles == 0) &&
                     (mpoly.polygons[i].holes == NULL) &&
                     (mpoly.polygons[i].geoloop.numVerts = 3),
                 "Expecting each polygon is a triangle");
    }

    double area;
    geoMultiPolygonAreaRads2(mpoly, &area);
    // TODO: probably need some error tolerance, but let's wait to see it fail
    // first
    t_assert(area == 4 * M_PI, "Exact area expected");
}

SUITE(edgeCellToPoly) {
    TEST(three_polygons) {
        double rel_tol = 1e-15;
        // Results in 3 polygons: 0 holes, 1 hole, 3 holes
        H3Index cells[] = {
            0x8027fffffffffff, 0x802bfffffffffff, 0x804dfffffffffff,
            0x8067fffffffffff, 0x806dfffffffffff, 0x8049fffffffffff,
            0x805ffffffffffff, 0x8057fffffffffff, 0x807dfffffffffff,
            0x80a5fffffffffff, 0x80a9fffffffffff, 0x808bfffffffffff,
            0x801bfffffffffff, 0x8035fffffffffff, 0x803ffffffffffff,
            0x8053fffffffffff, 0x8043fffffffffff, 0x8021fffffffffff,
            0x8011fffffffffff, 0x801ffffffffffff, 0x8097fffffffffff,
        };
        uint64_t num_cells = ARRAY_SIZE(cells);

        GeoMultiPolygon mpoly = get_mpoly(cells, num_cells);
        t_assert(mpoly.numPolygons == 3, "expecting 3 polygons");

        t_assert(mpoly.polygons[0].numHoles == 3, "3 holes in first poly");
        t_assert(mpoly.polygons[1].numHoles == 1, "1 hole in second poly");
        t_assert(mpoly.polygons[2].numHoles == 0, "0 holes in third poly");

        double area;
        double expected = 2.2440497074541694;
        geoMultiPolygonAreaRads2(mpoly, &area);
        t_assert(relative_diff(area, expected) < rel_tol, "Expected area");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(cells_at_res) {
        iterateAllIndexesAtRes(0, check_cell);
        iterateAllIndexesAtRes(1, check_cell);
        iterateAllIndexesAtRes(2, check_cell);
    }

    TEST(res15_hex) {
        // 0x8f754e64992d6d8 is a res 15 *hex*
        H3Index h = 0x8f754e64992d6d8;
        check_cell(h);
    }

    // check all pentagons at all resolutions
    TEST(all_pentagons) {
        H3Index cells[12];
        for (int res = 0; res <= 15; res++) {
            H3_EXPORT(getPentagons)(res, cells);
            for (int i = 0; i < 12; i++) {
                check_cell(cells[i]);
            }
        }
    }

    TEST(issue_1049) {
        // from https://github.com/uber/h3/issues/1049
        H3Index cells[] = {
            0x827487fffffffff, 0x82748ffffffffff, 0x827497fffffffff,
            0x82749ffffffffff, 0x8274affffffffff, 0x8274c7fffffffff,
            0x8274cffffffffff, 0x8274d7fffffffff, 0x8274e7fffffffff,
            0x8274effffffffff, 0x8274f7fffffffff, 0x82754ffffffffff,
            0x827c07fffffffff, 0x827c27fffffffff, 0x827c2ffffffffff,
            0x827c37fffffffff, 0x827d87fffffffff, 0x827d8ffffffffff,
            0x827d97fffffffff, 0x827d9ffffffffff, 0x827da7fffffffff,
            0x827daffffffffff, 0x82801ffffffffff, 0x8280a7fffffffff,
            0x8280affffffffff, 0x8280b7fffffffff, 0x828197fffffffff,
            0x82819ffffffffff, 0x8281a7fffffffff, 0x8281b7fffffffff,
            0x828207fffffffff, 0x82820ffffffffff, 0x828227fffffffff,
            0x82822ffffffffff, 0x8282e7fffffffff, 0x828307fffffffff,
            0x82830ffffffffff, 0x82831ffffffffff, 0x82832ffffffffff,
            0x828347fffffffff, 0x82834ffffffffff, 0x828357fffffffff,
            0x82835ffffffffff, 0x828367fffffffff, 0x828377fffffffff,
            0x82a447fffffffff, 0x82a457fffffffff, 0x82a45ffffffffff,
            0x82a467fffffffff, 0x82a46ffffffffff, 0x82a477fffffffff,
            0x82a4c7fffffffff, 0x82a4cffffffffff, 0x82a4d7fffffffff,
            0x82a4e7fffffffff, 0x82a4effffffffff, 0x82a4f7fffffffff,
            0x82a547fffffffff, 0x82a54ffffffffff, 0x82a557fffffffff,
            0x82a55ffffffffff, 0x82a567fffffffff, 0x82a577fffffffff,
            0x82a837fffffffff, 0x82a897fffffffff, 0x82a8a7fffffffff,
            0x82a8b7fffffffff, 0x82a917fffffffff, 0x82a927fffffffff,
            0x82a937fffffffff, 0x82a987fffffffff, 0x82a98ffffffffff,
            0x82a997fffffffff, 0x82a99ffffffffff, 0x82a9a7fffffffff,
            0x82a9affffffffff, 0x82ac47fffffffff, 0x82ac57fffffffff,
            0x82ac5ffffffffff, 0x82ac67fffffffff, 0x82ac6ffffffffff,
            0x82ac77fffffffff, 0x82ad47fffffffff, 0x82ad4ffffffffff,
            0x82ad57fffffffff, 0x82ad5ffffffffff, 0x82ad67fffffffff,
            0x82ad77fffffffff, 0x82c207fffffffff, 0x82c217fffffffff,
            0x82c227fffffffff, 0x82c237fffffffff, 0x82c287fffffffff,
            0x82c28ffffffffff, 0x82c29ffffffffff, 0x82c2a7fffffffff,
            0x82c2affffffffff, 0x82c2b7fffffffff, 0x82c307fffffffff,
            0x82c317fffffffff, 0x82c31ffffffffff, 0x82c337fffffffff,
            0x82cfb7fffffffff, 0x82d0c7fffffffff, 0x82d0d7fffffffff,
            0x82d0dffffffffff, 0x82d0e7fffffffff, 0x82d0f7fffffffff,
            0x82d147fffffffff, 0x82d157fffffffff, 0x82d15ffffffffff,
            0x82d167fffffffff, 0x82d177fffffffff, 0x82d187fffffffff,
            0x82d18ffffffffff, 0x82d197fffffffff, 0x82d19ffffffffff,
            0x82d1a7fffffffff, 0x82d1affffffffff, 0x82dc47fffffffff,
            0x82dc57fffffffff, 0x82dc5ffffffffff, 0x82dc67fffffffff,
            0x82dc6ffffffffff, 0x82dc77fffffffff, 0x82dcc7fffffffff,
            0x82dccffffffffff, 0x82dcd7fffffffff, 0x82dce7fffffffff,
            0x82dceffffffffff, 0x82dcf7fffffffff, 0x82dd1ffffffffff,
            0x82dd47fffffffff, 0x82dd4ffffffffff, 0x82dd57fffffffff,
            0x82dd5ffffffffff, 0x82dd6ffffffffff, 0x82dd87fffffffff,
            0x82dd8ffffffffff, 0x82dd97fffffffff, 0x82dd9ffffffffff,
            0x82ddaffffffffff, 0x82ddb7fffffffff, 0x82dec7fffffffff,
            0x82decffffffffff, 0x82ded7fffffffff, 0x82dee7fffffffff,
            0x82deeffffffffff, 0x82def7fffffffff, 0x82df0ffffffffff,
            0x82df1ffffffffff, 0x82df47fffffffff, 0x82df4ffffffffff,
            0x82df57fffffffff, 0x82df5ffffffffff, 0x82df77fffffffff,
            0x82df8ffffffffff, 0x82df9ffffffffff, 0x82e6c7fffffffff,
            0x82e6cffffffffff, 0x82e6d7fffffffff, 0x82e6dffffffffff,
            0x82e6effffffffff, 0x82e6f7fffffffff,
        };

        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 12, "expecting 12 polygons");

        for (int i = 0; i < mpoly.numPolygons; i++) {
            t_assert(mpoly.polygons[i].numHoles == 0, "expecting 0 holes");
        }

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    // A "global polygon" example.
    TEST(equator_cells) {
        H3Index cells[] = {
            0x81807ffffffffff, 0x817efffffffffff, 0x81723ffffffffff,
            0x817ebffffffffff, 0x817c3ffffffffff, 0x817e3ffffffffff,
            0x817a3ffffffffff, 0x8166fffffffffff, 0x8172bffffffffff,
            0x816afffffffffff, 0x81933ffffffffff, 0x8168fffffffffff,
            0x8188fffffffffff, 0x81853ffffffffff, 0x817f7ffffffffff,
            0x8180bffffffffff, 0x81783ffffffffff, 0x81743ffffffffff,
            0x8170bffffffffff, 0x8173bffffffffff, 0x8179bffffffffff,
            0x817cbffffffffff, 0x8188bffffffffff, 0x81857ffffffffff,
            0x816f7ffffffffff, 0x8177bffffffffff, 0x81617ffffffffff,
            0x816f3ffffffffff, 0x8174bffffffffff, 0x8180fffffffffff,
            0x817a7ffffffffff, 0x81767ffffffffff, 0x81757ffffffffff,
            0x81957ffffffffff, 0x81787ffffffffff, 0x81847ffffffffff,
            0x81653ffffffffff, 0x817bbffffffffff, 0x816cfffffffffff,
            0x816abffffffffff, 0x815f3ffffffffff, 0x817c7ffffffffff,
            0x8168bffffffffff, 0x818cbffffffffff, 0x818cfffffffffff,
            0x818afffffffffff, 0x8174fffffffffff, 0x8172fffffffffff,
            0x8170fffffffffff, 0x816fbffffffffff, 0x81657ffffffffff,
            0x816c7ffffffffff, 0x8186bffffffffff, 0x81763ffffffffff,
            0x818a7ffffffffff, 0x8186fffffffffff, 0x81707ffffffffff,
            0x8182bffffffffff, 0x818f3ffffffffff, 0x8182fffffffffff,
        };
        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 1, "expecting 1 polygon");
        t_assert(mpoly.polygons[0].numHoles == 1, "expecting 1 hole");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    // A "global polygon" example.
    TEST(prime_meridian) {
        H3Index cells[] = {
            0x81efbffffffffff, 0x81c07ffffffffff, 0x81d1bffffffffff,
            0x81097ffffffffff, 0x8109bffffffffff, 0x81d0bffffffffff,
            0x81987ffffffffff, 0x81017ffffffffff, 0x81e67ffffffffff,
            0x81ddbffffffffff, 0x81ac7ffffffffff, 0x8158bffffffffff,
            0x81397ffffffffff, 0x81593ffffffffff, 0x81c17ffffffffff,
            0x81827ffffffffff, 0x81197ffffffffff, 0x81eebffffffffff,
            0x81383ffffffffff, 0x81dcbffffffffff, 0x81757ffffffffff,
            0x81093ffffffffff, 0x81073ffffffffff, 0x8159bffffffffff,
            0x81f17ffffffffff, 0x81187ffffffffff, 0x81007ffffffffff,
            0x81997ffffffffff, 0x81753ffffffffff, 0x81033ffffffffff,
            0x81f2bffffffffff, 0x8138bffffffffff,
        };
        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 1, "expecting 1 polygon");
        t_assert(mpoly.polygons[0].numHoles == 0, "expecting 0 holes");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    // A "global polygon" example.
    TEST(anti_meridian) {
        H3Index cells[] = {
            0x817ebffffffffff, 0x8133bffffffffff, 0x81047ffffffffff,
            0x81f3bffffffffff, 0x81dbbffffffffff, 0x8132bffffffffff,
            0x810cbffffffffff, 0x81bb3ffffffffff, 0x81db3ffffffffff,
            0x81bafffffffffff, 0x81177ffffffffff, 0x817fbffffffffff,
            0x81ba3ffffffffff, 0x815abffffffffff, 0x815bbffffffffff,
            0x81eafffffffffff, 0x81ed7ffffffffff, 0x81057ffffffffff,
            0x819a7ffffffffff, 0x81eabffffffffff, 0x819b7ffffffffff,
            0x81167ffffffffff, 0x81227ffffffffff, 0x8171bffffffffff,
            0x81237ffffffffff, 0x810dbffffffffff, 0x81033ffffffffff,
            0x81f2bffffffffff, 0x8147bffffffffff, 0x81f33ffffffffff,
        };
        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 1, "expecting 1 polygon");
        t_assert(mpoly.polygons[0].numHoles == 0, "expecting 0 holes");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    // A "global polygon" example.
    TEST(both_meridians) {
        H3Index cells[] = {
            0x81efbffffffffff, 0x81c07ffffffffff, 0x81d1bffffffffff,
            0x81097ffffffffff, 0x817ebffffffffff, 0x8133bffffffffff,
            0x81047ffffffffff, 0x8109bffffffffff, 0x81f3bffffffffff,
            0x81d0bffffffffff, 0x81987ffffffffff, 0x81dbbffffffffff,
            0x81017ffffffffff, 0x81e67ffffffffff, 0x81ddbffffffffff,
            0x8132bffffffffff, 0x810cbffffffffff, 0x81bb3ffffffffff,
            0x81ac7ffffffffff, 0x81db3ffffffffff, 0x8158bffffffffff,
            0x81397ffffffffff, 0x81593ffffffffff, 0x81bafffffffffff,
            0x81177ffffffffff, 0x817fbffffffffff, 0x81ba3ffffffffff,
            0x81c17ffffffffff, 0x815abffffffffff, 0x81827ffffffffff,
            0x815bbffffffffff, 0x81eafffffffffff, 0x81197ffffffffff,
            0x81ed7ffffffffff, 0x81eebffffffffff, 0x81383ffffffffff,
            0x81057ffffffffff, 0x819a7ffffffffff, 0x81dcbffffffffff,
            0x81757ffffffffff, 0x81eabffffffffff, 0x81093ffffffffff,
            0x819b7ffffffffff, 0x81073ffffffffff, 0x8159bffffffffff,
            0x8147bffffffffff, 0x81167ffffffffff, 0x81f17ffffffffff,
            0x8171bffffffffff, 0x81227ffffffffff, 0x81187ffffffffff,
            0x81237ffffffffff, 0x81007ffffffffff, 0x810dbffffffffff,
            0x81997ffffffffff, 0x81753ffffffffff, 0x81033ffffffffff,
            0x81f2bffffffffff, 0x8138bffffffffff, 0x81f33ffffffffff,
        };
        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 1, "expecting 1 polygon");
        t_assert(mpoly.polygons[0].numHoles == 1, "expecting 1 hole");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    // A "global polygon" example.
    TEST(meridians_and_equator) {
        H3Index cells[] = {
            0x817c3ffffffffff, 0x81047ffffffffff, 0x8188fffffffffff,
            0x817f7ffffffffff, 0x8180bffffffffff, 0x81177ffffffffff,
            0x817fbffffffffff, 0x8188bffffffffff, 0x815bbffffffffff,
            0x81eafffffffffff, 0x816f3ffffffffff, 0x817a7ffffffffff,
            0x819a7ffffffffff, 0x81757ffffffffff, 0x817bbffffffffff,
            0x816cfffffffffff, 0x8168bffffffffff, 0x81237ffffffffff,
            0x818afffffffffff, 0x8172fffffffffff, 0x816fbffffffffff,
            0x81657ffffffffff, 0x81763ffffffffff, 0x818a7ffffffffff,
            0x81eabffffffffff, 0x8138bffffffffff, 0x8182fffffffffff,
            0x81c07ffffffffff, 0x8109bffffffffff, 0x8166fffffffffff,
            0x81987ffffffffff, 0x8172bffffffffff, 0x8168fffffffffff,
            0x81853ffffffffff, 0x810cbffffffffff, 0x81bb3ffffffffff,
            0x81db3ffffffffff, 0x81743ffffffffff, 0x81bafffffffffff,
            0x8179bffffffffff, 0x818f3ffffffffff, 0x81857ffffffffff,
            0x816f7ffffffffff, 0x8177bffffffffff, 0x8174bffffffffff,
            0x81eebffffffffff, 0x81383ffffffffff, 0x81767ffffffffff,
            0x81787ffffffffff, 0x819b7ffffffffff, 0x8159bffffffffff,
            0x8171bffffffffff, 0x818cbffffffffff, 0x818cfffffffffff,
            0x8170fffffffffff, 0x81707ffffffffff, 0x8147bffffffffff,
            0x81167ffffffffff, 0x81f33ffffffffff, 0x817efffffffffff,
            0x81f3bffffffffff, 0x81017ffffffffff, 0x816afffffffffff,
            0x81e67ffffffffff, 0x81ddbffffffffff, 0x8132bffffffffff,
            0x8170bffffffffff, 0x81ba3ffffffffff, 0x81c17ffffffffff,
            0x815abffffffffff, 0x81617ffffffffff, 0x8180fffffffffff,
            0x81dcbffffffffff, 0x81957ffffffffff, 0x81093ffffffffff,
            0x81847ffffffffff, 0x81653ffffffffff, 0x81073ffffffffff,
            0x8174fffffffffff, 0x810dbffffffffff, 0x81997ffffffffff,
            0x816c7ffffffffff, 0x81033ffffffffff, 0x8186bffffffffff,
            0x81f2bffffffffff, 0x81efbffffffffff, 0x81807ffffffffff,
            0x81d1bffffffffff, 0x81097ffffffffff, 0x817ebffffffffff,
            0x81723ffffffffff, 0x8133bffffffffff, 0x817e3ffffffffff,
            0x817a3ffffffffff, 0x81d0bffffffffff, 0x81dbbffffffffff,
            0x81933ffffffffff, 0x81783ffffffffff, 0x81ac7ffffffffff,
            0x8158bffffffffff, 0x81397ffffffffff, 0x81593ffffffffff,
            0x8173bffffffffff, 0x817cbffffffffff, 0x81827ffffffffff,
            0x81197ffffffffff, 0x81ed7ffffffffff, 0x81057ffffffffff,
            0x816abffffffffff, 0x815f3ffffffffff, 0x81f17ffffffffff,
            0x81227ffffffffff, 0x817c7ffffffffff, 0x81007ffffffffff,
            0x81753ffffffffff, 0x8186fffffffffff, 0x8182bffffffffff,
            0x81187ffffffffff,
        };
        GeoMultiPolygon mpoly = get_mpoly(cells, ARRAY_SIZE(cells));

        t_assert(mpoly.numPolygons == 1, "expecting 1 polygon");
        t_assert(mpoly.polygons[0].numHoles == 3, "expecting 3 holes");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(negative_cells) {
        GeoMultiPolygon mpoly;
        t_assert(H3_EXPORT(cellsToMultiPolygon)(NULL, -1, &mpoly) == E_DOMAIN,
                 "Can't pass in negative number of cells.");
    }

    TEST(empty_cells) {
        GeoMultiPolygon mpoly = get_mpoly(NULL, 0);

        t_assert(mpoly.numPolygons == 0, "expecting 0 polygons");
        t_assert(mpoly.polygons == NULL, "Pointer should be NULL");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(global_poly_area) {
        GeoMultiPolygon mpoly = createGlobeMultiPolygon();

        check_global_poly(mpoly);

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(all_cells) {
        H3Index cells[122];
        H3_EXPORT(getRes0Cells)(cells);

        GeoMultiPolygon mpoly = get_mpoly(cells, 122);

        check_global_poly(mpoly);

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(duplicate_cells) {
        H3Index cells[] = {
            0x81efbffffffffff,
            0x81efbffffffffff,
            0x81efbffffffffff,
        };

        GeoMultiPolygon mpoly;
        H3Error err =
            H3_EXPORT(cellsToMultiPolygon)(cells, ARRAY_SIZE(cells), &mpoly);
        t_assert(err == E_DUPLICATE_INPUT, "Can't have duplicated cells.");
    }

    TEST(multiple_resolutions) {
        H3Index cells[] = {
            0x8027fffffffffff,
            0x81efbffffffffff,
        };

        GeoMultiPolygon mpoly;
        H3Error err =
            H3_EXPORT(cellsToMultiPolygon)(cells, ARRAY_SIZE(cells), &mpoly);
        t_assert(err == E_RES_MISMATCH,
                 "Can't have multiple cell resolutions.");
    }

    TEST(invalid_cells) {
        H3Index cells[] = {
            0x8027fffffffffff,
            0x81efbffffffffff,
        };
        cells[1] += 1;  // make cell invalid

        GeoMultiPolygon mpoly;
        H3Error err =
            H3_EXPORT(cellsToMultiPolygon)(cells, ARRAY_SIZE(cells), &mpoly);
        t_assert(err == E_CELL_INVALID, "Can't have invalid cells.");
    }
}

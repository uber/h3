#include "h3api.h"
#include "test.h"
#include "polygon_to_cells.h"
#include <math.h>

SUITE(polygon_to_cells) {

    TEST(validate_polygon_coordinates) {
        // Test valid polygon
        LatLng verts[] = {
            {0.0, 0.0},
            {0.1, 0.1},
            {0.0, 0.1}
        };
        LatLngPoly polygon = {
            .verts = verts,
            .numVerts = 3
        };
        H3Error err = validatePolygonCoordinates(&polygon);
        t_assert(err == E_SUCCESS, "Valid polygon passes validation");

        // Test invalid latitude
        LatLng invalidVerts[] = {
            {M_PI, 0.0},  // Invalid latitude (90 degrees)
            {0.1, 0.1},
            {0.0, 0.1}
        };
        LatLngPoly invalidPolygon = {
            .verts = invalidVerts,
            .numVerts = 3
        };
        err = validatePolygonCoordinates(&invalidPolygon);
        t_assert(err == E_FAILED, "Invalid latitude fails validation");
    }

    TEST(normalize_coordinates) {
        // Test normalization of coordinates
        LatLng verts[] = {
            {M_PI_2 + 0.1, M_PI + 0.1},  // Beyond valid ranges
            {-M_PI_2 - 0.1, -M_PI - 0.1},
            {0.0, 2 * M_PI}
        };
        LatLngPoly polygon = {
            .verts = verts,
            .numVerts = 3
        };
        
        H3Error err = normalizePolygonCoordinates(&polygon);
        t_assert(err == E_SUCCESS, "Normalization succeeds");
        
        // Check normalized values
        t_assert(fabs(polygon.verts[0].lat - M_PI_2) < 1e-10, 
                "Latitude clamped to 90 degrees");
        t_assert(fabs(polygon.verts[0].lng - (-M_PI + 0.1)) < 1e-10, 
                "Longitude wrapped to -180 degrees");
    }

    TEST(pole_crossing_polygon) {
        // Test polygon that crosses the north pole
        LatLng verts[] = {
            {M_PI_2 - 0.0001, 0.0},
            {M_PI_2 - 0.0001, M_PI_2},
            {M_PI_2 - 0.0001, M_PI},
            {M_PI_2 - 0.0001, -M_PI_2}
        };
        LatLngPoly polygon = {
            .verts = verts,
            .numVerts = 4
        };

        H3Index cells[1000];
        H3Error err = polygonToCells(&polygon, 5, cells);
        t_assert(err == E_SUCCESS, "Pole-crossing polygon processed successfully");
    }

    TEST(complex_polygon_near_pole) {
        // Test the specific case from issue #1000
        LatLng verts[] = {
            {-0.9435, 1.1196},  // (-54.057708, 64.153144)
            {-1.2751, 2.6294},  // (-73.063939, 150.650215)
            {-1.0641, -1.7107}, // (-60.97747, -98.008123)
            {-0.7426, -0.7112}, // (-42.548997, -40.753213)
            {-0.6894, 0.1864}   // (-39.497647, 10.681521)
        };
        LatLngPoly polygon = {
            .verts = verts,
            .numVerts = 5
        };

        H3Index cells[1000];
        H3Error err = polygonToCells(&polygon, 5, cells);
        t_assert(err == E_SUCCESS, 
                "Complex polygon near pole processed successfully");
    }

    TEST(memory_management) {
        // Test memory handling for large polygons
        LatLng* verts = malloc(1000 * sizeof(LatLng));
        for (int i = 0; i < 1000; i++) {
            verts[i].lat = 0.1 * sin(i);
            verts[i].lng = 0.1 * cos(i);
        }
        
        LatLngPoly polygon = {
            .verts = verts,
            .numVerts = 1000
        };

        H3Index* cells = malloc(MAX_POLYGON_CELLS * sizeof(H3Index));
        H3Error err = polygonToCells(&polygon, 5, cells);
        t_assert(err == E_SUCCESS, "Large polygon handled without memory issues");

        free(verts);
        free(cells);
    }
}
#include <stdio.h>
#include <h3api.h>

void printPolygonStructure(LinkedGeoPolygon *polygon) {
    int polygonIndex = 0;
    while (polygon) {
        printf("Polygon %d:\n", polygonIndex);
        int loopIndex = 0;
        LinkedGeoLoop *loop = polygon->first;
        while (loop) {
            int coordCount = 0;
            LinkedLatLng *coord = loop->first;
            while (coord) {
                coordCount++;
                coord = coord->next;
            }
            printf("  Loop %d: %d coordinates\n", loopIndex, coordCount);
            loopIndex++;
            loop = loop->next;
        }
        polygonIndex++;
        polygon = polygon->next;
    }
}

int main() {
    H3Index cells[2] = {
        0x8001fffffffffff,
        0x8003fffffffffff
    };

    printf("Testing with two neighboring cells...\n\n");

    // Test each cell individually
    for (int i = 0; i < 2; i++) {
        LinkedGeoPolygon polygon;
        H3Error error = cellsToLinkedMultiPolygon(&cells[i], 1, &polygon);
        printf("Cell %d alone:\n", i);
        if (error == E_SUCCESS) {
            printPolygonStructure(&polygon);
            destroyLinkedMultiPolygon(&polygon);
        } else {
            printf("  Error: %d\n", error);
        }
        printf("\n");
    }

    // Test both cells together
    printf("Both cells together:\n");
    LinkedGeoPolygon polygon;
    H3Error error = cellsToLinkedMultiPolygon(cells, 2, &polygon);
    if (error == E_SUCCESS) {
        printPolygonStructure(&polygon);
        destroyLinkedMultiPolygon(&polygon);
    } else {
        printf("  Error: %d\n", error);
    }

    return 0;
}

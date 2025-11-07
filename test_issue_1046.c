#include <stdio.h>
#include <h3api.h>

int main() {
    H3Index cells[2] = {
        0x8001fffffffffff,  // 577445914721910783 in decimal
        0x8003fffffffffff   // 577903311559065599 in decimal
    };

    // VALID…
    for (int i = 0; i < 2; i++) {
        if (!isValidCell(cells[i])) {
            printf("Cell %d is invalid\n", i);
            return 1;
        }
        printf("Cell %d: Valid, Resolution %d\n", i, getResolution(cells[i]));
    }

    int areNeighbors = 0;
    H3Error neighborErr = areNeighborCells(cells[0], cells[1], &areNeighbors);
    if (neighborErr == E_SUCCESS) {
        printf("Cells are neighbors: %s\n", areNeighbors ? "Yes" : "No");
    } else {
        printf("Error checking neighbors: %d\n", neighborErr);
    }

    // WORKS…
    for (int i = 0; i < 2; i++) {
        LinkedGeoPolygon polygon;
        H3Error error = cellsToLinkedMultiPolygon(&cells[i], 1, &polygon);

        if (error == E_SUCCESS) {
            printf("  Cell %d: ✓ Success\n", i);
            destroyLinkedMultiPolygon(&polygon);
        } else {
            printf("  Cell %d: ✗ Failed (error %d)\n", i, error);
        }
    }

    // FAILS…
    LinkedGeoPolygon polygon;
    H3Error error = cellsToLinkedMultiPolygon(cells, 2, &polygon);

    if (error == E_SUCCESS) {
        printf("  Both cells: ✓ Success\n");
        destroyLinkedMultiPolygon(&polygon);
    } else {
        printf("  Both cells: ✗ Failed (error %d)\n", error);
    }

    return 0;
}

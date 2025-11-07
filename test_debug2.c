#include <stdio.h>
#include "algos.h"
#include "linkedGeo.h"
#include "vertexGraph.h"

void printPolygonStructure(LinkedGeoPolygon *polygon) {
    int polygonIndex = 0;
    while (polygon) {
        printf("Polygon %d (next=%p):\n", polygonIndex, (void*)polygon->next);
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

    printf("Testing vertex graph creation...\n\n");

    VertexGraph graph;
    H3Error err = h3SetToVertexGraph(cells, 2, &graph);
    if (err) {
        printf("Error creating vertex graph: %d\n", err);
        return 1;
    }

    printf("Vertex graph created with %d edges\n\n", graph.size);

    LinkedGeoPolygon out;
    _vertexGraphToLinkedGeo(&graph, &out);
    
    printf("After _vertexGraphToLinkedGeo:\n");
    printPolygonStructure(&out);
    
    printf("\nCalling normalizeMultiPolygon...\n");
    H3Error normalizeResult = normalizeMultiPolygon(&out);
    printf("normalizeMultiPolygon returned: %d\n", normalizeResult);
    
    if (normalizeResult == E_SUCCESS) {
        printf("\nAfter normalization:\n");
        printPolygonStructure(&out);
    }

    destroyLinkedMultiPolygon(&out);
    destroyVertexGraph(&graph);

    return 0;
}

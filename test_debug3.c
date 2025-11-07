#include <stdio.h>
#include "h3api.h"
#include "algos.h"
#include "linkedGeo.h"
#include "vertexGraph.h"

void printVertexGraph(VertexGraph *graph) {
    printf("Vertex graph has %d edges:\n", graph->size);
    for (int i = 0; i < graph->numBuckets; i++) {
        VertexNode *node = graph->buckets[i];
        while (node) {
            printf("  Edge: (%.6f, %.6f) -> (%.6f, %.6f)\n",
                   node->from.lat * 180.0 / M_PI, node->from.lng * 180.0 / M_PI,
                   node->to.lat * 180.0 / M_PI, node->to.lng * 180.0 / M_PI);
            node = node->next;
        }
    }
}

int main() {
    H3Index cells[2] = {
        0x8001fffffffffff,
        0x8003fffffffffff
    };

    printf("Cell 0: 0x%llx\n", (unsigned long long)cells[0]);
    printf("Cell 1: 0x%llx\n\n", (unsigned long long)cells[1]);

    // Get boundaries
    for (int i = 0; i < 2; i++) {
        CellBoundary boundary;
        H3Error err = cellToBoundary(cells[i], &boundary);
        if (err == E_SUCCESS) {
            printf("Cell %d boundary (%d vertices):\n", i, boundary.numVerts);
            for (int j = 0; j < boundary.numVerts; j++) {
                printf("  V%d: (%.6f, %.6f)\n", j,
                       boundary.verts[j].lat * 180.0 / M_PI,
                       boundary.verts[j].lng * 180.0 / M_PI);
            }
            printf("\n");
        }
    }

    VertexGraph graph;
    H3Error err = h3SetToVertexGraph(cells, 2, &graph);
    if (err) {
        printf("Error creating vertex graph: %d\n", err);
        return 1;
    }

    printf("\n");
    printVertexGraph(&graph);

    destroyVertexGraph(&graph);
    return 0;
}

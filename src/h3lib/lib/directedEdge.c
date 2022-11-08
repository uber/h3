/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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
/** @file  directedEdge.c
 * @brief DirectedEdge functions for manipulating directed edge indexes.
 */

#include <inttypes.h>
#include <stdbool.h>

#include "algos.h"
#include "constants.h"
#include "coordijk.h"
#include "h3Assert.h"
#include "h3Index.h"
#include "latLng.h"
#include "vertex.h"

/**
 * Returns whether or not the provided H3Indexes are neighbors.
 * @param origin The origin H3 index.
 * @param destination The destination H3 index.
 * @param out Set to 1 if the indexes are neighbors, 0 otherwise
 * @return Error code if the origin or destination are invalid or incomparable.
 */
H3Error H3_EXPORT(areNeighborCells)(H3Index origin, H3Index destination,
                                    int *out) {
    // Make sure they're hexagon indexes
    if (H3_GET_MODE(origin) != H3_CELL_MODE ||
        H3_GET_MODE(destination) != H3_CELL_MODE) {
        return E_CELL_INVALID;
    }

    // Hexagons cannot be neighbors with themselves
    if (origin == destination) {
        *out = 0;
        return E_SUCCESS;
    }

    // Only hexagons in the same resolution can be neighbors
    if (H3_GET_RESOLUTION(origin) != H3_GET_RESOLUTION(destination)) {
        return E_RES_MISMATCH;
    }

    // H3 Indexes that share the same parent are very likely to be neighbors
    // Child 0 is neighbor with all of its parent's 'offspring', the other
    // children are neighbors with 3 of the 7 children. So a simple comparison
    // of origin and destination parents and then a lookup table of the children
    // is a super-cheap way to possibly determine they are neighbors.
    int parentRes = H3_GET_RESOLUTION(origin) - 1;
    if (parentRes > 0) {
        // TODO: Return error codes here
        H3Index originParent;
        H3_EXPORT(cellToParent)(origin, parentRes, &originParent);
        H3Index destinationParent;
        H3_EXPORT(cellToParent)(destination, parentRes, &destinationParent);
        if (originParent == destinationParent) {
            Direction originResDigit =
                H3_GET_INDEX_DIGIT(origin, parentRes + 1);
            Direction destinationResDigit =
                H3_GET_INDEX_DIGIT(destination, parentRes + 1);
            if (originResDigit == CENTER_DIGIT ||
                destinationResDigit == CENTER_DIGIT) {
                *out = 1;
                return E_SUCCESS;
            }
            if (originResDigit >= INVALID_DIGIT) {
                // Prevent indexing off the end of the array below
                return E_CELL_INVALID;
            }
            if ((originResDigit == K_AXES_DIGIT ||
                 destinationResDigit == K_AXES_DIGIT) &&
                H3_EXPORT(isPentagon)(originParent)) {
                // If these are invalid cells, fail rather than incorrectly
                // reporting neighbors. For pentagon cells that are actually
                // neighbors across the deleted subsequence, they will fail the
                // optimized check below, but they will be accepted by the
                // gridDisk check below that.
                return E_CELL_INVALID;
            }
            // These sets are the relevant neighbors in the clockwise
            // and counter-clockwise
            const Direction neighborSetClockwise[] = {
                CENTER_DIGIT,  JK_AXES_DIGIT, IJ_AXES_DIGIT, J_AXES_DIGIT,
                IK_AXES_DIGIT, K_AXES_DIGIT,  I_AXES_DIGIT};
            const Direction neighborSetCounterclockwise[] = {
                CENTER_DIGIT,  IK_AXES_DIGIT, JK_AXES_DIGIT, K_AXES_DIGIT,
                IJ_AXES_DIGIT, I_AXES_DIGIT,  J_AXES_DIGIT};
            if (neighborSetClockwise[originResDigit] == destinationResDigit ||
                neighborSetCounterclockwise[originResDigit] ==
                    destinationResDigit) {
                *out = 1;
                return E_SUCCESS;
            }
        }
    }

    // Otherwise, we have to determine the neighbor relationship the "hard" way.
    H3Index neighborRing[7] = {0};
    H3_EXPORT(gridDisk)(origin, 1, neighborRing);
    for (int i = 0; i < 7; i++) {
        if (neighborRing[i] == destination) {
            *out = 1;
            return E_SUCCESS;
        }
    }

    // Made it here, they definitely aren't neighbors
    *out = 0;
    return E_SUCCESS;
}

/**
 * Returns a directed edge H3 index based on the provided origin and
 * destination
 * @param origin The origin H3 hexagon index
 * @param destination The destination H3 hexagon index
 * @return The directed edge H3Index, or H3_NULL on failure.
 */
H3Error H3_EXPORT(cellsToDirectedEdge)(H3Index origin, H3Index destination,
                                       H3Index *out) {
    // Determine the IJK direction from the origin to the destination
    Direction direction = directionForNeighbor(origin, destination);

    // The direction will be invalid if the cells are not neighbors
    if (direction == INVALID_DIGIT) {
        return E_NOT_NEIGHBORS;
    }

    // Create the edge index for the neighbor direction
    H3Index output = origin;
    H3_SET_MODE(output, H3_DIRECTEDEDGE_MODE);
    H3_SET_RESERVED_BITS(output, direction);

    *out = output;
    return E_SUCCESS;
}

/**
 * Returns the origin hexagon from the directed edge H3Index
 * @param edge The edge H3 index
 * @return The origin H3 hexagon index, or H3_NULL on failure
 */
H3Error H3_EXPORT(getDirectedEdgeOrigin)(H3Index edge, H3Index *out) {
    if (H3_GET_MODE(edge) != H3_DIRECTEDEDGE_MODE) {
        return E_DIR_EDGE_INVALID;
    }
    H3Index origin = edge;
    H3_SET_MODE(origin, H3_CELL_MODE);
    H3_SET_RESERVED_BITS(origin, 0);
    *out = origin;
    return E_SUCCESS;
}

/**
 * Returns the destination hexagon from the directed edge H3Index
 * @param edge The edge H3 index
 * @return The destination H3 hexagon index, or H3_NULL on failure
 */
H3Error H3_EXPORT(getDirectedEdgeDestination)(H3Index edge, H3Index *out) {
    Direction direction = H3_GET_RESERVED_BITS(edge);
    int rotations = 0;
    H3Index origin;
    // Note: This call is also checking for H3_DIRECTEDEDGE_MODE
    H3Error originResult = H3_EXPORT(getDirectedEdgeOrigin)(edge, &origin);
    if (originResult) {
        return originResult;
    }
    return h3NeighborRotations(origin, direction, &rotations, out);
}

/**
 * Determines if the provided H3Index is a valid directed edge index
 * @param edge The directed edge H3Index
 * @return 1 if it is a directed edge H3Index, otherwise 0.
 */
int H3_EXPORT(isValidDirectedEdge)(H3Index edge) {
    Direction neighborDirection = H3_GET_RESERVED_BITS(edge);
    if (neighborDirection <= CENTER_DIGIT || neighborDirection >= NUM_DIGITS) {
        return 0;
    }

    H3Index origin;
    // Note: This call is also checking for H3_DIRECTEDEDGE_MODE
    H3Error originResult = H3_EXPORT(getDirectedEdgeOrigin)(edge, &origin);
    if (originResult) {
        return 0;
    }
    if (H3_EXPORT(isPentagon)(origin) && neighborDirection == K_AXES_DIGIT) {
        return 0;
    }

    return H3_EXPORT(isValidCell)(origin);
}

/**
 * Returns the origin, destination pair of hexagon IDs for the given edge ID
 * @param edge The directed edge H3Index
 * @param originDestination Pointer to memory to store origin and destination
 * IDs
 */
H3Error H3_EXPORT(directedEdgeToCells)(H3Index edge,
                                       H3Index *originDestination) {
    H3Error originResult =
        H3_EXPORT(getDirectedEdgeOrigin)(edge, &originDestination[0]);
    if (originResult) {
        return originResult;
    }
    H3Error destinationResult =
        H3_EXPORT(getDirectedEdgeDestination)(edge, &originDestination[1]);
    if (destinationResult) {
        return destinationResult;
    }
    return E_SUCCESS;
}

/**
 * Provides all of the directed edges from the current H3Index.
 * @param origin The origin hexagon H3Index to find edges for.
 * @param edges The memory to store all of the edges inside.
 */
H3Error H3_EXPORT(originToDirectedEdges)(H3Index origin, H3Index *edges) {
    // Determine if the origin is a pentagon and special treatment needed.
    int isPent = H3_EXPORT(isPentagon)(origin);

    // This is actually quite simple. Just modify the bits of the origin
    // slightly for each direction, except the 'k' direction in pentagons,
    // which is zeroed.
    for (int i = 0; i < 6; i++) {
        if (isPent && i == 0) {
            edges[i] = H3_NULL;
        } else {
            edges[i] = origin;
            H3_SET_MODE(edges[i], H3_DIRECTEDEDGE_MODE);
            H3_SET_RESERVED_BITS(edges[i], i + 1);
        }
    }
    return E_SUCCESS;
}

/**
 * Provides the coordinates defining the directed edge.
 * @param edge The directed edge H3Index
 * @param cb The cellboundary object to store the edge coordinates.
 */
H3Error H3_EXPORT(directedEdgeToBoundary)(H3Index edge, CellBoundary *cb) {
    // Get the origin and neighbor direction from the edge
    Direction direction = H3_GET_RESERVED_BITS(edge);
    H3Index origin;
    H3Error originResult = H3_EXPORT(getDirectedEdgeOrigin)(edge, &origin);
    if (originResult) {
        return originResult;
    }

    // Get the start vertex for the edge
    int startVertex = vertexNumForDirection(origin, direction);
    if (startVertex == INVALID_VERTEX_NUM) {
        // This is not actually an edge (i.e. no valid direction),
        // so return no vertices.
        cb->numVerts = 0;
        return E_DIR_EDGE_INVALID;
    }

    // Get the geo boundary for the appropriate vertexes of the origin. Note
    // that while there are always 2 topological vertexes per edge, the
    // resulting edge boundary may have an additional distortion vertex if it
    // crosses an edge of the icosahedron.
    FaceIJK fijk;
    H3Error fijkResult = _h3ToFaceIjk(origin, &fijk);
    if (NEVER(fijkResult)) {
        return fijkResult;
    }
    int res = H3_GET_RESOLUTION(origin);
    int isPent = H3_EXPORT(isPentagon)(origin);

    if (isPent) {
        _faceIjkPentToCellBoundary(&fijk, res, startVertex, 2, cb);
    } else {
        _faceIjkToCellBoundary(&fijk, res, startVertex, 2, cb);
    }
    return E_SUCCESS;
}

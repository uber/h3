/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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
/** @file  h3UniEdge.c
 * @brief H3UniEdge functions for manipulating unidirectional edge indexes.
 */

#include <inttypes.h>
#include <stdbool.h>

#include "algos.h"
#include "constants.h"
#include "coordijk.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "vertex.h"

/**
 * Returns whether or not the provided H3Indexes are neighbors.
 * @param origin The origin H3 index.
 * @param destination The destination H3 index.
 * @return 1 if the indexes are neighbors, 0 otherwise;
 */
int H3_EXPORT(h3IndexesAreNeighbors)(H3Index origin, H3Index destination) {
    // Make sure they're hexagon indexes
    if (H3_GET_MODE(origin) != H3_HEXAGON_MODE ||
        H3_GET_MODE(destination) != H3_HEXAGON_MODE) {
        return 0;
    }

    // Hexagons cannot be neighbors with themselves
    if (origin == destination) {
        return 0;
    }

    // Only hexagons in the same resolution can be neighbors
    if (H3_GET_RESOLUTION(origin) != H3_GET_RESOLUTION(destination)) {
        return 0;
    }

    // H3 Indexes that share the same parent are very likely to be neighbors
    // Child 0 is neighbor with all of its parent's 'offspring', the other
    // children are neighbors with 3 of the 7 children. So a simple comparison
    // of origin and destination parents and then a lookup table of the children
    // is a super-cheap way to possibly determine they are neighbors.
    int parentRes = H3_GET_RESOLUTION(origin) - 1;
    if (parentRes > 0 && (H3_EXPORT(h3ToParent)(origin, parentRes) ==
                          H3_EXPORT(h3ToParent)(destination, parentRes))) {
        Direction originResDigit = H3_GET_INDEX_DIGIT(origin, parentRes + 1);
        Direction destinationResDigit =
            H3_GET_INDEX_DIGIT(destination, parentRes + 1);
        if (originResDigit == CENTER_DIGIT ||
            destinationResDigit == CENTER_DIGIT) {
            return 1;
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
            return 1;
        }
    }

    // Otherwise, we have to determine the neighbor relationship the "hard" way.
    H3Index neighborRing[7] = {0};
    H3_EXPORT(kRing)(origin, 1, neighborRing);
    for (int i = 0; i < 7; i++) {
        if (neighborRing[i] == destination) {
            return 1;
        }
    }

    // Made it here, they definitely aren't neighbors
    return 0;
}

/**
 * Returns a unidirectional edge H3 index based on the provided origin and
 * destination
 * @param origin The origin H3 hexagon index
 * @param destination The destination H3 hexagon index
 * @return The unidirectional edge H3Index, or H3_NULL on failure.
 */
H3Index H3_EXPORT(getH3UnidirectionalEdge)(H3Index origin,
                                           H3Index destination) {
    // Short-circuit and return an invalid index value if they are not neighbors
    if (H3_EXPORT(h3IndexesAreNeighbors)(origin, destination) == 0) {
        return H3_NULL;
    }

    // Otherwise, determine the IJK direction from the origin to the destination
    H3Index output = origin;
    H3_SET_MODE(output, H3_UNIEDGE_MODE);

    bool isPentagon = H3_EXPORT(h3IsPentagon)(origin);

    // Checks each neighbor, in order, to determine which direction the
    // destination neighbor is located. Skips CENTER_DIGIT since that
    // would be this index.
    H3Index neighbor;
    // Excluding from branch coverage as we never hit the end condition
    // LCOV_EXCL_BR_START
    for (Direction direction = isPentagon ? J_AXES_DIGIT : K_AXES_DIGIT;
         direction < NUM_DIGITS; direction++) {
        // LCOV_EXCL_BR_STOP
        int rotations = 0;
        neighbor = h3NeighborRotations(origin, direction, &rotations);
        if (neighbor == destination) {
            H3_SET_RESERVED_BITS(output, direction);
            return output;
        }
    }

    // This should be impossible, return H3_NULL in this case;
    return H3_NULL;  // LCOV_EXCL_LINE
}

/**
 * Returns the origin hexagon from the unidirectional edge H3Index
 * @param edge The edge H3 index
 * @return The origin H3 hexagon index, or H3_NULL on failure
 */
H3Index H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(H3Index edge) {
    if (H3_GET_MODE(edge) != H3_UNIEDGE_MODE) {
        return H3_NULL;
    }
    H3Index origin = edge;
    H3_SET_MODE(origin, H3_HEXAGON_MODE);
    H3_SET_RESERVED_BITS(origin, 0);
    return origin;
}

/**
 * Returns the destination hexagon from the unidirectional edge H3Index
 * @param edge The edge H3 index
 * @return The destination H3 hexagon index, or H3_NULL on failure
 */
H3Index H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(H3Index edge) {
    if (H3_GET_MODE(edge) != H3_UNIEDGE_MODE) {
        return H3_NULL;
    }
    Direction direction = H3_GET_RESERVED_BITS(edge);
    int rotations = 0;
    H3Index destination = h3NeighborRotations(
        H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge), direction,
        &rotations);
    return destination;
}

/**
 * Determines if the provided H3Index is a valid unidirectional edge index
 * @param edge The unidirectional edge H3Index
 * @return 1 if it is a unidirectional edge H3Index, otherwise 0.
 */
int H3_EXPORT(h3UnidirectionalEdgeIsValid)(H3Index edge) {
    if (H3_GET_MODE(edge) != H3_UNIEDGE_MODE) {
        return 0;
    }

    Direction neighborDirection = H3_GET_RESERVED_BITS(edge);
    if (neighborDirection <= CENTER_DIGIT || neighborDirection >= NUM_DIGITS) {
        return 0;
    }

    H3Index origin = H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge);
    if (H3_EXPORT(h3IsPentagon)(origin) && neighborDirection == K_AXES_DIGIT) {
        return 0;
    }

    return H3_EXPORT(h3IsValid)(origin);
}

/**
 * Returns the origin, destination pair of hexagon IDs for the given edge ID
 * @param edge The unidirectional edge H3Index
 * @param originDestination Pointer to memory to store origin and destination
 * IDs
 */
void H3_EXPORT(getH3IndexesFromUnidirectionalEdge)(H3Index edge,
                                                   H3Index* originDestination) {
    originDestination[0] =
        H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge);
    originDestination[1] =
        H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edge);
}

/**
 * Provides all of the unidirectional edges from the current H3Index.
 * @param origin The origin hexagon H3Index to find edges for.
 * @param edges The memory to store all of the edges inside.
 */
void H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(H3Index origin,
                                                    H3Index* edges) {
    // Determine if the origin is a pentagon and special treatment needed.
    int isPentagon = H3_EXPORT(h3IsPentagon)(origin);

    // This is actually quite simple. Just modify the bits of the origin
    // slightly for each direction, except the 'k' direction in pentagons,
    // which is zeroed.
    for (int i = 0; i < 6; i++) {
        if (isPentagon && i == 0) {
            edges[i] = H3_NULL;
        } else {
            edges[i] = origin;
            H3_SET_MODE(edges[i], H3_UNIEDGE_MODE);
            H3_SET_RESERVED_BITS(edges[i], i + 1);
        }
    }
}

/**
 * Provides the coordinates defining the unidirectional edge.
 * @param edge The unidirectional edge H3Index
 * @param gb The geoboundary object to store the edge coordinates.
 */
void H3_EXPORT(getH3UnidirectionalEdgeBoundary)(H3Index edge, GeoBoundary* gb) {
    // Get the origin and neighbor direction from the edge
    Direction direction = H3_GET_RESERVED_BITS(edge);
    H3Index origin = H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge);

    // Get the start vertex for the edge
    int startVertex = vertexNumForDirection(origin, direction);
    if (startVertex == INVALID_VERTEX_NUM) {
        // This is not actually an edge (i.e. no valid direction),
        // so return no vertices.
        gb->numVerts = 0;
        return;
    }

    // Get the geo boundary for the appropriate vertexes of the origin. Note
    // that while there are always 2 topological vertexes per edge, the
    // resulting edge boundary may have an additional distortion vertex if it
    // crosses an edge of the icosahedron.
    FaceIJK fijk;
    _h3ToFaceIjk(origin, &fijk);
    int res = H3_GET_RESOLUTION(origin);
    int isPentagon = H3_EXPORT(h3IsPentagon)(origin);

    if (isPentagon) {
        _faceIjkPentToGeoBoundary(&fijk, res, startVertex, 2, gb);
    } else {
        _faceIjkToGeoBoundary(&fijk, res, startVertex, 2, gb);
    }
}

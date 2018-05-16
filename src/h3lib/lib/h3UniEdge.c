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
/** @file  h3UniEdge.c
 * @brief H3UniEdge functions for manipulating unidirectional edge indexes.
 */

#include <inttypes.h>
#include "algos.h"
#include "constants.h"
#include "coordijk.h"
#include "geoCoord.h"
#include "h3Index.h"

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
        int originResDigit = H3_GET_INDEX_DIGIT(origin, parentRes + 1);
        int destinationResDigit =
            H3_GET_INDEX_DIGIT(destination, parentRes + 1);
        if (originResDigit == 0 || destinationResDigit == 0) {
            return 1;
        }
        // These sets are the relevant neighbors in the clockwise
        // and counter-clockwise
        const int neighborSetClockwise[] = {0, 3, 6, 2, 5, 1, 4};
        const int neighborSetCounterclockwise[] = {0, 5, 3, 1, 6, 4, 2};
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
 * @return The unidirectional edge H3Index, or 0 on failure.
 */
H3Index H3_EXPORT(getH3UnidirectionalEdge)(H3Index origin,
                                           H3Index destination) {
    // Short-circuit and return an invalid index value if they are not neighbors
    if (H3_EXPORT(h3IndexesAreNeighbors)(origin, destination) == 0) {
        return H3_INVALID_INDEX;
    }

    // Otherwise, determine the IJK direction from the origin to the destination
    H3Index output = origin;
    H3_SET_MODE(output, H3_UNIEDGE_MODE);

    // Checks each neighbor, in order, to determine which direction the
    // destination neighbor is located.
    H3Index neighbor;
    for (int i = 1; i < 7; i++) {
        int rotations = 0;
        neighbor = h3NeighborRotations(origin, i, &rotations);
        if (neighbor == destination) {
            H3_SET_RESERVED_BITS(output, i);
            return output;
        }
    }

    // This should be impossible, return an invalid H3Index in this case;
    return H3_INVALID_INDEX;  // LCOV_EXCL_LINE
}

/**
 * Returns the origin hexagon from the unidirectional edge H3Index
 * @param edge The edge H3 index
 * @return The origin H3 hexagon index
 */
H3Index H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(H3Index edge) {
    H3Index origin = edge;
    H3_SET_MODE(origin, H3_HEXAGON_MODE);
    H3_SET_RESERVED_BITS(origin, 0);
    return origin;
}

/**
 * Returns the destination hexagon from the unidirectional edge H3Index
 * @param edge The edge H3 index
 * @return The destination H3 hexagon index
 */
H3Index H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(H3Index edge) {
    int direction = H3_GET_RESERVED_BITS(edge);
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

    int neighborDirection = H3_GET_RESERVED_BITS(edge);
    if (neighborDirection < 1 || neighborDirection > 6) {
        return 0;
    }

    H3Index origin = H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge);
    if (H3_EXPORT(h3IsPentagon)(origin) && neighborDirection == 1) {
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
        if (isPentagon == 1 && i == 0) {
            edges[i] = 0;
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
    // TODO: More efficient solution :)
    GeoBoundary origin = {0};
    GeoBoundary destination = {0};
    H3_EXPORT(h3ToGeoBoundary)
    (H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge), &origin);
    H3_EXPORT(h3ToGeoBoundary)
    (H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edge),
     &destination);

    int k = 0;
    for (int i = 0; i < origin.numVerts; i++) {
        for (int j = 0; j < destination.numVerts; j++) {
            if (geoAlmostEqualThreshold(&origin.verts[i], &destination.verts[j],
                                        0.000001)) {
                gb->verts[k].lat = origin.verts[i].lat;
                gb->verts[k].lon = origin.verts[i].lon;
                k++;
            }
        }
    }
    gb->numVerts = k;
}

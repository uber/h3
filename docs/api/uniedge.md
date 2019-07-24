# Unidirectional edge functions

Unidirectional edges allow encoding the directed edge from one cell to a neighboring cell.

## h3IndexesAreNeighbors

```
int h3IndexesAreNeighbors(H3Index origin, H3Index destination);
```

Returns whether or not the provided H3Indexes are neighbors.

Returns 1 if the indexes are neighbors, 0 otherwise.

## getH3UnidirectionalEdge

```
H3Index getH3UnidirectionalEdge(H3Index origin, H3Index destination);
```

Returns a unidirectional edge **H3** index based on the provided origin and
destination.

Returns 0 on error.

## h3UnidirectionalEdgeIsValid

```
int h3UnidirectionalEdgeIsValid(H3Index edge);
```

Determines if the provided H3Index is a valid unidirectional edge index.

Returns 1 if it is a unidirectional edge H3Index, otherwise 0.

## getOriginH3IndexFromUnidirectionalEdge

```
H3Index getOriginH3IndexFromUnidirectionalEdge(H3Index edge);
```

Returns the origin hexagon from the unidirectional edge H3Index.

## getDestinationH3IndexFromUnidirectionalEdge

```
H3Index getDestinationH3IndexFromUnidirectionalEdge(H3Index edge);
```

Returns the destination hexagon from the unidirectional edge H3Index.

## getH3IndexesFromUnidirectionalEdge

```
void getH3IndexesFromUnidirectionalEdge(H3Index edge, H3Index* originDestination);
```

Returns the origin, destination pair of hexagon IDs for the given edge ID, which are placed at `originDestination[0]` and
`originDestination[1]` respectively.

## getH3UnidirectionalEdgesFromHexagon

```
void getH3UnidirectionalEdgesFromHexagon(H3Index origin, H3Index* edges);
```

Provides all of the unidirectional edges from the current H3Index. `edges` must be of length 6,
and the number of undirectional edges placed in the array may be less than 6.

## getH3UnidirectionalEdgeBoundary

```
void getH3UnidirectionalEdgeBoundary(H3Index edge, GeoBoundary* gb);
```

Provides the coordinates defining the unidirectional edge.

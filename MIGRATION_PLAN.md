# Migration Plan: cellsToLinkedMultiPolygon -> cellsToMultiPolygon

Replace the VertexGraph-based implementation of `cellsToLinkedMultiPolygon`
with `cellsToMultiPolygon` + `geoMultiPolygonToLinkedGeoPolygon`, then remove
the now-dead VertexGraph code.

Sanity-check results (reverted) confirmed:
- C API test suite passes with the swap (after one error-code tweak)
- CLI tests fail only on vertex start-position rotation (same coords, different starting vertex)

---

## 1. Swap `cellsToLinkedMultiPolygon` body

**File:** `src/h3lib/lib/algos.c`

Replace the VertexGraph path in `cellsToLinkedMultiPolygon` (~line 1277):

```c
VertexGraph graph;
H3Error err = h3SetToVertexGraph(h3Set, numHexes, &graph);
...
```

With:

```c
GeoMultiPolygon mpoly;
H3Error err = H3_EXPORT(cellsToMultiPolygon)(h3Set, numHexes, &mpoly);
if (err) {
    return err;
}
err = geoMultiPolygonToLinkedGeoPolygon(&mpoly, out);
H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
if (err) {
    H3_EXPORT(destroyLinkedMultiPolygon)(out);
}
return err;
```

Also in algos.c:
- Add `#include "cellsToMultiPoly.h"`
- Remove `#include "vertexGraph.h"` (line 39)

## 2. Delete `h3SetToVertexGraph` and `_vertexGraphToLinkedGeo`

**File:** `src/h3lib/lib/algos.c`

- Delete `h3SetToVertexGraph` (~lines 1177-1219, including doc comment)
- Delete `_vertexGraphToLinkedGeo` (~lines 1230-1247, including doc comment)

**File:** `src/h3lib/include/algos.h`

- Remove `#include "vertexGraph.h"` (line 27)
- Remove declaration of `h3SetToVertexGraph` (lines 40-42)
- Remove declaration of `_vertexGraphToLinkedGeo` (lines 44-45)

## 3. Edit `fuzzerInternalAlgos.c` -- keep valid fuzzers

**File:** `src/apps/fuzzers/fuzzerInternalAlgos.c`

Remove the VertexGraph fuzzing block (lines 44-53):

```c
    VertexGraph graph;
    H3Index *h3Set = (H3Index *)data;
    size_t inputSize = size / sizeof(H3Index);
    H3Error err = h3SetToVertexGraph(h3Set, inputSize, &graph);
    if (!err) {
        LinkedGeoPolygon linkedGeoPolygon;
        _vertexGraphToLinkedGeo(&graph, &linkedGeoPolygon);
        H3_EXPORT(destroyLinkedMultiPolygon)(&linkedGeoPolygon);
        destroyVertexGraph(&graph);
    }
```

Keep `h3NeighborRotations` and `directionForNeighbor` calls (lines 38-42).

**Note:** Remove `#include "h3api.h"` (line 22). It is redundant since
`algos.h` already includes `h3api.h`. Verify after the edit that the file
still compiles cleanly.

## 4. Delete `testH3SetToVertexGraphInternal`

- Delete file: `src/apps/testapps/testH3SetToVertexGraphInternal.c`
- Remove from `CMakeLists.txt` line 253:
  `src/apps/testapps/testH3SetToVertexGraphInternal.c`
- Remove from `CMakeTests.cmake` lines 226-227:
  `add_h3_test(testH3SetToVertexGraphInternal ...)`

## 5. Fix `specificLeak` test error code

**File:** `src/apps/testapps/testCellsToLinkedMultiPolygon.c` (lines 325-326)

Change:
```c
t_assert(... == E_FAILED, "invalid cells fail");
```
To:
```c
t_assert(... == E_CELL_INVALID, "invalid cells fail");
```

The old code went through `cellToBoundary` which returns `E_FAILED`; the new
code goes through `validateCellSet` which returns `E_CELL_INVALID`.

## 6. Regenerate CLI test expected outputs

**File:** `tests/cli/cellsToMultiPolygon.txt`

The CLI tests hardcode exact vertex ordering. The new implementation produces
the same coordinates but with different loop starting vertices.

After completing the code changes, regenerate expected values by running each
CLI command and capturing its output. Replace all expected-value strings in
`cellsToMultiPolygon.txt` with the new output.

## 7. Update fuzzers README

**File:** `src/apps/fuzzers/README.md` (lines 83-84)

Remove:
```
| h3SetToVertexGraph | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
| _vertexGraphToLinkedGeo | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
```

Keep lines 81-82 (`h3NeighborRotations`, `directionForNeighbor`).

## 8. Remove dead VertexGraph library code

After step 2, `vertexGraph.c` and `vertexGraph.h` have zero library
consumers. The only remaining user is `testVertexGraphInternal.c`, which
tests the data structure in isolation.

- Delete `src/h3lib/lib/vertexGraph.c`
- Delete `src/h3lib/include/vertexGraph.h`
- Remove from `CMakeLists.txt`: `src/h3lib/include/vertexGraph.h` (line 167)
  and `src/h3lib/lib/vertexGraph.c` (line 192)
- Delete `src/apps/testapps/testVertexGraphInternal.c`
- Remove from `CMakeLists.txt`: `src/apps/testapps/testVertexGraphInternal.c`
  (line 223, in OTHER_SOURCE_FILES)
- Remove from `CMakeTests.cmake`: `add_h3_test(testVertexGraphInternal ...)`
  (line 237)
- Remove stale comment in `src/h3lib/lib/linkedGeo.c` line 501 referencing
  `h3SetToVertexGraph`

## 9. Memory testing (testH3Memory.c)

No changes needed. `testH3Memory.c` already has comprehensive allocation-
failure tests for both `cellsToMultiPolygon` (lines 295-511) and
`geoMultiPolygonToLinkedGeoPolygon` (lines 444-473). After the swap,
`cellsToLinkedMultiPolygon` is just those two functions composed together,
so the existing memory tests provide full coverage of the underlying
allocation paths. There are no existing `cellsToLinkedMultiPolygon` memory
tests that would become duplicative.

## 10. Build and test

```sh
just test-fast
```

All tests should pass (minus the removed vertex graph tests).

## 11. Verify full coverage

```sh
just coverage
```

Run `just coverage-gaps` on all changed `.c` files (at minimum `algos.c`,
`cellsToMultiPoly.c`, `linkedGeo.c`) and confirm no new uncovered
lines or branches were introduced by the migration.

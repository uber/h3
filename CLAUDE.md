# H3 Development Guide

## Build System

This project uses CMake with a `justfile` for common tasks:

| Command                          | Description                               |
|----------------------------------|-------------------------------------------|
| `just init`                      | Clean rebuild of build directory          |
| `just build`                     | Build (Release mode)                      |
| `just test-fast`                 | Run fast test suite (309 tests)           |
| `just test-one testName`         | Run a single test binary                  |
| `just coverage`                  | Full coverage build + report (opens HTML) |
| `just coverage-gaps linkedGeo.c` | Show uncovered lines/branches for a file  |

## Coverage Workflow

When checking or improving coverage on changed files, follow this process:

### 1. Run `just coverage`

This does a full purge, Debug rebuild with `--coverage`, runs all tests, and
generates an lcov report. The summary lines in stdout show per-file stats:

```
Processing file lib/linkedGeo.c
  lines=256 hit=256 functions=15 hit=15 branches=90 hit=90
```

`just coverage` takes a while to run, so redirect its output to a file for
later processing:

```sh
just coverage > /tmp/h3-coverage.txt 2>&1
grep 'Processing file lib/YOURFILE' -A1 /tmp/h3-coverage.txt
```

### 2. Find specific gaps with `just coverage-gaps <file>`

After `just coverage` has run, use `just coverage-gaps linkedGeo.c` to see
exactly which lines and branches are uncovered. This parses the lcov `.info`
file (`build/coverage.cleaned.info`) which has merged coverage from all test
binaries (including the custom-allocator `h3WithTestAllocators` build).

**Do NOT use `gcov` directly** — it only reads one `.gcno`/`.gcda` pair and
will miss coverage from `h3WithTestAllocators`. The lcov data is the
authoritative source.

### 3. Common coverage gap categories in this codebase

- **Allocation failure paths** (`E_MEMORY_ALLOC` returns): Tested via
  `testH3Memory.c` which uses a custom allocator (`test_prefix_malloc` etc.)
  that can be set to fail after N calls. Pattern:
  ```c
  // Build test data with unlimited allocs
  resetMemoryCounters(0);
  // ... build data structures ...

  // Then limit allocs for the function under test
  resetMemoryCounters(permitted);  // permit only N allocs
  err = H3_EXPORT(functionUnderTest)(...);
  ```
  **Important:** After building data structures, call `resetMemoryCounters(N)`
  with just the number of allocs the *function under test* should get — don't
  add the build allocs (they're already done, counter resets to 0).

- **Edge cases** (empty loops, zero-count arrays): Tested in the regular
  internal test files (e.g., `testLinkedGeoInternal.c`).

### 4. Cleanup safety with `destroyGeoMultiPolygon`

When writing functions that allocate into a `GeoMultiPolygon` and may fail
partway through: set count fields (`numHoles`, `numPolygons`) **after** their
corresponding array allocation succeeds, not before. The `calloc`-zeroed
initial state (count=0, pointer=NULL) is safe for `destroyGeoMultiPolygon` to
iterate over. Setting the count before the alloc means a failure + cleanup will
iterate over a NULL array.

## Project Structure

- `src/h3lib/lib/` — Core library implementation
- `src/h3lib/include/` — Headers (public API in `h3api.h.in`)
- `src/apps/testapps/` — Test files
- `CMakeTests.cmake` — Test registration and coverage targets

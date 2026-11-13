# H3 conformance suite

A language-neutral description of what the H3 reference implementation
returns for a fixed set of inputs, so that a port can prove it matches without
building the C library. This document is the format specification. The
generator, `src/apps/testapps/mkConformance.c`, produces the suite from the
library under `tests/inputfiles`; the runners, `src/apps/testapps/
testConformance*.c`, check the library against the committed files on every
platform in CI; and ports consume the same files.

Records are newline-delimited JSON, one per subject, with every key present
on every line and errors spelled as `H3Error` names, so a file can be read
by any language without a schema and a failing line can be quoted as it is.

## What conformance means

An implementation that passes the suite makes these promises, and only
these. The rest of this file is the mechanism; this section is the
contract, including what it deliberately leaves out.

**Discrete outputs are identical to the reference.** Indexes, counts,
booleans, errors, sets and sequences of indexes are compared exactly,
exhaustively over every cell at resolutions 0 through 7 through the digest
streams and by sample beyond. There is no tolerance on a cell index.

**Coordinates are within 1e-12 degrees of arc of the reference**, measured
as angular distance on the sphere, and **measures are within 1e-9
relative**, or within an absolute floor derived from that angle where the
quantity is too small for a relative bound to be meaningful. The bound is
what the reference meets against itself: the same C source built for
x86-64 and arm64 differs in the last bits of most coordinate outputs, from
fused multiply-add and libm differences, so bit identity is not something
any implementation can promise, the reference included.

**Why the bound is an angle and not a latitude and a longitude.** The
projection produces a unit vector, and longitude is a chart on it that is
singular at the poles: a point a given angular distance from the pole has
a longitude defined only to that distance divided by `cos(lat)`. Two
implementations that agree to a few ulps of arc, about 5e-14 degrees, can
therefore differ in longitude by that times `1/cos(lat)`, which is 1,384 at
the highest-latitude resolution 4 cell, latitude 89.96 degrees, and grows
by about `sqrt(7)` per resolution as cells crowd toward the poles. A
per-coordinate bound met at the equator is breached at the poles from
about resolution 10 on, by the reference built for two architectures as
much as by any port, while the angular distance between the same two
points stays at a few ulps. Angular distance is the quantity the library
actually computes and is well conditioned everywhere; see Tolerances.

**The bound is a floor on agreement, not a ceiling on accuracy.** Checked
against a 256-bit evaluation of the same formula
(`testExactProjectionExhaustive`), the reference places every cell center
and boundary vertex within 1e-13 degrees of arc of the exact value, worst
case about 5e-14 on the platforms CI builds, and the pure-Go port does the
same. Two implementations that each sit that close to the truth differ by
at most the sum of their errors, far inside the bound; the bound is set by
what the reference meets against itself across platforms, not by how far
either side is from the exact value.

**Knife-edge inputs are outside the contract**, and the suite avoids them
by construction rather than by filtering. Where an output is a discrete
decision made on a float that lands within rounding error of the decision
boundary, two correct implementations may decide differently, and so may
one implementation on two platforms:

- *Points on cell edges.* `latLngToCell` for a point within about 1e-9
  degrees of a cell edge or vertex may return either neighbour. The
  suite's reverse-lookup points are interior, halfway from the center to
  each vertex, and the digests' round trip starts from cell centers.
- *Polygon vertices on cell edges.* The `full` and `overlapping`
  containment modes of `polygonToCellsExperimental` decide whether a cell
  boundary vertex is inside the polygon. A polygon that is exactly a cell
  boundary puts every finer cell's vertices on its edges, and the
  reference disagreed with itself on 135 of 270 such rows after a round
  trip through degrees. The suite's polygons are cell boundaries scaled by
  0.8 and 1.2 about the center, which keeps every cell vertex clear of
  every polygon edge.
- *Nearly antipodal points.* The haversine `asin` has infinite slope at
  its argument's upper limit, so a last-bit difference in the inputs moves
  the distance by 1e-8 radians. The suite never pairs points within 1e-3
  radians of antipodal.
- *Fused arithmetic.* A compiler that contracts a multiply and an add into
  one rounding, which gcc does across statements on arm64 and clang does
  within an expression, moves last bits on both sides of a comparison.
  A port that takes degrees should round its degree-to-radian conversions
  explicitly before subtracting them so that a point is at distance exactly
  zero from itself; the reference takes radians and has nothing to fuse
  there.

**Where the reference fails from its own limits, the better answer is
allowed.** The classic `polygonToCells` returns `E_FAILED` when its output
size estimate underestimates the fill, which happens for a band from 85 to
89.9 degrees north spanning 179 degrees of longitude; an implementation
without a fixed output buffer returns the cells. No such input is in the
suite, so both behaviours pass. By contrast, `gridDistance` and
`gridPathCells` returning `E_FAILED` for pairs that cross a pentagon or
span too many faces is part of the contract: the reference's refusal is
recorded and an implementation must refuse the same pairs.

**Not specified** because the reference does not specify it: the output of
`compactCells` on an input of mixed resolutions, which is order dependent
in the reference; the order of indexes in any output, which the suite
sorts before comparing; and the `0` padding of grid outputs, which the
suite removes.

## Layout

```
tests/inputfiles/
  manifest.json
  inspection/cells.jsonl
  inspection/curated.jsonl
  floats/cells.jsonl
  traversal/cells.jsonl
  traversal/curated.jsonl
  unsafe/cells.jsonl
  localij/pairs.jsonl
  localij/curated.jsonl
  hierarchy/cells.jsonl
  hierarchy/curated.jsonl
  sets/sets.jsonl
  sets/curated.jsonl
```

The remaining groups of the suite (`edges`, `vertexes`, `regions`, the
other `floats` files and `digests`) are added group by group; the generator already writes all
of them, and each is specified here when its files land.

`manifest.json` is the only file a consumer opens by name. Every record file
is listed in it with its line count and SHA-256, and a consumer must refuse a
file whose hash does not match. Record files are grouped in one directory per
function family; a group directory may hold several files. A `curated.jsonl`
holds hand-chosen subjects in its group's record type (see Curated files);
a consumer runs every file of a group the same way.

### Manifest

```json
{
  "format": "h3-conformance/1",
  "h3Version": "4.5.0",
  "generator": {"name": "src/apps/testapps/mkConformance.c", "seed": 20260929},
  "tolerances": {"angularDeg": 1e-12, "relative": 1e-9},
  "files": {"inspection/cells.jsonl": {"records": 1885, "sha256": "..."}}
}
```

| Key | Meaning |
|---|---|
| `format` | The specification version. A consumer that does not recognize it must not run the suite. |
| `h3Version` | The H3 release the expected values were produced by, without a leading `v`. |
| `generator.name` | The program that produced the files. |
| `generator.seed` | The seed its sampler ran from. Two generators that follow the sampling procedure below with the same seed produce byte-identical record files. |
| `tolerances` | The comparison bounds for floating-point outputs, described under Tolerances. They apply to every file in the suite. |
| `files` | Slash-separated paths relative to the manifest, each with its number of lines and the lowercase hex SHA-256 of its bytes. |

## Record files

A record file is UTF-8 text containing one JSON object per line. Every line,
including the last, ends in a single `\n`. There are no blank lines and no
comments. Generators write keys in the order given in the group definition
and no whitespace outside strings; consumers must not depend on either.

Each line describes one **subject** (an index, a pair of indexes, a point) and
carries the expected output of every function in the group as a sibling key.
Every key of a group is present on every line of that group's files. Nothing
is optional and nothing is inferred from absence.

### Value conventions

- **Indexes** are strings in the form `h3ToString` produces: lowercase
  hexadecimal, no `0x` prefix, no leading zeros. A consumer parses them with
  `stringToH3` and must also check that formatting the parsed value gives the
  same string back.
- **Errors** are objects with a single key: `{"err": "E_CELL_INVALID"}`. The
  value is the `H3Error` enum name, never its number. A key whose function can
  fail holds either the function's output or an error object.
- **Booleans** are JSON `true` and `false`, for functions that return
  `int` used as a boolean in C.
- **Sets** are JSON arrays whose order is not significant. Generators write
  them in ascending order without duplicates; consumers compare them as
  sets. Where the C API returns a sparse array padded with a sentinel (`-1`
  for faces, `0` for indexes), the padding is removed before writing and
  must be ignored when comparing.
- **Sequences** are JSON arrays whose order is significant, for example cell
  boundary vertices or the cells of a grid path. The group definition says
  which arrays are sets and which are sequences.
- **Floating-point** values are JSON numbers with enough digits to round-trip
  a binary64. They are compared using the manifest's tolerances, never for
  equality.
- **Points** are `[latitude, longitude]` pairs in degrees, both as inputs and
  as outputs. A consumer converts to whatever its implementation takes.

### Tolerances

The reference implementation is not bit-reproducible across platforms: the
same C source built for x86-64 and arm64 differs in the last bits of most
coordinate outputs, because of fused multiply-add and libm differences. A
port therefore cannot be asked to match float outputs exactly, and the bound
it is asked to meet has to be one the reference itself meets everywhere.

- **Coordinates** (`cellToLatLng`, `cellToBoundary`, `vertexToLatLng` and any
  other latitude/longitude output) are compared by the **angular distance on
  the sphere** between the expected point and the actual point, in degrees,
  which must not exceed `tolerances.angularDeg`. Angular distance is the
  great-circle separation of the two points, equivalently the angle between
  their unit vectors. It is never compared as separate latitude and longitude
  differences: those are amplified by `1/cos(latitude)` near the poles, so a
  per-coordinate bound that is met at the equator is unmeetable at high
  latitudes and fine resolutions, for the reference as much as for any port.
- **Lengths and areas** (`cellArea*`, `edgeLength*`, `greatCircleDistance*`,
  the `getHexagon*Avg*` functions) are compared by relative error,
  `|actual - expected| / |expected|`, which must not exceed
  `tolerances.relative`, **or** by an absolute floor derived from the angular
  tolerance, whichever is looser. The floor exists because a fine-resolution
  edge is the distance between two points a few times 1e-7 radians apart,
  each carrying its own last-bit error: the difference inherits an absolute
  error near 1e-16 radians, which is a relative error near 1e-9 that no
  implementation, the reference included, can hold below `relative`. With
  `A` the angular tolerance in radians and `R` the earth radius the library
  uses, 6371.007180918475 km, the floor is `A` for a length in radians,
  `A * R` in kilometres and `A * R * 1000` in metres; for an area it is
  `A * sqrt(expected)` in steradians, `A * sqrt(expected) * R` in square
  kilometres and `A * sqrt(expected) * R * 1000` in square metres, which is
  the area swept by moving a boundary of that size by `A`. An expected value
  of zero therefore requires the actual value to be within the floor of
  zero.

Discrete outputs, which is everything else, are compared exactly.

- **Digests** are lowercase hex SHA-256 strings over a stream of text lines,
  one per subject, each terminated by `\n`. They stand in for discrete
  outputs that are too large to store, and are compared exactly. Within a
  line, fields are separated by single spaces: indexes as index strings,
  integers in decimal, booleans as `1` and `0`, errors as `H3Error` names,
  sets in ascending order, and `|` between consecutive sets. Text rather
  than binary so that a failing implementation can dump its stream and
  `diff` it against the reference's.

## Groups

### `inspection`

One line per 64-bit index. The subject may be a valid cell, a directed edge,
a vertex, or an arbitrary bit pattern; every function below is defined for
all of them and the record holds whatever the reference returned.

| Key | Function | Type |
|---|---|---|
| `index` | the subject, as `h3ToString` formats it | index string |
| `res` | `getResolution` | integer |
| `baseCell` | `getBaseCellNumber` | integer |
| `validCell` | `isValidCell` | boolean |
| `validIndex` | `isValidIndex` | boolean |
| `resClassIII` | `isResClassIII` | boolean |
| `pentagon` | `isPentagon` | boolean |
| `faces` | `getIcosahedronFaces`, with `-1` padding removed | set of integers, or error |
| `digits` | `getIndexDigit` for resolutions 1 through 15 | sequence of 15 integers |
| `construct` | `constructCell(res, baseCell, digits[0..res))` | index string, or error |

`digits` always has fifteen entries, including the slots beyond `res`, because
`getIndexDigit` is defined for every resolution and a valid cell must hold `7`
in the unused slots. `construct` reads only the first `res` of them, so its
round trip back to `index` is expected exactly when `validCell` is true, and
its error otherwise tells the consumer which rule the index breaks.

Example:

```json
{"index":"804dfffffffffff","res":0,"baseCell":38,"validCell":true,"validIndex":true,"resClassIII":false,"pentagon":true,"faces":[2,3,7,8,12],"digits":[7,7,7,7,7,7,7,7,7,7,7,7,7,7,7],"construct":"804dfffffffffff"}
{"index":"fffffffffffffff","res":15,"baseCell":127,"validCell":false,"validIndex":false,"resClassIII":true,"pentagon":false,"faces":{"err":"E_CELL_INVALID"},"digits":[7,7,7,7,7,7,7,7,7,7,7,7,7,7,7],"construct":{"err":"E_BASE_CELL_DOMAIN"}}
```

### `floats`

The floating-point functions, every value compared under the manifest
tolerances: points by angular distance, measures by relative error.
Subjects are valid cells (see Sampling). Further `floats` files cover
directed edges, vertexes, great-circle distances and the per-resolution
averages.

`floats/cells.jsonl`, one line per cell:

| Key | Function | Type |
|---|---|---|
| `index` | the subject cell | index string |
| `center` | `cellToLatLng(index)` | point |
| `boundary` | `cellToBoundary(index)` | sequence of points, in the reference's order |
| `areaRads2`, `areaKm2`, `areaM2` | `cellAreaRads2`, `cellAreaKm2`, `cellAreaM2` | number |
| `interiorRes` | `min(res + 2, 15)` | integer |
| `interior` | one point per boundary vertex, halfway from the center to it (see Sampling) | sequence of points |
| `interiorCells` | `latLngToCell(point, interiorRes)` for each point of `interior`, in order | sequence of index strings |

Example:

```json
{"index":"8009fffffffffff","center":[64.700000127934885,10.536199075467678],"boundary":[[63.095054077525454,-10.444977544778341],[55.706768465152265,5.5236465492903184],[58.401544870352687,25.082722326707898],[68.929957881939814,31.831280499087402],[73.310223685444001,0.32561035194323518]],"areaRads2":0.063123898710068072,"areaKm2":2562182.1629555039,"areaM2":2562182162955.5039,"interiorRes":2,"interior":[[63.897527102730166,0.045610765344669346],[60.203384296543575,8.0299228123789987],[61.55077249914379,17.809460701087787],[66.814979004937356,21.183739787277538],[69.00511190668945,5.4309047137054565]],"interiorCells":["82091ffffffffff","82098ffffffffff","8208affffffffff","8208c7fffffffff","820957fffffffff"]}
```

### `traversal`

One line per **valid cell and target**. The sampled file draws the target
from the cell's 3-disk; the curated file pairs cells by hand, so a target
there may be far away or not a valid cell.
`gridDistance` and `gridPathCells` fail with `E_FAILED` when the pair
straddles a pentagon or is otherwise too far apart in local coordinates;
those rows are the group's error cases.

Only the rings are recorded. The `k`-disk is the cell together with rings 1
through `k`, and `gridDiskDistances(index, k)` grouped by distance is the
cell, then ring 1, and so on, so a consumer checks `gridDisk` and
`gridDiskDistances` for `k` 1 to 3 against sets it builds from the rings.

| Key | Function | Type |
|---|---|---|
| `index` | the subject cell | index string |
| `ring1`, `ring2`, `ring3` | `gridRing(index, k)` for `k` 1, 2, 3, with `0` padding removed | set of index strings |
| `target` | the second cell of the pair | index string |
| `distance` | `gridDistance(index, target)` | integer, or error |
| `path` | `gridPathCells(index, target)`, `gridPathCellsSize` entries | sequence of index strings, or error |
| `neighbor` | `areNeighborCells(index, target)` | boolean, or error |

Example:

```json
{"index":"8001fffffffffff","ring1":["8003fffffffffff","8005fffffffffff","8007fffffffffff","8009fffffffffff","800bfffffffffff","8011fffffffffff"],"ring2":["800dfffffffffff","800ffffffffffff","8013fffffffffff","8015fffffffffff","8017fffffffffff","8019fffffffffff","801bfffffffffff","801ffffffffffff","8021fffffffffff","8025fffffffffff","802dfffffffffff"],"ring3":["801dfffffffffff","8023fffffffffff","8027fffffffffff","8029fffffffffff","802bfffffffffff","802ffffffffffff","8031fffffffffff","8033fffffffffff","8035fffffffffff","8039fffffffffff","803bfffffffffff","803dfffffffffff","803ffffffffffff","8041fffffffffff","8043fffffffffff","8053fffffffffff"],"target":"8005fffffffffff","distance":1,"path":["8001fffffffffff","8005fffffffffff"],"neighbor":true}
```

### `unsafe`

One line per **valid cell**. `gridDiskUnsafe` and `gridRingUnsafe` walk
the grid in a spiral and refuse with `E_PENTAGON` when the walk meets a
pentagon, where the safe functions fall back to a slower search. Which
origins and `k` are refused is a property of the reference's walk, not of
the geometry. The disk walk refuses when any cell it visits is a pentagon.
The ring walk steps `k` cells outward along one axis, circles the ring, and
refuses when a cell on either leg is a pentagon or when the circuit fails to
return to its first cell, which is how it notices distortion from a pentagon
it never steps on. In the generated file the two decisions agree for every
subject; both are recorded because they are separate walks and an
implementation can reproduce one without the other. This file records the
decision. It does not repeat the cells, because
when the unsafe function succeeds the reference returns exactly the safe
function's cells; an implementation checks its unsafe output against its own
safe output, which `traversal` and the digests cover. `gridDiskDistancesUnsafe`
is the function `gridDiskUnsafe` calls and `gridDisksUnsafe` is a loop over
it, so neither has a key: they refuse exactly when `diskUnsafe` does.

| Key | Function | Type |
|---|---|---|
| `index` | the subject cell | index string |
| `diskUnsafe` | `gridDiskUnsafe(index, k)` for `k` 1, 2, 3, in that order | sequence of three: `true` on success, or error |
| `ringUnsafe` | `gridRingUnsafe(index, k)` for `k` 1, 2, 3, in that order | sequence of three: `true` on success, or error |

Example:

```json
{"index":"8003fffffffffff","diskUnsafe":[true,{"err":"E_PENTAGON"},{"err":"E_PENTAGON"}],"ringUnsafe":[true,{"err":"E_PENTAGON"},{"err":"E_PENTAGON"}]}
```

### `localij`

One line per **(origin, target) pair** of valid cells at the same
resolution; the line's subject is the pair. Mode is always 0. Pairs that
cross too many base cells fail with `E_FAILED`, and those rows are the
group's error cases.

| Key | Function | Type |
|---|---|---|
| `index` | the origin cell | index string |
| `target` | the cell being located | index string |
| `ij` | `cellToLocalIj(index, target, 0)` | `{"i": integer, "j": integer}`, or error |
| `cell` | `localIjToCell(index, ij, 0)` when `ij` succeeded; otherwise the same error as `ij` | index string, or error |

Example:

```json
{"index":"8003fffffffffff","target":"8013fffffffffff","ij":{"i":-1,"j":0},"cell":"8013fffffffffff"}
{"index":"8003fffffffffff","target":"8001fffffffffff","ij":{"i":1,"j":0},"cell":"8001fffffffffff"}
```

### `hierarchy`

One line per **valid cell**. Resolution bounds are the only source of errors:
a resolution-0 cell has no parents, and a resolution-15 cell has no children.

| Key | Function | Type |
|---|---|---|
| `index` | the subject cell | index string |
| `parents` | `cellToParent(index, r)` for `r` from 0 to `res - 1`, in that order | sequence of index strings |
| `centerChild` | `cellToCenterChild(index, res + 1)` | index string, or error |
| `children` | `cellToChildren(index, res + 1)`, with `0` padding removed | set of index strings, or error |
| `childPos` | `cellToChildPos(index, 0)` | integer, or error |

A consumer must also check the round trip `childPosToCell(childPos, parents[0],
res) == index` (using `index` itself as the parent when `res` is 0) whenever
`childPos` succeeded.

Example:

```json
{"index":"8100bffffffffff","parents":["8001fffffffffff"],"centerChild":"820087fffffffff","children":["820087fffffffff","82008ffffffffff","820097fffffffff","82009ffffffffff","8200a7fffffffff","8200affffffffff","8200b7fffffffff"],"childPos":2}
```

### `sets`

One line per **input set**, built from a subject cell in one of six kinds.
The `id` is the kind and the cell, joined by `:`. `input` is a sequence
because duplicates and invalid entries are part of the test; the two
function outputs are sets.

| Key | Function | Type |
|---|---|---|
| `id` | `<kind>:<cell>` | string |
| `input` | the cells passed to both functions, in order | sequence of index strings |
| `compact` | `compactCells(input)` | set of index strings, or error |
| `uncompactRes` | the resolution passed to `uncompactCells` | integer |
| `uncompact` | `uncompactCells(input, uncompactRes)` | set of index strings, or `true` when that set is `input` as a set, or error |

When `uncompactRes` is the resolution of `input`, `uncompactCells` returns
the input itself, and the file writes `true` rather than repeating it. A
consumer that reads `true` compares against `input` with padding removed,
sorted and deduplicated.

The kinds, for a subject at resolution `res`:

| Kind | `input` | `uncompactRes` |
|---|---|---|
| `children` | `cellToChildren(cell, min(res + 2, 15))` | `min(res + 2, 15)` |
| `disk` | `gridDisk(cell, 2)` in slot order | `res + 1` |
| `holes` | `gridDisk(cell, 3)` in slot order with one to three slots cleared (see Sampling) | `res` |
| `duplicate` | `cellToChildren(p, res(p) + 1)` followed by its first entry again, where `p` is the cell, or its parent when `res` is 15 | `res(p) + 1` |
| `invalid` | `gridDisk(cell, 1)` in slot order followed by the cell with reserved bits 56-58 set to `001` | `res` |
| `coarse` | `gridDisk(cell, 1)` in slot order | `res - 1` |

Padding is removed from every input. `duplicate` fails `compactCells` with
`E_DUPLICATE_INPUT`; `invalid` fails it with `E_CELL_INVALID` except at
resolution 0, where the reference copies the input through unchecked;
`coarse` fails `uncompactCells` with `E_RES_MISMATCH`, as does `disk` at
resolution 15. Mixed-resolution input to `compactCells` is outside its
contract and is not sampled.

A consumer must also check the round trip `uncompactCells(compact, r) ==
input` as sets, where `r` is the finest resolution in `input`, whenever
`compact` succeeded and every entry of `input` is a valid cell.

Example:

```json
{"id":"coarse:8009fffffffffff","input":["8007fffffffffff","8009fffffffffff","8019fffffffffff","8001fffffffffff","801ffffffffffff","8011fffffffffff"],"compact":["8001fffffffffff","8007fffffffffff","8009fffffffffff","8011fffffffffff","8019fffffffffff","801ffffffffffff"],"uncompactRes":-1,"uncompact":{"err":"E_RES_MISMATCH"}}
{"id":"children:8f0800000000000","input":["8f0800000000000"],"compact":["8f0800000000000"],"uncompactRes":15,"uncompact":true}
{"id":"duplicate:81083ffffffffff","input":["820807fffffffff","820817fffffffff","82081ffffffffff","820827fffffffff","82082ffffffffff","820837fffffffff","820807fffffffff"],"compact":{"err":"E_DUPLICATE_INPUT"},"uncompactRes":2,"uncompact":["820807fffffffff","820817fffffffff","82081ffffffffff","820827fffffffff","82082ffffffffff","820837fffffffff"]}
```

## Curated files

Seeded sampling gives coverage density but is blind to the particular inputs
that have broken implementations. A `curated.jsonl` in a group directory
holds such inputs by hand. Each has exactly the record type of its group, so a
consumer runs it with the group's checks and nothing else. The expected
values are the reference's answers, produced by the same generator as every
other file; only the subjects are chosen. The tables below are the
specification: subjects are written in the order given, never sorted or
deduplicated, and the generator draws nothing for them.

Sources: *cli* is the command-line test suite (`tests/cli/*.txt`), *tests*
the C test applications (`src/apps/testapps/*.c`), *inputfiles* the test
input directory (`tests/inputfiles/`), and *hand* a choice made here.

### `inspection/curated.jsonl`

| Index | Source | Why |
|---|---|---|
| `85283473fffffff` | cli | the cell every command-line example uses |
| `85283473ffff` | cli | the `isValidCell` false example, a truncated string |
| `115283473fffffff` | cli | a directed edge of that cell |
| `22528340bfffffff` | cli | a vertex of a neighbour |
| `8f754e64992d6d6` | cli | the `getIndexDigit` example at res 15 |
| `81743ffffffffff` | cli | the `getIcosahedronFaces` example |
| `8928342e20fffff` | cli | the res 9 example |
| `5` | cli | the invalid index in every error example |
| `200f202020202020` | tests | an invalid `cellToLocalIj` origin |
| `80c3fffffffffff` | tests | a res 0 pentagon |
| `8f283080dcb0ae2` | tests | a res 15 cell |
| `88283080ddfffff` | tests | the `cellToChildren` fixture |
| `81083ffffffffff` | tests | a res 1 pentagon |
| `81087ffffffffff` | hand | that pentagon with digit 1 set to `1`, a deleted subsequence |
| `82080ffffffffff` | hand | its res 2 center child with digit 2 set to `1` |
| `8029fffffffffff` | tests | the res 0 `gridDisk` fixture |
| `85283472fffffff` | cli | an `areNeighborCells` target whose digit 6 is not `7` |

### `traversal/curated.jsonl`

| Origin | Target | Source | Why |
|---|---|---|---|
| `85283473fffffff` | `8528342bfffffff` | cli | the `gridDistance` and `gridPathCells` example |
| `85283473fffffff` | `85291ac7fffffff` | cli | the far `gridDistance` example |
| `85283473fffffff` | `85283477fffffff` | cli | the `areNeighborCells` true example |
| `85283473fffffff` | `85283472fffffff` | cli | an invalid target |
| `85285aa7fffffff` | `851d9b1bfffffff` | tests | a `gridPathCells` fixture across faces |
| `820807fffffffff` | `8208e7fffffffff` | tests | a `gridPathCells` fixture from a pentagon |
| `8411b61ffffffff` | `84016d3ffffffff` | tests | a `gridPathCells` fixture |
| `820c4ffffffffff` | `821ce7fffffffff` | tests | a `gridDistance` fixture |
| `832830fffffffff` | `822837fffffffff` | tests | a resolution mismatch |
| `832830fffffffff` | `832834fffffffff` | tests | a `gridDistance` fixture |
| `8029fffffffffff` | `8051fffffffffff` | tests | res 0 cells on different faces |
| `80c3fffffffffff` | `80c3fffffffffff` | hand | a pentagon to itself |
| `81083ffffffffff` | `8108bffffffffff` | hand | a res 1 pentagon to its neighbour |
| `8f283080dcb0ae2` | `8f283080dcb0ae2` | hand | a res 15 cell to itself |

### `localij/curated.jsonl`

| Origin | Target | Source | Why |
|---|---|---|---|
| `85283473fffffff` | `8528342bfffffff` | cli | the `cellToLocalIj` example, `[25, 13]` |
| `8029fffffffffff` | `8029fffffffffff` | tests | a res 0 origin to itself |
| `8029fffffffffff` | `8051fffffffffff` | tests | res 0 cells on different faces |
| `820897fffffffff` | `821f67fffffffff` | tests | a `cellToLocalIj` fixture near a pentagon |
| `85283473fffffff` | `85291ac7fffffff` | cli | the far pair |
| `832830fffffffff` | `832834fffffffff` | tests | a `gridDistance` fixture |
| `820807fffffffff` | `8208e7fffffffff` | tests | a pentagon origin |
| `80c3fffffffffff` | `80c3fffffffffff` | hand | a pentagon to itself |
| `81083ffffffffff` | `8108bffffffffff` | hand | a res 1 pentagon to its neighbour |
| `85285aa7fffffff` | `851d9b1bfffffff` | tests | a pair across faces |
| `85283473fffffff` | `85283472fffffff` | cli | an invalid target |
| `200f202020202020` | `85283473fffffff` | tests | an invalid origin |

### `hierarchy/curated.jsonl`

`88283080ddfffff`, `8f283080dcb0ae2`, `81083ffffffffff`, `820807fffffffff`,
`85283473fffffff`, `80c3fffffffffff`, `8928342e20fffff`, `8029fffffffffff`:
the `cellToChildren` fixtures, a res 15 cell, a res 1 pentagon and its
res 2 center child, the command-line cells, and res 0 cells.

### `sets/curated.jsonl`

The `id` is `curated:<name>`; the `input` is in the record, in this order.

| Name | `input` | `uncompactRes` | Source |
|---|---|---|---|
| `compact_test1` | the 19 res 5 cells of `compact_test1.txt`, which include all seven children of `8428347ffffffff` | 6 | inputfiles |
| `multipolygon_test3` | the 6 res 5 cells of `multipolygon_test3.txt`, a ring without its center | 6 | inputfiles |
| `multipolygon_test4` | the 7 cells of `multipolygon_test4.txt`, that ring and a distant cell | 6 | inputfiles |
| `multipolygon_test5` | the 128 res 5 cells of `multipolygon_test5.txt` | 6 | inputfiles |
| `pentagon_children` | the six children of the res 1 pentagon `81083ffffffffff` | 3 | hand |
| `res0` | `getRes0Cells` in the reference's order | 1 | hand |

## Sampling

Inputs are chosen by a deterministic procedure from `generator.seed`, so that
a generator written against the C library directly reproduces this suite byte
for byte, and so that a failing line can be traced to the step that produced
it. The procedure for each file is given here in full. Every file starts
from a fresh generator whose state is the seed, so files can be produced
independently and in any order.

### Pseudo-random source

The sampler is splitmix64 with the seed as its initial state. Each draw
returns 64 bits:

```
state += 0x9e3779b97f4a7c15
z = state
z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9
z = (z ^ (z >> 27)) * 0x94d049bb133111eb
return z ^ (z >> 31)
```

All arithmetic is modulo 2^64. "Draw `n`" below means one draw reduced by
`% n`. Steps consume draws in exactly the order written; a step that says it
draws nothing must not touch the generator.

### Index construction

`build(mode, res, baseCell, digits)` assembles a 64-bit index: bit 63 clear,
bits 59-62 the mode, bits 56-58 zero, bits 52-55 the resolution, bits 45-51
the base cell, then fifteen 3-bit digit slots from bit 42 down to bit 0, the
first `res` slots holding `digits` in order and the rest holding `7`.

"Draw a cell at `res`" means: draw 122 for the base cell, then draw 7 for
each of `res` digits in order, and `build(1, res, baseCell, digits)`. The
result may be invalid: a pentagon base cell with a leading `1` digit is a
deleted subsequence, and such draws are kept.

### `inspection/cells.jsonl`

1. The fixed indexes `0`, `ffffffffffffffff`, `7fffffffffffffff`,
   `800000000000000`, `8001fffffffffff`, `81283ffffffffff`,
   `804dfffffffffff` and `fffffffffffffff`. No draws.
2. `build(1, 0, b, [])` for every base cell `b` from 0 to 121. No draws.
3. `getPentagons(res)` for `res` 1 through 15, in the order the reference
   returns them. No draws.
4. For `res` 1 through 15: draw 48 cells at `res`.
5. For `res` 0 through 15, four times: draw a cell at `res` as the base, then
   emit these mutations of it in order, each drawing only as stated:
   1. mode field replaced by draw 16;
   2. reserved bits (56-58) replaced by 1 + draw 7;
   3. bit 63 set, no draw;
   4. base cell field replaced by 122 + draw 6;
   5. if `res > 0`: digit slot 1 + draw `res` set to 7;
   6. if `res < 15`: digit slot `res` + 1 + draw (15 - `res`) set to draw 7.
6. For `res` 0 through 15, four times: draw cells at `res` until
   `isValidCell` accepts one; take its `originToDirectedEdges` and
   `cellToVertexes` arrays (six slots each, a zero where a pentagon has no
   edge or vertex); then twice, draw 6 and emit the edge at that slot if it is
   non-zero; then twice, draw 6 and emit the vertex at that slot if it is
   non-zero.
7. 256 raw draws, each emitted as an index.

The emitted indexes are sorted ascending as unsigned 64-bit integers and
duplicates are removed. Each remaining index becomes one line.

### Valid-cell subjects

Several files share one way of choosing subjects, parameterized by a count
`n` per resolution:

1. `build(1, 0, b, [])` for every base cell `b` from 0 to 121. No draws.
2. `getPentagons(res)` for `res` 1 through 15, in the order the reference
   returns them. No draws.
3. For `res` 1 through 15, `n` times: draw cells at `res` until `isValidCell`
   accepts one, and emit it.

Sorted ascending and deduplicated as above. The draws a file's records need
beyond this happen afterwards, for each subject in file order.

"Draw from the `k`-disk of `h`" means: take the `gridDisk(h, k)` array of
`maxGridDiskSize(k)` slots as the reference fills it, including its `0`
padding, and draw `maxGridDiskSize(k)` repeatedly until the slot it selects
is non-zero.

### `floats/`

`cells.jsonl`: valid-cell subjects with `n` = 16. No further draws. The
interior points are computed in degrees, with factor 0.5: with center
`(clat, clng)` and vertex `(vlat, vlng)`, the point is
`(clat + 0.5 * (vlat - clat), wrap(clng + 0.5 * wrap(vlng - clng)))`, where
`wrap` adds or subtracts 360 to bring a value into [-180, 180], one per
vertex of `cellToBoundary` in its order. Degrees are what the file holds, so
a generator converts the points to its implementation's units rather than
interpolating in radians.

### `traversal/cells.jsonl`

Valid-cell subjects with `n` = 16. Then for each subject, `target` is drawn
from its 3-disk.

### `unsafe/cells.jsonl`

Valid-cell subjects with `n` = 16. Then, with no further draws, every
non-zero cell of `gridDisk(p, 4)` for every pentagon `p` at every resolution
0 through 15, in `getPentagons` order. The whole list is sorted ascending and
deduplicated again. The 4-disks put every cell within one step of the
largest disk or ring the file asks about, which is where the unsafe
traversals change their answer.

### `localij/pairs.jsonl`

Valid-cell subjects with `n` = 16 are the origins. Then for each origin, a
near target is drawn from its 3-disk, and a far target is drawn by drawing
cells at the origin's resolution until `isValidCell` accepts one. The origin
is written with the near target, then with the far target unless the two
targets are the same cell.

### `hierarchy/cells.jsonl`

Valid-cell subjects with `n` = 32. No further draws.

### `sets/sets.jsonl`

Subjects:

1. `getPentagons(res)` for `res` 0 through 15, in the order the reference
   returns them. No draws.
2. For `res` 0 through 15, twice: draw cells at `res` until `isValidCell`
   accepts one, and emit it.

Sorted ascending and deduplicated as above. Then for each subject, the six
kinds are written in the order of the kinds table. Only `holes` draws: draw 3
to get a count from 1 to 3, then that many times draw 37 and clear that slot
of the `gridDisk(cell, 3)` array; a slot that is already zero stays zero.

### Curated files

No draws. The subjects are the tables under Curated files, written in the
order given.

## Running and regenerating

The runners are ordinary test apps. `ctest -L File` runs every test that
reads an input file, including the conformance runners and the manifest
check; `make test-files` is the same thing. Each runner takes one record
file and exits non-zero at the first record that differs, naming the file,
the line, the key and both values. Discrete groups must pass bit for bit on
every platform CI builds; float outputs must pass under the manifest
tolerances. A failure on one platform only is cross-platform float drift,
localised to a group and a row.

`mkConformance`, built with the other generators (`BUILD_GENERATORS`),
rewrites the suite from the library it is linked against:

```
mkConformance -o tests/inputfiles [-s <seed>] [-g <group-or-file>]...
```

The group directories must already exist. `-g` restricts the run to one
group (`inspection`) or one file (`floats/cells.jsonl`) and may be repeated;
the manifest then lists only the selected files. Without `-g` every file is
written, which takes about ten minutes because of the digests group. The
manifest's `h3Version` comes from the library's version macros, so
regenerating after a release bump is required and the manifest check says
so. Expect a diff only when the library's behaviour changed; a diff in a
discrete file is a behaviour change to explain in the pull request, and a
diff in a `floats` file that the runners still accept is last-bit drift
between the machine that generated the files and this one.

`scripts/conformance_check.sh <path-to-mkConformance> [group]...`
regenerates the suite into a temporary directory and compares the discrete
files byte for byte against the committed ones, and fails on a committed
file the generator no longer writes; the gcc CI jobs run it on Linux
x86-64 and arm64, in Debug and Release, on every change. `floats` files
are excluded from the byte comparison on purpose, since the runners
already judge them under tolerance.

## Consumers

Ports consume the files, never the generator: a port needs a JSON parser
and this document. `manifest.json` is the entry point; a consumer must
refuse a file whose hash does not match and must not run a suite whose
`format` it does not recognise. The files are regenerated per H3 release,
so a port pins the release it claims conformance with, for example by a
submodule of this repository at the release tag.

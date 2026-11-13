# Spec: Voxels (altitude-aware H3)

Status: ready-for-agent

## Problem Statement

H3 indexes the Earth's surface, but many of the things people need to index are above or below it: drones, aircraft, weather observations, satellites. Today there is no H3-compatible way to say "this volume of space" — users either flatten altitude away, bolt an ad-hoc altitude bucket onto a cell index, or adopt a different 3D system that doesn't interoperate with the H3 cells they already store. Any solution must leave every existing H3 index and function bit-for-bit unchanged, must leave no gaps in space, and must be easy to use from many languages and databases.

## Solution

Add **voxels** to this fork of H3. A voxel is one **cell** (its **footprint**) combined with one **layer** of the same resolution; the voxels over one footprint form a **column**. Columns rise along the WGS84 ellipsoid normal, layers grow thicker with distance from the Earth's centre so voxels keep roughly cube-like proportions from the ground to geostationary orbit and beyond, and layers nest exactly across resolutions. A voxel is identified by a single 64-bit integer, like a cell, so it fits a signed `BIGINT` and every language's 64-bit integer type.

Resolutions 0–10 cover the full **altitude range** (about −1,780 km to +443,000 km). Resolutions 11–13 cover narrow ranges near the ground (up to about +1,440 km, +81 km and +2.8 km). Resolutions 14–15 have no voxels. Within its own altitude range, every point of space lies in exactly one voxel at each resolution.

Callers can locate a voxel from ellipsoidal altitude directly, or from mean-sea-level or above-ground altitude by supplying a geoid or terrain lookup.

See `GLOSSARY.md` and ADR-0001 to ADR-0006 in `docs/adr/`.

## User Stories

### Locating voxels

1. As a drone operator, I want to turn a GPS lat/lng/altitude into a voxel at a chosen resolution, so that I can index my drone's position in 3D.
2. As a GPS user, I want ellipsoidal altitude to be the default altitude type, so that raw GPS fixes need no conversion.
3. As an aviation developer, I want to locate a voxel from a mean-sea-level altitude by supplying a geoid lookup, so that barometric/MSL altitudes land in the correct voxel.
4. As a drone developer, I want to locate a voxel from an above-ground altitude by supplying a terrain lookup, so that "50 m above the ground" lands in the correct voxel wherever I fly.
5. As a library user, I want a clear error when I request a mean-sea-level or above-ground altitude without supplying the matching lookup, so that I don't silently get the wrong voxel.
6. As a library user, I want an error from my lookup function to be passed back to me, so that I can tell a lookup failure from a bad input.
7. As a library user, I want a distinct error when a point lies outside the altitude range of the resolution I asked for, so that I can retry at a coarser resolution.
8. As a satellite operator, I want to locate voxels for objects in geostationary, Tundra, Molniya and high science orbits (up to about 443,000 km), so that my whole fleet is indexable in one system.
9. As a geophysicist, I want points down to about −1,780 km to have voxels at resolutions 0–10, so that subsurface data is indexable too.
10. As a mapping user, I want a ship at sea level anywhere on Earth to have a voxel at every resolution 0–13, so that the geoid sitting below the ellipsoid never pushes sea level out of range.
11. As a library user, I want a point exactly on a layer boundary to belong to the layer above it, so that classification is well defined.
12. As a library user, I want the voxel's footprint to equal the cell that `latLngToCell` returns for the same lat/lng and resolution, so that voxels and my existing cells line up exactly.
13. As a library user, I want to build a voxel directly from a cell and a layer number, so that I can construct voxels without going through lat/lng/altitude.
14. As a library user, I want to find the layer number that contains a given ellipsoidal altitude at a resolution, so that I can reason about columns without a lat/lng.

### Inspecting voxels

15. As a library user, I want to check whether a 64-bit value is a valid voxel, so that I can reject corrupt or foreign data.
16. As a library user, I want existing cell validity checks to reject voxels, so that voxels are never mistaken for cells.
17. As a library user, I want to get a voxel's resolution, so that I can group voxels by resolution.
18. As a library user, I want to get a voxel's footprint cell, so that I can use every existing 2D H3 function on it.
19. As a library user, I want to get a voxel's signed layer number counted from altitude zero, so that "layer −1" always means "the first layer below zero" at any resolution.
20. As a library user, I want to get a voxel's floor and ceiling altitudes, so that I know exactly which altitudes it covers.
21. As a library user, I want to get the altitude range of any resolution, so that I know where voxels exist before indexing.
22. As an analyst, I want a voxel's centre as lat/lng plus altitude, so that I can plot or label it.
23. As an analyst, I want a voxel's volume in cubic kilometres and cubic metres, consistent with H3's cell areas, so that densities computed per voxel agree with densities computed per cell.

### Strings and storage

24. As a database engineer, I want a voxel to be a single 64-bit integer with the top bit clear, so that it fits a signed `BIGINT` column.
25. As a database engineer, I want all voxels in a column to sort contiguously, so that "everything over this cell" is a single range scan.
26. As a database engineer, I want voxels to sort first by footprint in H3's order and then by altitude, so that spatial locality from H3 is preserved.
27. As a developer, I want voxels to convert to and from the same hex string form as H3 indexes, so that logs, URLs and JSON handle them identically.
28. As a binding author, I want voxels to use the same 64-bit index type as cells, so that bindings can reuse their existing index handling.

### Hierarchy

29. As a library user, I want a voxel's parent at any coarser resolution, so that I can aggregate upward.
30. As a library user, I want a parent voxel's layer to be exactly covered by its children's layers, so that vertical aggregation has no overlaps or gaps.
31. As a library user, I want a voxel's children at any finer resolution up to 13, plus their count, so that I can refine.
32. As a library user, I want asking for children outside the finer resolution's altitude range to return an empty set rather than an error, so that refining a column near a range boundary needs no special-casing.
33. As a library user, I want every voxel to have either all of its children or none, so that compacting is always reversible.
34. As a library user, I want to compact a set of voxels into the coarsest equivalent set and uncompact it back, so that I can store large volumes efficiently.
35. As a library user, I want compacting to reject duplicate input, consistent with H3's `compactCells`, so that errors behave the same way.

### Neighbours

36. As an airspace manager, I want the voxel disk of radius k around a voxel (footprints within k grid steps and layers within k), so that I can find everything near an aircraft.
37. As a library user, I want the maximum voxel disk size for a given k, so that I can allocate output buffers the same way as for `gridDisk`.
38. As a library user, I want voxel disks to be truncated, not errors, at the ends of the altitude range, so that queries near the floor or ceiling just work.
39. As a library user, I want voxel disks around pentagon footprints to work, so that I don't need special handling for them.
40. As a simulation developer, I want a voxel's face neighbours (up to six beside, one above, one below), so that I can run cheap 3D adjacency algorithms.

### Command line

41. As a command-line user, I want CLI subcommands for each voxel function, so that I can explore voxels without writing C.

### Compatibility

42. As an existing H3 user, I want every existing index and function to behave exactly as before, so that adopting this fork is risk-free.
43. As the fork maintainer, I want voxel code isolated in its own files and its constants taken from the far end of H3's number spaces, so that merging upstream releases stays conflict-free.

## Implementation Decisions

### Modules

- **Voxel module (new)**: all voxel logic, in new source files. Built into the same library as the H3 core.
- **Voxel public header (new)**: a separate public header installed next to the H3 API header, declaring voxel functions, types and error codes. The upstream API header is not edited.
- **CLI**: voxel subcommands are added to the existing `h3` CLI in one appended block.
- **Build/test registration**: the only edits to upstream build files are adding voxel sources and tests to the existing lists.

### Geometry and layers (ADR-0002, ADR-0003)

- A point is in a voxel when its WGS84 geodetic lat/lng falls in the footprint (via the existing `latLngToCell`) and its ellipsoidal altitude falls in the layer.
- Layers are uniform in log-radius: with `a` = WGS84 semi-major axis (6,378,137 m), define `u(h) = ln(1 + h/a)`. At resolution r, layer n spans `u ∈ [n·Δu_r, (n+1)·Δu_r)`, so its floor altitude is `a·(exp(n·Δu_r) − 1)`.
- `Δu_0 = √(H3's average resolution-0 hexagon area) / a ≈ 0.32728`, hard-coded as a full-precision constant (not recomputed at runtime).
- `Δu_r = Δu_0 / K_r`, where `K_r` is the product of the split schedule for res 1..r: `3 2 3 3 2 3 3 3 2 3 3 2 3 3 2` (only the first 13 entries are used).
- Layers include the floor and exclude the ceiling. Points within roughly 1e-9 relative of a boundary may classify differently across platforms; this is documented as H3 does for cell edges.

### Altitude ranges (ADR-0003, ADR-0004)

Layer numbers are signed, counted from altitude zero (layer 0 is the first above zero). The valid range of layer numbers at each resolution:

| Resolution | Valid layer numbers n | Approx. altitudes |
|---|---|---|
| 0–10 | −K_r ≤ n < 13·K_r | −1,780 km to +443,000 km |
| 11 | −15 ≤ n < 32,751 | −597 m to +1,440 km |
| 12 | −30 ≤ n < 4,066 | −597 m to +81 km |
| 13 | −90 ≤ n < 420 | −597 m to +2.8 km |
| 14–15 | none | — |

Each narrow range is made of whole layers of the next coarser resolution, and every narrow range shares the same floor (five resolution-10 layers below zero).

### Index layout (ADR-0004, ADR-0005, ADR-0006)

- A voxel index is a 64-bit unsigned integer of the existing H3 index type, laid out from the high bit down as: `0` (1 bit) | mode = 15 (4 bits) | resolution (4 bits) | base cell (7 bits) | digits (3 bits × resolution) | stored layer (the remaining 48 − 3·resolution bits).
- The stored layer is `n − n_min(resolution)`: unsigned, counted from the bottom of that resolution's range.
- Voxels therefore sort by resolution, then by footprint in H3 order, then by altitude. Each column is one contiguous range.
- Recovering the footprint shifts the digits back into cell layout, sets mode 1, clears the reserved bits and fills unused digits with 7.
- A voxel is valid when: bit 63 is 0; mode is 15; resolution is 0–13; the reconstructed footprint is a valid cell; and the layer number is within that resolution's range.
- Upstream inspection functions (`getResolution`, `getBaseCellNumber`, `isValidCell`, etc.) are not voxel-aware and must not be used on voxels. `isValidCell` already returns false for voxels because of the mode.

### Public API contract

Names follow H3's naming conventions (RFC in `dev-docs/RFCs/v4.0.0`). All functions return `H3Error`, with outputs passed through pointers.

- **Locating**: `latLngAltToVoxel(latLng, altitude metres, resolution, altitude options, out)`. The altitude options carry the altitude type (ellipsoidal by default, mean sea level, or above ground) plus an optional lookup and an opaque user-data pointer.
  - The lookup takes a lat/lng and the user data, and returns either the geoid separation (mean sea level) or the terrain's ellipsoidal height (above ground) in metres, or an error.
  - Ellipsoidal altitude = mean-sea-level altitude + geoid separation, or terrain ellipsoidal height + above-ground altitude.
- **Constructing**: `cellAndLayerToVoxel(cell, layer number, out)`, plus `altitudeToLayerNumber(altitude, resolution, out)`.
- **Inspecting**:
  - `isValidVoxel` (returns an int, like `isValidCell`)
  - `getVoxelResolution`
  - `voxelToFootprint`
  - `getLayerNumber`
  - `voxelToAltitudeBounds` (floor, ceiling in metres)
  - `getAltitudeRange(resolution)` (floor, ceiling in metres)
  - `voxelToCenter` (lat/lng and altitude; the centre is the footprint's centre at the mid-point between floor and ceiling)
  - `voxelVolumeKm3` / `voxelVolumeM3`
- **Volume**: measured on H3's sphere (radius R as used by the cell-area functions), as `cellArea · ((R+ceiling)³ − (R+floor)³) / (3R²)`. At altitude zero a column's cross-section area therefore equals H3's cell area.
- **Strings**: the existing `h3ToString` / `stringToH3` are used unchanged. Voxels share the same hex form.
- **Hierarchy**:
  - `voxelToParent`: footprint via `cellToParent`; layer by floor division by `K_child / K_parent`.
  - `voxelToChildrenSize` / `voxelToChildren`: footprint children × child layers `n·K … n·K + K − 1`. Returns an empty set when the child layers are outside the child resolution's range. A child resolution above 13 gives a resolution-domain error.
  - `compactVoxels`, `uncompactVoxelsSize`, `uncompactVoxels`: the same semantics and errors as the cell equivalents.
- **Neighbours**:
  - `maxVoxelDiskSize(k)` = `maxGridDiskSize(k) · (2k + 1)`.
  - `voxelDisk(origin, k, out)`: footprints from `gridDisk`, layers n−k…n+k clipped to the range. Unused output slots are zero, as with `gridDisk`.
  - `voxelFaceNeighbors(origin, out)`: the footprint's `gridDisk(1)` excluding itself, plus layers n±1 within the range.

### Errors (ADR-0006)

New error codes live in the voxel header, in a block far above upstream's codes (e.g. starting at 1000):
- **voxel invalid**: the argument is not a valid voxel.
- **altitude domain**: the altitude is outside the resolution's altitude range, or the resolution has no voxels.
- **lookup failed**: a caller-supplied lookup reported an error.

A missing lookup for mean-sea-level or above-ground altitude returns the existing option-invalid error. Bad lat/lng and resolution arguments use the existing upstream codes.

### Altitude-type conversions (ADR-0001)

The core ships no geoid or terrain data. Conversions happen only through the caller-supplied lookup.

## Testing Decisions

- **Seams**:
  - The **public C voxel API** is the main seam. It's tested through new suites in the existing test-app framework (`SUITE` / `TEST` / `t_assert`), registered in the existing CMake test lists.
  - The **CLI** is a thin second seam, with one smoke test per voxel subcommand via the existing CLI-test macro and `.txt` registration files.
  - No internal-only tests unless some layer math can't be reached through the public API.
- **What makes a good test**: assert on externally visible behaviour (returned voxels, layer numbers, bounds, error codes, set equality, sort order) and never on bit positions or private helpers. A test should survive a rewrite of the internals.
- **Suites**, each following its nearest upstream prior art:
  - **Indexing** (prior art: the lat/lng-to-cell and vertex tests):
    - footprint equals `latLngToCell`
    - points just below, on and just above a layer boundary
    - points at and beyond each resolution's range ends
    - res 14/15 and out-of-range resolutions
    - all three altitude types, using stub lookups that return fixed values or errors
    - missing lookup
    - pentagon footprints
  - **Inspection** (prior art: inspection and area tests):
    - validity rejects bit 63 set, wrong mode, res > 13, invalid footprint digits and out-of-range layers
    - `isValidCell` rejects voxels
    - footprint, layer number, bounds and centre round-trip: the centre maps back to the same voxel
    - the bounds of adjacent layers meet exactly
    - volumes are positive, and a parent's volume equals the sum of its children's volumes (within a tolerance)
  - **Hierarchy** (prior art: the cell parent/children and compact tests):
    - parent of a child is the original
    - children's layers exactly tile the parent's layer
    - empty children outside the narrow ranges
    - children-size matches the children count
    - compact/uncompact round-trip, including duplicate-input errors
    - a resolution-10 voxel at the edge of the resolution-11 range has all of its children or none
  - **Neighbours** (prior art: the grid disk tests):
    - disk size for interior voxels equals `maxVoxelDiskSize`
    - truncation at the floor and ceiling
    - pentagon footprints
    - face neighbours count 8, or 7 at pentagons, and fewer at the range ends
  - **Ordering**: after sorting a set of voxels as unsigned integers, each column is contiguous and ordered by layer.
- **Regression**: the whole existing upstream test suite must pass unchanged. This is the proof of "existing H3 bit-for-bit unchanged".
- **Fuzzing**: add one voxel fuzzer mirroring the existing cell-properties fuzzer, so that arbitrary 64-bit input to the inspection functions never crashes.

## Out of Scope

- Language bindings (Python, JS, Java, …) and database extensions (e.g. Postgres). These are follow-on work in their own repos.
- 3D polygon fill (polygon × altitude range to voxels).
- Voxels at resolutions 14–15, and altitude ranges that follow terrain.
- Claiming bit 63 or unused mode values for extra layer bits (ADR-0004 names bit 63 as the switch to revisit).
- Exact ellipsoidal volumes, and voxel boundary geometry (3D polygons or meshes).
- 3D grid distance, paths, rings, local IJK coordinates, and voxel edges or vertices.
- Bundled geoid or terrain data in the core.
- Any change to an existing H3 function, index or constant.

## Further Notes

- All `K_r`, `Δu_0` and range constants are fixed by this spec; changing any of them changes the meaning of every stored voxel.
- The CLI and build-list edits are the only places where upstream files are touched. Keep them in single appended blocks so that upstream merges stay trivial.
- Reference points for the documentation:
  - layer thickness near the ground is about 119 m at res 10, 40 m at res 11, 20 m at res 12 and 6.6 m at res 13
  - resolution 13's range doesn't reach drones over high plateaus such as the Altiplano or Tibet (ADR-0004)

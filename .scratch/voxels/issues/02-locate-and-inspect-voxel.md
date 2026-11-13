# 02: Tracer bullet: locate and inspect a voxel (ellipsoidal altitude, res 0–10)

**What to build:** A user can turn a WGS84 lat/lng and ellipsoidal altitude into a voxel at resolutions 0–10, and inspect it: check validity, read its resolution, footprint and layer number. This works through the C API and the `h3` CLI. This slice puts in place everything later tickets build on: the separate voxel public header, voxel error codes, the fixed layer constants and split schedule, and the packed 64-bit layout (see the spec's "Geometry and layers", "Altitude ranges" and "Index layout" sections; ADR-0002 to ADR-0006).

**Blocked by:** 01 (CLI can take subcommands from a separate file)

**Status:** ready-for-agent

- [ ] A separate voxel public header is installed next to the H3 API header; the upstream API header is unchanged
- [ ] New error codes for voxel invalid, altitude domain and lookup failed sit in a block far above upstream's codes; a voxel-aware error description function covers them and falls back to upstream descriptions for all other codes, and the CLI reports voxel errors with it
- [ ] `latLngAltToVoxel` with ellipsoidal altitude returns a voxel at res 0–10 for any altitude in the full altitude range, and the altitude-domain error outside it
- [ ] The voxel's footprint equals `latLngToCell` for the same lat/lng and resolution, pentagon footprints included
- [ ] The layer number is signed and counted from altitude zero; a point exactly on a layer boundary belongs to the layer above it
- [ ] `isValidVoxel` accepts produced voxels and rejects: bit 63 set, a mode other than 15, resolution above 13, invalid footprint digits, and out-of-range layers; `isValidCell` rejects every voxel
- [ ] `getVoxelResolution`, `voxelToFootprint` and `getLayerNumber` work and return the voxel-invalid error on bad input
- [ ] Voxels round-trip through the existing `h3ToString` / `stringToH3`
- [ ] Ordering test: after sorting voxels as unsigned integers, each column is contiguous and ordered by layer
- [ ] CLI subcommands for each new function, with one smoke test each
- [ ] The whole upstream test suite passes unchanged

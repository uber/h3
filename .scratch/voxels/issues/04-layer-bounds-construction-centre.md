# 04: Layer bounds, construction and centre

**What to build:** A user can reason about layers without a lat/lng: find the layer number for an altitude, build a voxel from a cell and a layer number, read a voxel's floor and ceiling altitudes, and get its centre (the footprint's centre at the altitude midway between floor and ceiling).

**Blocked by:** 02 (Tracer bullet: locate and inspect a voxel)

**Status:** ready-for-agent

- [ ] `altitudeToLayerNumber` returns the signed layer number for an altitude and resolution, or the altitude-domain error
- [ ] `cellAndLayerToVoxel` builds a voxel from a valid cell and in-range layer number; invalid cells and out-of-range layers return errors
- [ ] `voxelToAltitudeBounds` returns floor and ceiling in metres; the ceiling of layer n equals the floor of layer n+1 exactly
- [ ] Altitude zero is the floor of layer 0 at every resolution
- [ ] `voxelToCenter` returns the footprint's centre lat/lng and the altitude midway between floor and ceiling; locating that point returns the same voxel
- [ ] `cellAndLayerToVoxel` agrees with `latLngAltToVoxel` for the same footprint and altitude
- [ ] CLI subcommands for each new function, with one smoke test each

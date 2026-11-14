# 03: Narrow altitude ranges (res 11–13) and getAltitudeRange

**What to build:** A user can locate voxels at resolutions 11–13 within each one's narrow altitude range (sharing a floor about −597 m; reaching about +1,440 km, +81 km and +2.8 km), and query the altitude range of any resolution. Resolutions 14–15 have no voxels. The valid layer numbers per resolution are given in the spec's "Altitude ranges" table (ADR-0004).

**Blocked by:** 02 (Tracer bullet: locate and inspect a voxel)

**Status:** ready-for-agent

- [ ] `latLngAltToVoxel` returns voxels at res 11–13 within their ranges, and the altitude-domain error above or below them
- [ ] Res 14 and 15 return the altitude-domain error for every altitude
- [ ] `isValidVoxel` uses each resolution's own range for its layer check
- [ ] `getAltitudeRange` returns floor and ceiling in metres for res 0–13, and an error for 14–15 and out-of-range resolutions
- [ ] Each narrow range's floor and ceiling are boundaries of the next coarser resolution's layers (checked through the public API)
- [ ] A ship at sea level (an ellipsoidal altitude of about −106 m, the geoid low) gets a voxel at every resolution 0–13
- [ ] CLI subcommand for `getAltitudeRange`, with a smoke test

## Comments

From ticket 02: the CLI help strings for `latLngAltToVoxel` ("Resolution, 0-10 inclusive.") and `getVoxelResolution` ("(0 - 10)") need updating to 0-13 once narrow ranges land. `layerRange` (internal) is the single place that decides which resolutions have voxels and their layer numbers.

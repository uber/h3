# 10: Hardening and documentation

**What to build:** Voxel inspection is robust against arbitrary input, and users can learn the voxel API from the docs.

**Blocked by:** 05 (Volume), 06 (Altitude types), 08 (Compact and uncompact), 09 (Voxel disk and face neighbours)

**Status:** ready-for-agent

- [ ] A voxel fuzzer, mirroring the existing cell-properties fuzzer, feeds arbitrary 64-bit values to every voxel inspection function without crashes or sanitizer findings
- [ ] Documentation pages cover the voxel concepts (using the glossary's terms), every voxel function, and the altitude ranges per resolution
- [ ] Docs state the boundary-precision caveat (points within about 1e-9 relative of a layer boundary may classify differently across platforms)
- [ ] Docs warn that upstream inspection functions (`getResolution`, `getBaseCellNumber`, …) are not voxel-aware
- [ ] Docs give reference layer thicknesses near the ground (about 119 m at res 10, 40 m at res 11, 20 m at res 12, 6.6 m at res 13) and note that res 13 does not reach drones over high plateaus (ADR-0004)
- [ ] The whole upstream test suite and all voxel tests pass

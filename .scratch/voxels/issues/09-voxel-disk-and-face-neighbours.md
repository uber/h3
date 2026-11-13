# 09: Voxel disk and face neighbours

**What to build:** A user can find everything near a voxel. The voxel disk of radius k is every voxel whose footprint is in the footprint's grid disk of radius k and whose layer is within k. Face neighbours are the voxels sharing a wall, floor or ceiling. Both are truncated at the ends of the altitude range and work around pentagon footprints.

**Blocked by:** 03 (Narrow altitude ranges)

**Status:** ready-for-agent

- [ ] `maxVoxelDiskSize(k)` equals `maxGridDiskSize(k)` × (2k + 1)
- [ ] `voxelDisk` returns exactly the footprints from `gridDisk` combined with layers n−k … n+k; unused output slots are zero, as with `gridDisk`
- [ ] An interior voxel's disk fills `maxVoxelDiskSize(k)` exactly (for a hexagon footprint away from pentagons)
- [ ] Disks are truncated, not errors, at the floor and ceiling of the resolution's altitude range, including the narrow ranges
- [ ] Disks around pentagon footprints work
- [ ] `voxelFaceNeighbors` returns 8 neighbours for an interior hexagon footprint, 7 for a pentagon, and fewer at the ends of the range
- [ ] CLI subcommands for each function, with one smoke test each

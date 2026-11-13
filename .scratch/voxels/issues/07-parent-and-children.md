# 07: Parent and children

**What to build:** A user can move up and down the voxel hierarchy. A parent's footprint is the footprint's parent cell, and its layer is the one containing the child's layer. A voxel's children are the footprint's children combined with the child layers that exactly subdivide its layer. Children outside the finer resolution's altitude range come back as an empty set, and every voxel has either all of its children or none.

**Blocked by:** 03 (Narrow altitude ranges)

**Status:** ready-for-agent

- [ ] `voxelToParent` works for any coarser resolution; the parent of each child is the original voxel
- [ ] Negative layer numbers map to the correct parent layer (below-zero voxels stay below zero)
- [ ] `voxelToChildrenSize` and `voxelToChildren` work for any finer resolution up to 13; the size matches the number of children produced
- [ ] The children's layers exactly tile the parent's layer: no overlaps or gaps in altitude
- [ ] Asking for children outside the child resolution's altitude range returns size 0 and an empty set, not an error
- [ ] A res-10 voxel just inside and just outside the res-11 range has all of its children or none
- [ ] Child resolution above 13, or coarser than the voxel, returns the resolution-domain error
- [ ] CLI subcommands for each function, with one smoke test each

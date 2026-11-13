# 05: Volume

**What to build:** A user can get a voxel's volume in cubic kilometres and cubic metres, measured on H3's sphere so it is consistent with H3's cell areas. At altitude zero a column's cross-section area equals the cell's area.

**Blocked by:** 04 (Layer bounds, construction and centre)

**Status:** ready-for-agent

- [ ] `voxelVolumeKm3` and `voxelVolumeM3` return positive volumes for valid voxels and the voxel-invalid error otherwise
- [ ] Volume equals cell area × the radial growth from floor to ceiling on H3's sphere, as defined in the spec
- [ ] A voxel's volume is larger than the volume of the voxel directly below it in the same column (for layers of the same resolution, near altitude zero, where thickness grows with altitude)
- [ ] The units are consistent: the cubic-metre value is 1e9 × the cubic-kilometre value
- [ ] CLI subcommands for both functions, with one smoke test each
- [ ] When ticket 07 is done: a parent's volume equals the sum of its children's within a tolerance (add the test then if 07 lands later)

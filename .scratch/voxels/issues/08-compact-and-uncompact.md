# 08: Compact and uncompact

**What to build:** A user can compact a set of voxels into the coarsest equivalent set and uncompact it back to a chosen resolution, with the same semantics and errors as H3's cell compaction.

**Blocked by:** 07 (Parent and children)

**Status:** ready-for-agent

- [ ] `compactVoxels` replaces each complete set of siblings with their parent, repeatedly, and leaves incomplete sets alone
- [ ] `uncompactVoxelsSize` and `uncompactVoxels` expand to a target resolution; the size matches the output
- [ ] Compact then uncompact round-trips to the original set
- [ ] Duplicate input returns the same duplicate-input error as `compactCells`
- [ ] Invalid voxels and mixed-up resolutions are handled as the cell equivalents do
- [ ] Sets near the edges of the narrow ranges compact and uncompact correctly
- [ ] CLI subcommands for each function, with one smoke test each

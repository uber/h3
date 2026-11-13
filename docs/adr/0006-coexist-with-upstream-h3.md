# Voxel identifiers come from the far end of upstream H3's number spaces

This fork keeps merging upstream H3 releases, and upstream assigns modes, error codes and similar values one after another (mode 5 would be next). To avoid collisions, voxel additions take values from the far end of each space (voxel mode is 15; new error codes start high), and all voxel code lives in new files (`voxel.c`, `voxel.h`, and similar) so upstream merges never touch the same lines. Bit 63 and unused mode values are not claimed for extra layer bits for the same reason (see ADR-0004).

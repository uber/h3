# Voxel identifiers come from the far end of upstream H3's number spaces

This fork keeps merging upstream H3 releases, and upstream assigns modes, error codes and similar values one after another (mode 5 would be next). To avoid collisions, voxel additions take values from the far end of each space (voxel mode is 15; new error codes start high), and all voxel code lives in new files (`voxel.c`, `voxel.h`, and similar) so upstream merges never touch the same lines. Bit 63 and unused mode values are not claimed for extra layer bits for the same reason (see ADR-0004).

The exceptions are list entries in the build files (voxel sources, tests and the voxel header's install rule) and three one-line hooks in the upstream CLI source, each placed away from where upstream adds subcommands: the include of the voxel subcommand file, the voxel entries at the end of the subcommand index, and the error-description call used when the CLI reports an error (so voxel error codes are described).

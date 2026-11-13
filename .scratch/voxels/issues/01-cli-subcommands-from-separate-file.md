# 01: Prefactor: CLI can take subcommands from a separate file

**What to build:** A minimal hook in the existing `h3` CLI so that a separate source file can contribute subcommand definitions and their entries in the subcommand index. Voxel subcommands will live in that file, so upstream H3 merges never touch the same lines (ADR-0006). No CLI behaviour changes in this ticket.

**Blocked by:** None (can start immediately)

**Status:** done

- [x] The upstream CLI source gains at most a couple of include/hook lines; no existing subcommand is moved or edited
- [x] An (initially empty) voxel subcommand file is compiled into the CLI and can add subcommands to the index
- [x] `h3 --help` output is unchanged while the voxel file is empty
- [x] The full existing CLI test suite and upstream test suite pass unchanged

## Comments

Done on branch `voxels/01-cli-subcommands-from-separate-file`. Two hook lines in the upstream CLI: the voxel subcommand file is included after the CLI helpers and before the first upstream subcommand, and its index entries expand just before the end of the subcommand index (listed last in `h3 --help`). The include originally sat right after the last upstream subcommand. A merge simulation showed that spot conflicts when upstream appends a subcommand, so it was moved. Simulated upstream additions (appended at the end, inside a category, and as a new first subcommand) now all merge cleanly. Verified: `h3 --help` byte-identical to master's, 332/332 tests pass, and a temporary probe subcommand defined only in the voxel file ran and was listed.

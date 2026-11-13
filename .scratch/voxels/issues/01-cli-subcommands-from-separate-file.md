# 01: Prefactor: CLI can take subcommands from a separate file

**What to build:** A minimal hook in the existing `h3` CLI so that a separate source file can contribute subcommand definitions and their entries in the subcommand index. Voxel subcommands will live in that file, so upstream H3 merges never touch the same lines (ADR-0006). No CLI behaviour changes in this ticket.

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

- [ ] The upstream CLI source gains at most a couple of include/hook lines; no existing subcommand is moved or edited
- [ ] An (initially empty) voxel subcommand file is compiled into the CLI and can add subcommands to the index
- [ ] `h3 --help` output is unchanged while the voxel file is empty
- [ ] The full existing CLI test suite and upstream test suite pass unchanged

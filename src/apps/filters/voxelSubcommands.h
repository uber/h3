/*
 * Copyright 2026 J Reynolds
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file voxelSubcommands.h
 * @brief Voxel subcommands for the `h3` CLI.
 *
 * This is not a standalone header and must be included exactly once: `h3.c`
 * includes it after its CLI helpers (`SUBCOMMAND`, `PARSE_SUBCOMMAND`,
 * `helpArg`, `DEFINE_FORMAT_ARG`, ...) and before its first subcommand.
 * Keeping voxel subcommands here means merging upstream changes to `h3.c`
 * never touches the same lines (docs/adr/0006-coexist-with-upstream-h3.md).
 * The two hooks in `h3.c` sit away from where upstream adds subcommands, so
 * an upstream addition merges without conflicts.
 *
 * To add a voxel subcommand:
 *   1. define it below with `SUBCOMMAND(name, help) { ... }`, and
 *   2. append `SUBCOMMAND_INDEX(name)` to `VOXEL_SUBCOMMANDS_INDEX`, ending
 *      each line but the last with a backslash. Don't put `//` comments on
 *      those lines: they would swallow the backslash and break the macro.
 *
 * Helpers defined later in `h3.c` (e.g. `readCellsFromFile`) are not in
 * scope here.
 */

#ifndef VOXEL_SUBCOMMANDS_H
#define VOXEL_SUBCOMMANDS_H

/**
 * Subcommand index entries for voxel subcommands. Expanded inside `h3.c`'s
 * subcommand index after all upstream entries, so voxel subcommands are
 * listed last in `h3 --help`.
 */
#define VOXEL_SUBCOMMANDS_INDEX

#endif

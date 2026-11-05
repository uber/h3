#!/usr/bin/env bash
# Produces the digest rows of one resolution with one mkConformance process
# per base cell, running JOBS processes at a time (default: every online
# CPU), replaces that resolution's rows in the suite's digest files, and
# rewrites the manifest. mkConformance writes resolutions 0 through 6 in one
# process; this is how resolution 7, about an hour single-threaded, is
# produced. See dev-docs/conformance.md, "Running and regenerating".
#
#   conformance_digests.sh <path-to-mkConformance> <res> [suite-dir]
set -eu

generator=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
res=$2
root=$(cd "$(dirname "$0")/.." && pwd)
out=${3:-$root/tests/inputfiles}
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT
jobs=${JOBS:-$(getconf _NPROCESSORS_ONLN)}

mkdir -p "$build/rows"

# One process per base cell; each writes its row to its own file so the rows
# can be concatenated in base-cell order afterwards.
seq 0 121 | xargs -P "$jobs" -I'{}' sh -c \
    '"$1" --digests "$2" "$3" "$3" > "$4/$3.jsonl"' sh "$generator" "$res" '{}' "$build/rows"

for baseCell in $(seq 0 121); do
    cat "$build/rows/$baseCell.jsonl"
done > "$build/base.jsonl"
"$generator" --resolution-row "$res" < "$build/base.jsonl" > "$build/res.jsonl"

# Replace any existing rows of this resolution, keeping the others in order.
for file in baseCells resolutions; do
    grep -v "^{\"res\":$res," "$out/digests/$file.jsonl" > "$build/$file.keep" || true
done
cat "$build/baseCells.keep" "$build/base.jsonl" > "$out/digests/baseCells.jsonl"
cat "$build/resolutions.keep" "$build/res.jsonl" > "$out/digests/resolutions.jsonl"

"$generator" -o "$out" --manifest

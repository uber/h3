#!/usr/bin/env bash
# Regenerates the conformance suite with the mkConformance binary given as
# the first argument and compares the discrete record files against the
# committed ones under tests/inputfiles. Files under floats/ are judged by
# the runners under tolerance and are not byte-compared: a compiler or libm
# change may legitimately move their last digit. See dev-docs/conformance.md.
#
#   conformance_check.sh <path-to-mkConformance> [group-or-file]...
#
# Without groups, every group present in tests/inputfiles except digests is
# regenerated; digests takes minutes and is checked on a schedule instead.
set -eu

generator=$1
shift
root=$(cd "$(dirname "$0")/.." && pwd)
committed="$root/tests/inputfiles"
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT

if [ $# -eq 0 ]; then
    for dir in "$committed"/*/; do
        group=$(basename "$dir")
        if [ "$group" != digests ]; then
            set -- "$@" "$group"
        fi
    done
fi

args=()
for group in "$@"; do
    mkdir -p "$out/${group%%/*}"
    args+=(-g "$group")
done

"$generator" -o "$out" "${args[@]}"

status=0
for file in $(cd "$out" && find . -name '*.jsonl' | sort); do
    file=${file#./}
    case "$file" in
        floats/*) continue ;;
    esac
    if ! cmp -s "$out/$file" "$committed/$file"; then
        echo "conformance_check: $file differs from the committed file" >&2
        status=1
    fi
done

if [ "$status" -eq 0 ]; then
    echo "conformance_check: regenerated files match tests/inputfiles"
fi
exit "$status"

init: purge
    mkdir build

build:
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

purge:
    rm -rf build
    rm -rf *.trace
    rm -rf .ipynb_checkpoints
    rm -rf .cache
    rm -rf .claude

test-fast: build
    cd build; make test-fast

test-slow: build
    cd build; make test

test-one TEST: build
    ./build/bin/{{TEST}}

test:
    just test-fast

bench: build
    ./build/bin/benchmarkCellsToPolyAlgos

coverage: purge
    mkdir build
    cd build; cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..; make; make coverage
    open build/coverage/index.html

# Show uncovered lines/branches for a source file from the lcov .info data.
# Run `just coverage` first. Example: just coverage-gaps linkedGeo.c
coverage-gaps FILE:
    #!/usr/bin/env bash
    set -e
    info="build/coverage.cleaned.info"
    if [ ! -f "$info" ]; then
        echo "No coverage data found — run 'just coverage' first."
        exit 1
    fi
    # Extract the section for this file
    section=$(sed -n "/SF:.*\/{{FILE}}$/,/end_of_record/p" "$info")
    if [ -z "$section" ]; then
        echo "File {{FILE}} not found in coverage data."
        echo "Available files:"
        grep '^SF:' "$info" | sed 's|.*src/h3lib/||'
        exit 1
    fi
    echo "=== Summary ==="
    lf=$(echo "$section" | grep '^LF:' | cut -d: -f2)
    lh=$(echo "$section" | grep '^LH:' | cut -d: -f2)
    brf=$(echo "$section" | grep '^BRF:' | cut -d: -f2)
    brh=$(echo "$section" | grep '^BRH:' | cut -d: -f2)
    echo "Lines: $lh/$lf  Branches: $brh/$brf"
    echo ""
    echo "=== Uncovered lines (DA:line,0) ==="
    echo "$section" | grep '^DA:' | awk -F'[,:]' '$3 == 0 {print "  line " $2}' || echo "  (none)"
    echo ""
    echo "=== Untaken branches (BRDA:line,block,branch,0) ==="
    echo "$section" | grep '^BRDA:' | awk -F'[,:]' '$5 == 0 {print "  line " $2 " branch " $4}' || echo "  (none)"

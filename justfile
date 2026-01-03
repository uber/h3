# TODO: Remove before landing

init: purge
    mkdir build

build:
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

profile: init
    cd build; cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS='-fno-omit-frame-pointer' ..; make
    # cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make
    xcrun xctrace record \
        --template 'Time Profiler' \
        --output h3-prof.trace \
        --launch -- ./build/bin/benchmarkCellsToPolyAlgos
    open h3-prof.trace

purge:
    rm -rf build
    rm -rf *.trace
    rm -rf .ipynb_checkpoints

test: build
    # ./build/bin/testH3CellAreaExhaustive
    # ./build/bin/testEdgeCellsToPoly
    # ./build/bin/testDirectedEdge
    # ./build/bin/testArea
    # just test-slow
    just test-fast
    # ./build/bin/testCellsToMultiPoly

time:
    time ./build/bin/testArea

test-fast: build
    cd build; make test-fast

test-slow: build
    cd build; make test

test-one TEST: build
    ./build/bin/{{TEST}}

bench: build
    ./build/bin/benchmarkCellsToPolyAlgos

coverage: purge
    mkdir build
    cd build; cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..; make; make coverage
    open build/coverage/index.html

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

# Run a single test binary. Dots (progress) go to /dev/null; failures print to stderr.
test-one TEST: build
    ./build/bin/{{TEST}} > /dev/null

test:
    just test-fast

bench: build
    # ./build/bin/benchmarkCellsToPolyAlgos
    ./build/bin/benchmarkGosperIter

coverage: purge
    mkdir build
    cd build; cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..; make; make coverage
    open build/coverage/index.html

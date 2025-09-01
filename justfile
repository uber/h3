init: purge
    mkdir build

build:
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

purge:
    rm -rf build

test: build
    # ./build/bin/testCreateCell
    # just test-fast
    ./build/bin/h3 getIndexDigit -r 15 -c 8f754e64992d6d6  # should work
    ./build/bin/h3 getIndexDigit -r 15 -c 5  # should fail

ctest: build
    cd build; ctest

test-fast: build
    cd build; make test-fast

test-slow: build
    cd build; make test

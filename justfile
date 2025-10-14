# TODO: Remove before landing

init: purge
    mkdir build

build:
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

purge:
    rm -rf build

test: build
    ./build/bin/testConstructCell

test-fast: build
    cd build; make test-fast

test-slow: build
    cd build; make test

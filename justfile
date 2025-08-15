init: purge
    mkdir build

build:
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

purge:
    rm -rf build

test:
    # cd build; make test
    cd build; make test-fast

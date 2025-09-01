default:
    @just --list

purge:
    rm -rf build

build:
    -mkdir build
    cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

# whatever test is currently in the hot loop; often changes
test:
    # ./build/bin/testCreateCell
    # just test-fast
    # just fail-example
    just ctest

# Pass/fail example of getIndexDigit
fail-example: build
    ./build/bin/h3 getIndexDigit -r 15 -c 8f754e64992d6d6  # should work
    ./build/bin/h3 getIndexDigit -r 15 -c 5  # should fail


# Runs all C and CLI tests
ctest: build
    cd build; ctest

# Runs fast subset of C tests
test-fast: build
    cd build; make test-fast

# Runs all C tests (slow)
test-slow: build
    cd build; make test

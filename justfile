# NOTE: remove before landing PR

_default:
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
    # test_bad_getIndexDigit
    # just ctest
    # just ctest-demo1
    just regex-demo2



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

list-tests:
    cd build; ctest -N

regex-demo1:
    # ctest takes a -R option, which is a regex. can use to run a single test
    cd build; ctest -R test_getIndexDigit
    cd build; ctest -R test_bad_getIndexDigit

regex-demo2:
    # can also use to run multiple tests that match the regex
    cd build; ctest -R getIndexDigit

ctest-demo1:
    # ctest takes a -V option for verbose output
    cd build; ctest -R getIndexDigit -V


# TODO: can i use ctest to run individual C tests (instead of CLI tests)? (show multiple ways to run the same test)
# TODO: can i use ctest to run benchmarks?

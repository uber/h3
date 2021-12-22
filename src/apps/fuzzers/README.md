# Fuzzer harnesses for H3

This directory contains helper programs for testing the H3 library using the
''[American fuzzy lop](https://lcamtuf.coredump.cx/afl/)'' fuzzer.

# Installation

```
apt install afl-clang
```

(There is also an afl-cov which looks interesting but isn't necessary.)

# libFuzzer Usage

[libFuzzer](https://www.llvm.org/docs/LibFuzzer.html) is one of the supported fuzzing drivers.

This is the fuzzer used in [oss-fuzz](https://github.com/google/oss-fuzz/tree/master/projects/h3).

# AFL Usage

[AFL++](https://github.com/AFLplusplus/AFLplusplus) is one of the supported fuzzing drivers.

You must compile with the instrumented compiler:

```
CXX=afl-clang++ CC=afl-clang cmake .
make fuzzers
```

Generate a blank (zeroed) test case file. This will not be very interesting test case but is usedful
for having files of the right size.

```
fuzzerLatLngToCell --generate bytes24
```

An individual fuzzer run is invoked as follows. The argument is a file containing the number of bytes needed.

```
fuzzerLatLngToCell bytes24
```

To begin running the fuzzer, run the following. The testcase directory (`testcase_dir`) should contain a file
with at least the right number of bytes that the fuzzer will read (this can be generated using the `--generate`
option above.)

```
afl-fuzz -i testcase_dir -o findings_dir -- fuzzerLatLngToCell @@
```

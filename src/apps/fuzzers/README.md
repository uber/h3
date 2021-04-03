# Fuzzer harnesses for H3

This directory contains helper programs for testing the H3 library using the
''[American fuzzy lop](https://lcamtuf.coredump.cx/afl/)'' fuzzer.

# Installation

```
apt install afl-clang
```

(There is also an afl-cov which looks interesting but isn't necessary.)

# Usage

You must compile with the instrumented compiler:

```
CXX=afl-clang++ CC=afl-clang cmake .
make fuzzers
```

An individual fuzzer run is invoked as follows. The argument is a file containing the number of bytes needed.

```
fuzzerGeoToH3 bytes24
```

To begin running the fuzzer, run the following. The testcase directory (`testcase_dir`) should contain a file
with at least the right number of bytes that the fuzzer will read (such as 16 for fuzzerKRing.)

```
afl-fuzz -i testcase_dir -o findings_dir -- fuzzerGeoToH3 @@
```

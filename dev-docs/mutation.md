Version number must match your LLVM version:

```
brew trust mull-project/mull
brew install mull-project/mull/mull@19
```

Then, compile with that version of LLVM, enabling the mutation build support here:

```
mkdir build
cd build
CC=/opt/homebrew/Cellar/llvm@19/19.1.7/bin/clang-19 cmake .. -DENABLE_MUTATION=ON -DMULL_ROOT=/opt/homebrew/Cellar/mull@19/0.34.0/ -DMULL_VERSION=19 -DCMAKE_BUILD_TYPE=Debug -DBUILD_GENERATORS=OFF -DBUILD_FUZZERS=OFF -DBUILD_BENCHMARKS=OFF -DENABLE_DOCS=OFF
```

Then, run the mutation test suite:

```
make mutation
```

This should produce a final report with a mutation score. Note that not all tests are run through
the mutation testing pipeline, specifically tests that use data files.

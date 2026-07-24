Version number must match your LLVM version:

```
brew trust mull-project/mull
brew install mull-project/mull/mull@19
```

Then, compile with that version of LLVM, enabling the mutation build support here:

```
mkdir build
cd build
CC=/opt/homebrew/Cellar/llvm@19/19.1.7/bin/clang-19 cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_MUTATION=ON
```

Then, run the mutation test suite:

```
make mutation
```

This should produce a final report with a mutation score. Note that not all tests are run through
the mutation testing pipeline, specifically tests that use data files.

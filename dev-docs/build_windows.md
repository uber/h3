# Building H3 on Windows

H3 has been successfully built on Windows using Visual Studio 2017.

You can use the CMake GUI to configure the Visual Studio project files needed to build H3. Alternately, you can do the same from the command line:

```
mkdir build
cd build
cmake ..
```

You can now open `h3.sln` and build the `ALL_BUILD` project to build the H3 library, filter applications, and tests. Tests can be run by building the `RUN_TESTS` project. From the command line:

```
msbuild ALL_BUILD.vcxproj
msbuild RUN_TESTS.vcxproj
```

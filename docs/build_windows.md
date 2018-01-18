# Building H3 on Windows

H3 has been successfully built on Windows using Visual Studio 2017.

You can use the CMake GUI to configure the Visual Studio project files needed to build H3. You will need to set `BUILD_STATIC` to `ON`. Alternately, you can do the same from the command line:

```
mkdir build
cd build
cmake -DBUILD_STATIC=ON ..
```

You can now open `h3.sln` and build the `ALL_BUILD` project to build the H3 static library, filter applications, and tests. Tests can be run by building the `RUN_TESTS` project. From the command line:

```
msbuild ALL_BUILD.vcxproj
msbuild RUN_TESTS.vcxproj
```

## Building a DLL

To build a DLL (`h3.1.dll`) you will need to rerun CMake with the `BUILD_STATIC` option set to `OFF`, and then build the `h3.1` project. From the command line:

```
cmake -DBUILD_STATIC=OFF ..
msbuild h3.1.vcxproj
```

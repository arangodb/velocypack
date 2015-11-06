Compling and Installing the VPack library
=========================================

Prerequisites
-------------

The VPack library is implemented in C++. To build it, a C++11-enabled compiler 
is required. Recent versions of g++ and clang are known to work.

VPack uses [CMake](https://cmake.org/download/) for building. Therefore, a recent 
version of CMake is required, too.


Building the VPack library
--------------------------

*Note: the following build instructions are for Linux and MacOS.*

Building the VPack library is straightforward with `cmake`. Simply execute the 
following commands to create an out-of-source build:

```bash
mkdir -p build
(cd build && cmake .. && make)
```

This will build a static library `libvelocypack.a` in the `build` directory 
in *Release* mode.

By default a few example programs and tools will also be built. These can
be found in the `build` directory when the build is complete.

To install the library and tools, run the following command:

```bash
(cd build && cmake .. && make install)
```

Running the tests
-----------------

Building VPack's own test suite requires the [googletest framework](https://github.com/google/googletest)
to be built. To build the tests, run cmake with the option `-DBuildTests=ON`:

```bash
mkdir -p build
(cd build && cmake -DBuildTests=ON .. && make)
```

Afterwards, you can run the tests via:

```bash
(cd build/tests && $(find . -maxdepth 1 -type f -name "tests*" | xargs))
```

Build Options
-------------

The following options can be set when building VPack:

* `-DCMAKE_BUILD_TYPE=Release`: builds the VPack library in release mode. This
  does not build debug symbols and turns on all optimizations. Use this mode for 
  production.
* `-DCMAKE_BUILD_TYPE=Debug`: builds the VPack library in debug mode. This
  adds debug symbols and turns off optimizations. Use this mode for development,
  but not for production or performance testing.
* `-DBuildBench`: controls whether the benchmark suite should be built. The 
  default is `OFF`, meaning the suite will not be built. Set the option to `ON` to
  build it. Building the benchmark suite requires the subdirectory *rapidjson* to 
  be present (see below). 
* `-DBuildExamples`: controls whether VPack's examples should be built. The 
  examples are not needed when VPack is used as a library only.
* `-DBuildTests`: controls whether VPack's own test suite should be built. The
  default is `OFF`. Set the option to `ON` for building the tests. This requires 
  the subdirectory *googletest* to be present (see below). 
* `-DEnableSSE`: controls whether SSE4.2 optimizations are compiled into the
  library. The default is either `ON` or `OFF`, depending on the detected SSE4.2
  support of the host platform. Note that this option should be turned off when
  running VPack under Valgrind, as Valgrind does not seem to support all SSE4
  operations used in VPack. 


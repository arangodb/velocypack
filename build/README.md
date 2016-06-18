This is the build output directory.

It will be populated when building VelocyPack like this:
```bash
(cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBuildExamples=ON -DBuildTests=ON -DBuildTools=ON .. && make)
```

On Debian Jessie (8) you have to use the g++ flags '-std=c++11 -std=gnu++11' to compile VelocyPack:

```bash
(cd build && cmake-DCMAKE_CXX_FLAGS="-std=c++11 -std=gnu++11" -DCMAKE_BUILD_TYPE=Release -DBuildExamples=ON -DBuildTests=ON -DBuildTools=ON .. && make)
```

You can remove the contents in this directory to build
everything from scratch.

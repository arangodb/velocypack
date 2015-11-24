This is the build output directory.

It will be populated when building VelocyPack like this:
```bash
(cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBuildExamples=ON -DBuildTests=ON -DBuildTools=ON .. && make)
```

You can remove the contents in this directory to build
everything from scratch.

ulimit -c unlimited -S # enable core files
mkdir -p build
(cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCoverage=OFF -DBuildTests=ON -DBuildLargeTests=OFF -DBuildExamples=ON -DBuildTools=ON -DEnableSSE=OFF -DCMAKE_CXX_COMPILER=$CXX ..)
(cd build && make)
(cd build/tests && ctest -V)

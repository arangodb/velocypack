mkdir -p build
(cd build && cmake -DCMAKE_BUILD_TYPE=Release -DHashType=xxhash -DCoverage=OFF -DBuildTests=ON -DBuildLargeTests=OFF -DBuildVelocyPackExamples=ON -DBuildTools=ON -DEnableSSE=OFF -DCMAKE_CXX_COMPILER=$CXX ..)
(cd build && make)
(cd build/tests && ctest -V)
sleep 2
echo Done.

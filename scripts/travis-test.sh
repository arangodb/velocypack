#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}


CXX_STANDARD=${CXX_STANDARD:-14}
COVERAGE=${COVERAGE:-OFF}
BUILD_TYPE=${BUILD_TYPE:-Release}

if [[ $COVERAGE == "ON" ]]; then
    BUILD_TYPE=Debug
fi

threads=2
if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    threads=$(nproc)
fi

echo "Building with C++ Standard $CXX_STANDARD"
echo "coverage: $COVERAGE"

mkdir -p build && cd build || ferr "could not create build dir"

cmake -DCMAKE_BUILD_TYPE=Release -DHashType=xxhash -DCoverage=${COVERAGE} \
      -DBuildTests=ON -DBuildLargeTests=OFF -DBuildVelocyPackExamples=ON \
      -DBuildTools=ON -DEnableSSE=OFF \
      -DCMAKE_CXX_COMPILER=$CXX \
      -DCMAKE_CXX_STANDARD=${CXX_STANDARD} \
      .. || ferr "failed to configure"

make -j $threads || ferr "failed to build"
ctest -V || ferr "failed to run tests"
sleep 2
echo Done.

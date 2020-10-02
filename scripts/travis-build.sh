#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}


CXX_STANDARD=${CXX_STANDARD:-14}
echo "requesting c++ standard ${CXX_STANDARD}"
BUILD_TYPE=${BUILD_TYPE:-Release}

threads=2
if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    threads=$(nproc)
fi

echo "Building with C++ Standard $CXX_STANDARD"
echo "TaoJson support: $TAO"

mkdir -p build && cd build || ferr "could not create build dir"

set -x
cmake -DCMAKE_BUILD_TYPE=Release -DHashType=xxhash \
      -DBuildTests=ON -DBuildLargeTests=OFF -DBuildVelocyPackExamples=ON \
      -DBuildTools=ON -DEnableSSE=OFF -DTaoJson="$TAO" \
      -DCMAKE_CXX_STANDARD=${CXX_STANDARD} \
      .. || ferr "failed to configure"
set +x

make -j $threads || ferr "failed to build"
echo Done.

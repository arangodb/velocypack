#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}

(cd build/tests && ctest -V) || ferr "failed to run tests"

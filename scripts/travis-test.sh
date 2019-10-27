#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}

COVERAGE=${COVERAGE:-OFF}
if [[ $COVERAGE != OFF ]] ; then
    (cd build/tests && ctest -V) || ferr "failed to run tests"
fi

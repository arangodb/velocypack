#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}

(cd build/tests && ctest --output-on-failure) || ferr "failed to run tests"
build/tools/fuzzer --vpack --iterations 10000 --threads 1 || ferr "failed to run sunshine fuzzer for vpack"
build/tools/fuzzer --vpack --iterations 10000 --threads 1 --evil || ferr "failed to run darkness fuzzer for vpack"
build/tools/fuzzer --json --iterations 1000 --threads 1 || ferr "failed to run sunshine fuzzer for json"

#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}

if ${COVERAGE:-false}; then
  gem install coveralls-lcov || ferr "failed to install gem"
  cd build

  # clear counters
  lcov --capture --initial --directory . --output-file base_coverage.info || ferr "failed lcov"
  lcov --directory . --zerocounters || ferr "failed lcov"

  # run tests
  (cd tests && ctest -V) || ferr "failed to run tests"

  # collect coverage info
  lcov --directory . --capture --output-file test_coverage.info || ferr "failed lcov"
  lcov --add-tracefile base_coverage.info --add-tracefile test_coverage.info --output-file coverage.info || ferr "failed lcov"
  lcov --remove coverage.info 'tests/*' '/usr/*' 'src/*hash*' 'src/powers.h' --output-file coverage.info || ferr "failed lcov"
  lcov --list coverage.info || ferr "failed lcov"

  # upload coverage info
  coveralls-lcov --repo-token ${COVERALLS_TOKEN} coverage.info || ferr "failed coveralls-lcov"
fi

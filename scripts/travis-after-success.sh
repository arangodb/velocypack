#!/bin/bash
set -u
ferr(){
    echo "$@"
    exit 1
}

if ${COVERAGE:-false}; then
  #prepare
  project_dir="$(readlink -f ..)"
  build_dir="$project_dir/build"

  echo "coverage build directory $build_dir"
  cd ${build_dir} || ferr "can not enter build dir"

  gem install coveralls-lcov || ferr "failed to install gem"

  # clear counters
  lcov --directory "$build_dir" --capture --initial --output-file base_coverage.info || ferr "failed lcov"
  lcov --directory "$build_dir" --zerocounters || ferr "failed lcov"

  # run tests
  (cd tests && ctest -V) || ferr "failed to run tests"

  # collect coverage info
  lcov --directory "$build_dir" --capture --output-file test_coverage.info || ferr "failed lcov"
  lcov --add-tracefile base_coverage.info --add-tracefile test_coverage.info --output-file coverage.info || ferr "failed lcov"
  lcov --remove coverage.info \
           "$project_dir"'tests/*' \
           "$project_dir"'/usr/*' \
           "$project_dir"'src/*hash*' \
           "$project_dir"'src/powers.h' \
            --output-file coverage.info || ferr "failed lcov"
  lcov --list coverage.info || ferr "failed lcov"

  # upload coverage info
  if ${COVERALLS_TOKEN:-false}; then
    coveralls-lcov --repo-token ${COVERALLS_TOKEN} coverage.info || ferr "failed to upload"
  else
    # should not be required on github
    coveralls-lcov coverage.info || ferr "failed to upload"
  fi
fi

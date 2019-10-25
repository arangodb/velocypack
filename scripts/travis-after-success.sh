if [ "$CXX" = "g++-7" ]; then
  gem install coveralls-lcov
  cd build

  # clear counters
  lcov --capture --initial --directory . --output-file base_coverage.info
  lcov --directory . --zerocounters
  # run tests
  ctest -V
  # collect coverage info
  lcov --directory . --capture --output-file test_coverage.info
  lcov --add-tracefile base_coverage.info --add-tracefile test_coverage.info --output-file coverage.info
  lcov --remove coverage.info 'tests/*' '/usr/*' 'src/*hash*' 'src/powers.h' --output-file coverage.info
  lcov --list coverage.info
  # upload coverage info
  coveralls-lcov --repo-token ${COVERALLS_TOKEN} coverage.info
fi

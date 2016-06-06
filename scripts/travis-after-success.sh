if [ "$CXX" = "/usr/bin/g++-4.9" ]; then 
  gem install coveralls-lcov
  # install lcov 1.11
  wget http://ftp.de.debian.org/debian/pool/main/l/lcov/lcov_1.11.orig.tar.gz
  tar xf lcov_1.11.orig.tar.gz
  make -C lcov-1.11/ install PREFIX=~/bin/lcov

  rm -Rf build/*
  # build again
  (cd build && cmake -DCoverage=ON -DBuildTests=ON -DBuildLargeTests=OFF -DBuildExamples=OFF -DBuildTools=OFF -DEnableSSE=OFF -DCMAKE_CXX_COMPILER=$CXX ..)
  (cd build && make)
  # clear counters
  ~/bin/lcov/usr/bin/lcov --directory build --zerocounters --gcov-tool=gcov-4.9
  # run tests
  (cd build/tests/ && ctest -V)
  # collect coverage info
  ~/bin/lcov/usr/bin/lcov --directory build --capture --output-file build/coverage.info --gcov-tool=gcov-4.9
  ~/bin/lcov/usr/bin/lcov --remove build/coverage.info 'tests/*' '/usr/*' 'src/*hash*' 'src/powers.h' --output-file build/coverage.info --gcov-tool=gcov-4.9
  ~/bin/lcov/usr/bin/lcov --list build/coverage.info --gcov-tool=gcov-4.9
  # upload coverage info
  coveralls-lcov --repo-token ${COVERALLS_TOKEN} build/coverage.info
fi

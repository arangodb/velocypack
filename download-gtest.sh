export COMPILER="`which g++`"
if test "x$CC" = "x"; then 
  export COMPILER="`which clang++`"
fi

rm -Rf googletest
git clone https://github.com/google/googletest.git
(cd googletest/googletest && ${COMPILER} -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc)
(cd googletest/googletest && ar -rv libgtest.a gtest-all.o)

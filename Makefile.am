ACLOCAL_AMFLAGS="-Im4"

lib_LIBRARIES = libjason.a

libjason_a_SOURCES = JasonAsm.cpp Jason.cpp JasonParser.cpp JasonBuilder.cpp
libjason_a_CXXFLAGS = -Wall -Wextra -std=c++11 -O3 -g -msse4.2 -march=native

noinst_PROGRAMS = bench test JasonAsm

bench_SOURCES = bench.cpp fpconv.cpp
bench_LDADD = libjason.a
bench_CXXFLAGS = -Wall -Wextra -std=c++11 -O3 -g -DRAPIDJSON_SSE42 -msse4.2 -march=native -Irapidjson/include

test_SOURCES = test.cpp fpconv.cpp
test_LDADD = libjason.a googletest/googletest/libgtest.a
test_LDFLAGS = -pthread
test_CXXFLAGS = -Wall -Wextra -std=c++11 -O3 -g -Igoogletest/googletest/include -march=native

JasonAsm_SOURCES = JasonAsm.cpp
JasonAsm_CXXFLAGS = -Wall -Wextra -std=c++11 -O3 -g -msse4.2 -march=native
JasonAsm_CPPFLAGS = -DCOMPILE_JASONASM_UNITTESTS

BUILT_SOURCES = googletest/googletest/include/gtest/gtest.h rapidjson/rapidjson.h

googletest/googletest/include/gtest/gtest.h:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

rapidjson/rapidjson.h:
	rm -rf rapidjson
	git clone https://github.com/miloyip/rapidjson

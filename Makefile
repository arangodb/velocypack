CC=g++
#CC=clang++
CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DJASON_DEBUG
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -DRAPIDJSON_SSE42 -march=native
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -march=native -msse4.2 -DNO_SSE42
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -march=native -msse4.2
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -msse4.2
#CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DNO_SSE42

all:	test bench JasonAsm

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o: Makefile powers.h fpconv.h fpconv.cpp
	$(CC) $(CFLAGS) fpconv.cpp -c -o fpconv.o

JasonBuilder.o: Makefile JasonBuilder.h JasonBuilder.cpp Jason.h
	$(CC) $(CFLAGS) JasonBuilder.cpp -c -o JasonBuilder.o

test:	Makefile test.cpp JasonDumper.h fpconv.o Jason.h JasonBuffer.h JasonBuilder.o JasonParser.h JasonSlice.h JasonType.h
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o JasonBuilder.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp Jason.h JasonBuilder.h JasonBuilder.o JasonParser.h JasonSlice.h JasonType.h
	$(CC) $(CFLAGS) bench.cpp JasonBuilder.o -o bench

JasonAsm: Makefile JasonAsm.h JasonAsm.cpp
	$(CC) $(CFLAGS) JasonAsm.cpp -DCOMPILE_JASONASM_UNITTESTS -o JasonAsm

clean:	
	rm -rf *.o test bench

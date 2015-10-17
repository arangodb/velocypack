CC=g++
#CC=clang++
#CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DJASON_DEBUG -DJASON_VALIDATEUTF8
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -DRAPIDJSON_SSE42 -march=native -msse4.2
CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -DRAPIDJSON_SSE42 -march=native
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -march=native -msse4.2 -DNO_SSE42
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -march=native -msse4.2
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -msse4.2
#CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DNO_SSE42
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -DJASON_VALIDATEUTF8 
#CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DJASON_DEBUG -DJASON_VALIDATEUTF8

all:	test bench JasonAsm

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o: Makefile powers.h fpconv.h fpconv.cpp
	$(CC) $(CFLAGS) fpconv.cpp -c -o fpconv.o

JasonAsm.o: Makefile JasonAsm.h
	$(CC) $(CFLAGS) JasonAsm.cpp -c -o JasonAsm.o

Jason.o: Makefile Jason.h JasonBuilder.h JasonDumper.h JasonParser.h JasonSlice.h JasonType.h Jason.cpp
	$(CC) $(CFLAGS) Jason.cpp -c -o Jason.o

test:	Makefile test.cpp fpconv.o Jason.h Jason.o JasonAsm.o
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o Jason.o JasonAsm.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp fpconv.o Jason.h Jason.o JasonAsm.o
	$(CC) $(CFLAGS) bench.cpp fpconv.o Jason.o JasonAsm.o -o bench

JasonAsm: Makefile JasonAsm.h JasonAsm.cpp
	$(CC) $(CFLAGS) JasonAsm.cpp -DCOMPILE_JASONASM_UNITTESTS -o JasonAsm

clean:	
	rm -rf *.o test bench

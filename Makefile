CC=g++
CFLAGS=-Wall -Wextra -std=c++11 -g -O3 
#CFLAGS=-Wall -Wextra -std=c++11 -g -O3 -DJASON_VALIDATEUTF8 
#CFLAGS=-Wall -Wextra -std=c++11 -g -O0 -DJASON_DEBUG -DJASON_VALIDATEUTF8

all:	test bench

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o: Makefile powers.h fpconv.h fpconv.cpp
	$(CC) $(CFLAGS) fpconv.cpp -c -o fpconv.o

Jason.o: Makefile Jason.h JasonBuilder.h JasonDumper.h JasonParser.h JasonSlice.h JasonType.h Jason.cpp
	$(CC) $(CFLAGS) Jason.cpp -c -o Jason.o

test:	Makefile test.cpp fpconv.o Jason.h Jason.o
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o Jason.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp fpconv.o Jason.h Jason.o
	$(CC) $(CFLAGS) bench.cpp fpconv.o Jason.o -o bench

clean:	
	rm -rf *.o test bench

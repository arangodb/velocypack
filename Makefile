CC=g++
CFLAGS=-Wall -Wextra -std=c++11 -g -O0

all:	test bench

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o: Makefile fpconv.h
	$(CC) $(CFLAGS) fpconv.cpp -c -o fpconv.o

JasonBuilder.o: Makefile JasonBuilder.h JasonBuilder.cpp Jason.h
	$(CC) $(CFLAGS) JasonBuilder.cpp -c -o JasonBuilder.o

JasonSlice.o: Makefile JasonSlice.h JasonSlice.cpp Jason.h
	$(CC) $(CFLAGS) JasonSlice.cpp -c -o JasonSlice.o

test:	Makefile test.cpp JasonDumper.h fpconv.o Jason.h JasonBuffer.h JasonBuilder.o JasonParser.h JasonSlice.o JasonType.h
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o JasonBuilder.o JasonSlice.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp Jason.h JasonBuilder.o JasonParser.h JasonSlice.o JasonType.h
	$(CC) $(CFLAGS) bench.cpp JasonBuilder.o JasonSlice.o -pthread -o bench

clean:	
	rm -rf *.o test bench

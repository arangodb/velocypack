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

Jason.o: Makefile JasonType.h Jason.h
	$(CC) $(CFLAGS) Jason.cpp -c -o Jason.o

JasonBuffer.o: Makefile JasonBuffer.h JasonBuffer.cpp Jason.o
	$(CC) $(CFLAGS) JasonBuffer.cpp -c -o JasonBuffer.o

JasonBuilder.o: Makefile JasonBuilder.h JasonBuilder.cpp Jason.o
	$(CC) $(CFLAGS) JasonBuilder.cpp -c -o JasonBuilder.o

JasonParser.o: Makefile JasonBuilder.o JasonParser.h JasonParser.cpp Jason.o
	$(CC) $(CFLAGS) JasonParser.cpp -c -o JasonParser.o

JasonSlice.o: Makefile JasonSlice.h JasonSlice.cpp Jason.o
	$(CC) $(CFLAGS) JasonSlice.cpp -c -o JasonSlice.o

JasonType.o: Makefile JasonType.h JasonType.cpp Jason.o
	$(CC) $(CFLAGS) JasonType.cpp -c -o JasonType.o

JasonUtils.o: Makefile JasonUtils.h JasonUtils.cpp Jason.o
	$(CC) $(CFLAGS) JasonUtils.cpp -c -o JasonUtils.o

test:	Makefile test.cpp JasonDumper.h fpconv.o Jason.o JasonBuffer.o JasonBuilder.o JasonParser.o JasonSlice.o \
        JasonUtils.o JasonType.o
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o Jason.o JasonBuffer.o JasonBuilder.o JasonParser.o JasonSlice.o JasonType.o JasonUtils.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp Jason.o JasonBuilder.o JasonParser.o JasonSlice.o \
        JasonUtils.o JasonType.o
	$(CC) $(CFLAGS) bench.cpp Jason.o JasonBuilder.o JasonParser.o JasonSlice.o JasonType.o JasonUtils.o -pthread -o bench

clean:	
	rm -rf *.o test bench

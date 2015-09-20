CC=g++
CFLAGS=-Wall -Wextra -std=c++11 -g -O0

all:	test bench

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o:	Makefile fpconv.h
	$(CC) $(CFLAGS) fpconv.cpp -c -o fpconv.o

Jason.o:	Makefile JasonType.h Jason.h
	$(CC) $(CFLAGS) Jason.cpp -c -o Jason.o

JasonUtils.o:	Makefile JasonUtils.cpp Jason.h JasonType.h
	$(CC) $(CFLAGS) JasonUtils.cpp -c -o JasonUtils.o

JasonSlice.o:	Makefile JasonSlice.cpp JasonSlice.h Jason.h JasonType.h
	$(CC) $(CFLAGS) JasonSlice.cpp -c -o JasonSlice.o

JasonDumper.o: Makefile JasonDumper.cpp JasonDumper.h Jason.h JasonType.h
	$(CC) $(CFLAGS) JasonDumper.cpp -c -o JasonDumper.o

JasonType.o: Makefile JasonType.h Jason.h
	$(CC) $(CFLAGS) JasonType.cpp -c -o JasonType.o

test:	Makefile test.cpp fpconv.o JasonBuilder.h Jason.h Jason.o JasonUtils.o JasonBuffer.h JasonParser.h JasonDumper.h \
        JasonDumper.o JasonSlice.h JasonSlice.o JasonType.h JasonType.o
	$(CC) $(CFLAGS) -Igoogletest/googletest/include test.cpp fpconv.o Jason.o JasonDumper.o JasonSlice.o JasonType.o JasonUtils.o googletest/googletest/libgtest.a -pthread -o test

bench: Makefile bench.cpp fpconv.o JasonBuilder.h Jason.h Jason.o JasonUtils.o JasonBuffer.h JasonParser.h JasonDumper.h \
        JasonDumper.o JasonSlice.h JasonSlice.o JasonType.h JasonType.o
	$(CC) $(CFLAGS) bench.cpp fpconv.o Jason.o JasonDumper.o JasonSlice.o JasonType.o JasonUtils.o -pthread -o bench

clean:	
	rm -rf *.o test bench

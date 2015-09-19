CC=g++

all:	test

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

fpconv.o:	Makefile fpconv.h
	$(CC) fpconv.cpp -Wall -Wextra -g -std=c++11 -c -o fpconv.o

Jason.o:	Makefile JasonType.h Jason.h
	$(CC) Jason.cpp -Wall -Wextra -g -std=c++11 -c -o Jason.o

JasonUtils.o:	Makefile JasonUtils.cpp Jason.h JasonType.h
	$(CC) JasonUtils.cpp -Wall -Wextra -g -std=c++11 -c -o JasonUtils.o

JasonSlice.o:	Makefile JasonSlice.cpp JasonSlice.h Jason.h JasonType.h
	$(CC) JasonSlice.cpp -Wall -Wextra -g -std=c++11 -c -o JasonSlice.o

JasonType.o:	Makefile JasonType.h Jason.h
	$(CC) JasonType.cpp -Wall -Wextra -g -std=c++11 -c -o JasonType.o

test:	Makefile test.cpp fpconv.o JasonBuilder.h Jason.h Jason.o JasonUtils.o JasonBuffer.h JasonParser.h JasonDumper.h \
        JasonSlice.o JasonType.h JasonType.o
	$(CC) -Igoogletest/googletest/include test.cpp fpconv.o Jason.o JasonSlice.o JasonType.o JasonUtils.o googletest/googletest/libgtest.a -pthread -Wall -Wextra -g -std=c++11 -o test

clean:	
	rm -rf *.o test

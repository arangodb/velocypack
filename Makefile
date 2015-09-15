all:	test

.PHONY: googletest

googletest:
	rm -Rf googletest
	git clone https://github.com/google/googletest.git
	cd googletest/googletest && $(CC) -isystem ./include/ -I. -pthread -c ./src/gtest-all.cc
	cd googletest/googletest && ar -rv libgtest.a gtest-all.o

Jason.o:	Makefile JasonType.h Jason.h
	g++ Jason.cpp -Wall -Wextra -g -std=c++11 -c -o Jason.o

JasonUtils.o:	Makefile JasonUtils.cpp Jason.h JasonType.h
	g++ JasonUtils.cpp -Wall -Wextra -g -std=c++11 -c -o JasonUtils.o

JasonSlice.o:	Makefile JasonSlice.cpp Jason.h JasonType.h
	g++ JasonSlice.cpp -Wall -Wextra -g -std=c++11 -c -o JasonSlice.o

JasonType.o:	Makefile JasonType.h Jason.h
	g++ JasonType.cpp -Wall -Wextra -g -std=c++11 -c -o JasonType.o

test:	Makefile test.cpp JasonBuilder.h Jason.h Jason.o JasonUtils.o JasonParser.h \
        JasonSlice.o JasonType.h JasonType.o
	g++ -Igoogletest/googletest/include test.cpp Jason.o JasonSlice.o JasonType.o JasonUtils.o googletest/googletest/libgtest.a -pthread -Wall -g -std=c++11 -o test

clean:	
	rm -rf *.o test

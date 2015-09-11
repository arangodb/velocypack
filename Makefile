all:	test

test:	Makefile test.cpp JasonBuilder.h Jason.h JasonParser.h JasonSlice.h JasonSlice.cpp JasonType.h JasonType.cpp
	g++ JasonSlice.cpp -Wall -g -std=c++11 -c -o JasonSlice.o
	g++ JasonType.cpp -Wall -g -std=c++11 -c -o JasonType.o
	g++ test.cpp JasonSlice.o JasonType.o -Wall -g -std=c++11 -o test

clean:	
	rm -rf test

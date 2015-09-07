all:	test

test:	Makefile test.cpp JasonBuilder.h Jason.h JasonType.h JasonParser.h JasonSlice.h
	g++ test.cpp -Wall -std=c++11 -o test

clean:	
	rm -rf test

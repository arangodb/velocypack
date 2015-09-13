all:	test

test:	Makefile test.cpp JasonBuilder.h Jason.h JasonUtils.h JasonUtils.cpp JasonParser.h JasonSlice.h JasonSlice.cpp JasonType.h JasonType.cpp
	g++ JasonUtils.cpp -Wall -Wextra -g -std=c++11 -c -o JasonUtils.o
	g++ JasonSlice.cpp -Wall -Wextra -g -std=c++11 -c -o JasonSlice.o
	g++ JasonType.cpp -Wall -Wextra -g -std=c++11 -c -o JasonType.o
	g++ test.cpp JasonSlice.o JasonType.o JasonUtils.o -Wall -g -std=c++11 -o test

clean:	
	rm -rf *.o test

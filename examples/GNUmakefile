all: usebuilder usebuilder2 useparser

usebuilder: usebuilder.cpp
	${CXX} usebuilder.cpp -Wall -std=c++11 -I../include -L.. -ljason -o usebuilder

usebuilder2: usebuilder2.cpp
	${CXX} usebuilder2.cpp -Wall -std=c++11 -I../include -L.. -ljason -o usebuilder2

useparser: useparser.cpp
	${CXX} useparser.cpp -Wall -std=c++11 -I../include -L.. -ljason -o useparser

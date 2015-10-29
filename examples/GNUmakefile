all: usebuilder usebuilder2 useparser useslicer usedumper

usebuilder: usebuilder.cpp
	${CXX} usebuilder.cpp -Wall -std=c++11 -I../include -L.. -ljason -o usebuilder

usebuilder2: usebuilder2.cpp
	${CXX} usebuilder2.cpp -Wall -std=c++11 -I../include -L.. -ljason -o usebuilder2

useparser: useparser.cpp
	${CXX} useparser.cpp -Wall -std=c++11 -I../include -L.. -ljason -o useparser

useslicer: useslicer.cpp
	${CXX} useslicer.cpp -Wall -std=c++11 -I../include -L.. -ljason -o useslicer

usedumper: usedumper.cpp
	${CXX} usedumper.cpp -Wall -std=c++11 -I../include -L.. -ljason -o usedumper -g -O0

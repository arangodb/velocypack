all: python2

python2:
	swig -c++ -python velocypack.i
	g++ -shared -fPIC -std=c++11 -Wall -O3 -DSWIG=1 -I../include -I../src -I/usr/include/python2.7 velocypack_wrap.cxx ../src/*.cpp -o _velocypack.so 

javascript:
	ln -s ../GNUmakefile .
	ln -s ../../include/velocypack .
	ln -s ../velocypack.i
	swig -c++ -javascript -node velocypack.i
	node-gyp configure
	node-gyp build

php:
	swig -c++ -php velocypack.i

clean:
	rm -rf _velocypack.so velocypack_wrap.cxx velocypack.py velocypack.pyc

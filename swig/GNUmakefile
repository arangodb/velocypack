all: python2

python2:
	swig -c++ -python velocypack.i
	g++ -shared -fPIC -std=c++11 -Wall -O3 -I../include -I../src -I/usr/include/python2.7 velocypack_wrap.cxx ../src/*.cpp -o _velocypack.so 

javascript:
	swig -c++ -javascript -node velocypack.i

php:
	swig -c++ -php velocypack.i

clean:
	rm -rf _velocypack.so velocypack_wrap.cxx velocypack.py velocypack.pyc

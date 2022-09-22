tiny: tiny.cc ./libDK.so
	g++ -std=c++11 -o tiny tiny.cc ./libDK.so -lpthread

./libDK.so : DK.cc
	g++ -std=c++11 -shared -fpic -o libDK.so DK.cc

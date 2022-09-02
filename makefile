dkfile : dkfile.cc ./libDK.so
	g++ -o dkfile dkfile.cc ./libDK.so

./libDK.so : DK.cc
	g++ -shared -fpic -o libDK.so DK.cc

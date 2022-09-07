tiny: tiny.cc ./libDK.so
	g++ -std=c++11 -o tiny tiny.cc ./libDK.so

dkfile : dkfile.cc ./libDK.so
	g++ -std=c++11 -o dkfile dkfile.cc ./libDK.so

echoclient: echoclient.cc ./libDK.so
	g++ -std=c++11 -o echoclient echoclient.cc ./libDK.so

echoserveri: echoserveri.cc ./libDK.so
	g++ -std=c++11 -o echoserveri echoserveri.cc ./libDK.so

./libDK.so : DK.cc
	g++ -std=c++11 -shared -fpic -o libDK.so DK.cc

VPATH = ./cgi_bin
tiny: tiny.o ./libDK.so Fei
	g++ -std=c++11 -o tiny tiny.o -L./ -lDK -lpthread

tiny.o: tiny.cc
	g++ -std=c++11 -c tiny.cc

./libDK.so : DK.cc
	g++ -std=c++11 -shared -fpic -o libDK.so DK.cc

Fei: Fei.cc
	cd ./cgi_bin && $(MAKE)

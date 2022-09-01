#include "DK.h"

void unix_error(const string& message){
	cout << message << endl;
	exit(0);
}

ssize_t Read(int fd, void* buf, int count){
	ssize_t ret;

	if((ret = read(fd, buf, count)) < 0){
		unix_error("Read error");
	}
	return ret;
}

ssize_t Write(int fd, void* buf, int count){
	ssize_t ret;

	if((ret = write(fd, buf, count)) < 0){
		unix_error("Write error");
	}
	return ret;
}

#ifndef DKhead
#define DKhead

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdlib.h>

typedef long ssize_t;
typedef unsigned long size_t;
using namespace std;

void unix_erroe(const string& message);
ssize_t Read(int fd, void *buf, int count);
ssize_t Write(int fd, void *buf, int count);

#endif

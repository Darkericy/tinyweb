#ifndef DKhead
#define DKhead

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <cstring>

#define RIO_BUFSIZE 8192
#define MAXLINE 8192

typedef struct {
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
} rio_t;

typedef long ssize_t;
typedef unsigned long size_t;
using namespace std;

//错误处理函数
void unix_erroe(const string& message);

//基础IO函数
ssize_t Read(int fd, void *buf, int count);
ssize_t Write(int fd, void *buf, int count);

//RIO包函数声明
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_writen(int fd, void* buf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *buf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

//给程序提供的接口
void rio_Readinitb(rio_t *rp, int fd);
void rio_Writen(int fd, void* buf, size_t n);
ssize_t rio_Readlineb(rio_t *rp, void *buf, size_t maxlen);
ssize_t rio_Readnb(rio_t *rp, void *usrbuf, size_t n);

#endif

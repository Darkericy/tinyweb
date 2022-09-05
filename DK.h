#ifndef DKhead
#define DKhead

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>

#define RIO_BUFSIZE 8192
#define MAXLINE 8192
#define LISTENQ 1024

typedef struct {
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
} rio_t;

typedef struct sockaddr SA;
typedef long ssize_t;
typedef unsigned long size_t;
using namespace std;

//错误处理函数
//这里分为unix、app、gai不同的错误我觉得可以重点关注一下，这样以后想要处理对应的错误的话就可以直接在对应的地方添加了。
//这就是所谓的扩展性吗？
void unix_erroe(const string& message);
void gai_error(int code, const string& message);
void app_error(const string& msg);

//基础IO函数
ssize_t Read(int fd, void *buf, int count);
ssize_t Write(int fd, void *buf, int count);
void Close(int fd);
char *Fgets(char *ptr, int n, FILE *streami);
void Fputs(const char *ptr, FILE *stream);

/*健壮的IO包，携带输入输出缓冲，可以处理基础包的不足*/

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

/*处理服务器接口的一些基本函数*/
void Getaddrinfo(const char* host, const char *service, const struct addrinfo *hints, struct addrinfo **result);
void Freeaddrinfo(struct addrinfo *result);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servlen, int flags);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/*服务器与客户端API*/
//基础函数
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

//接口
int Open_clientfd(char *hostname, char *port);
int Open_listenfd(char *port);

#endif

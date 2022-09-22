#ifndef DKhead
#define DKhead

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

const int RIO_BUFSIZE = 8192;
const int MAXLINE = 8192;
const int LISTENQ = 1024;

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
int Open(const char *pathname, int flags, mode_t mode);


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

//内存管理函数
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void Munmap(void *start, size_t length);

//进程相关函数
pid_t Fork();
int Dup2(int fd1, int fd2);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);

//重点处理网页请求的类
class Mission {
	using Action = void (Mission::*)(const string&);
private:
	//这个method用来保存不同请求方法对应的函数
	unordered_map<string, Action> method;
	//conned是初始化的连接描述符
	int conned;
	
	//此函数用来处理错误
	void error(const string& errcode, const string& errmsg);
	
	//处理函数
	bool parse_uri(const string& uri, string& filename, string& cgiargs);
	void serve_static(string& filename, int filesize);
	void serve_dynamic(string& filename, string& cgiargs);
	void get_filetype(string& filename, string& filetype);
	void read_requesthdrs(rio_t *rp);
	
	//方法分类
	void Get(const string& uri);
	void Head(const string& uri);
	void Post(const string& uri);
	void Put(const string& uri);
	void Delete(const string& uri);
	void Connect(const string& uri);
	void Options(const string& uri);
	void Trace(const string& uri);
	
public:
	Mission(int connect):conned(connect) 
	{
		method["GET"] = &Mission::Get;
        	method["HEAD"] = &Mission::Head;
        	method["POST"] = &Mission::Post;
        	method["PUT"] = &Mission::Put;
        	method["DELETE"] = &Mission::Delete;
        	method["CONNECT"] = &Mission::Connect;
        	method["OPTIONS"] = &Mission::Options;
        	method["TRACE"] = &Mission::Trace;
	};
	
	//析构函数，保证描述符正确关闭
	
	~Mission(){
		Close(conned);
	};	

	//明确删除我不需要的函数
	Mission() = delete;
	Mission(const Mission&) = delete;
	Mission& operator=(const Mission&) = delete;

	//这个函数就是处理网页请求的主体
	void start();
};

//生产者消费者模型的缓冲区实现
//使用循环队列模型外加多线程操作
class sbuf{
private:
	vector<int> buf; //保存描述符的
	int maxlen; //最大容量
	int front, tail; //头指针尾指针
	mutex mtx;
	condition_variable cnd;	

public:
	sbuf(int max): maxlen(max), buf(max + 1), front(0), tail(0) {};
	~sbuf() = default;
	
	//明确删除我不需要的函数
	sbuf(const sbuf&) = delete;
	sbuf& operator=(const sbuf&) = delete;

	//向数组中插入删除描述符
	void insert(int conned);
	int get();
};

#endif

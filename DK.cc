#include "DK.h"

//错误处理函数，虽然看起来多与但是却是含金量的体现
void unix_error(const string& message){
	cerr << message << ": ";
	cerr << strerror(errno) << endl;
	exit(0);
}

void gai_error(int code, const string& message){
	cerr << message << ": ";
	cerr << gai_strerror(code) << endl;
}

void app_error(char *msg) /* Application error */
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}


/*基础IO函数*/
//基础io的接口
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

void Close(int fd){
	int rc;
	
	if((rc = close(fd)) < 0){
		unix_erro("close error");
	}
}

/*健壮的IO包实现*/
//这里的rio_read声明为静态函数就是向只作为这个文件的函数使用。
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
	int cnt;
	
	while (rp->rio_cnt <= 0) {
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if (rp->rio_cnt < 0) {
			if(errno != EINTR)
				return -1;
		}
		else if (rp->rio_cnt == 0) 
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;
	}
	
	cnt = min(static_cast<int>(n), rp->rio_cnt);
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}

//RIO包函数实现
void rio_readinitb(rio_t *rp, int fd){
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n){
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = static_cast<char*>(usrbuf);
	
	while(nleft > 0){
		if ((nwritten = write(fd, bufp, nleft)) <= 0) {
			if(errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}


ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
	int n, rc;
	char c, *bufp = static_cast<char*>(usrbuf);

	for (n = 1; n < maxlen; ++n){
		if ((rc = rio_read(rp, &c, 1)) == 1) {
			*bufp++ = c;
			if (c == '\n') {
				++n;
				break;
			}
		}
		else if (rc == 0) {
			if (n == 1)
				return 0;
			else
				break;
		} else
			return -1;
	}
	*bufp = 0;
	return n - 1;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n){
	size_t nleft = n;
	ssize_t nread;
	char *bufp = static_cast<char*>(usrbuf);
	
	while (nleft > 0) {
		if ((nread = rio_read(rp, bufp, nleft)) < 0)
			return -1;
		else if (nread == 0)
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

//程序接口
void rio_Readinitb(rio_t *rp, int fd){
	rio_readinitb(rp, fd);
}

void rio_Writen(int fd, void* usrbuf, size_t n){
	if (rio_writen(fd, usrbuf, n) != n)
        unix_error("Rio_writen error");
}

ssize_t rio_Readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
	ssize_t rc;
	
	if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
        	unix_error("Rio_readlineb error");
    	return rc;
}

ssize_t rio_Readnb(rio_t *rp, void *usrbuf, size_t n){
	ssize_t rc;

    	if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
        	unix_error("Rio_readnb error");
    	return rc;
}

/*服务器编程的基础处理函数*/
void Getaddrinfo(const char* host, const char *service, const struct addrinfo *hints, struct addrinfo **result){
	int rc;

	if((rc = getaddrinfo(host, service, hints, result)) != 0 ){
		gai_error(rc, "getaddrinfo error");
	}
	return rc;
}

void Freeaddrinfo(struct addrinfo *result){
	freeadrinfo(result);
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servlen, int flags){
	int rc;

	if((rc = getnameinfo(sa, salen, host, hostlen, service, sevlen, flags)) != 0)
		gai_error(rc, "getnameinfo error");
}

void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    int rc;

    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
        unix_error("Setsockopt error");
}


/*服务器套接字接口辅助函数*/
//基础函数实现,csapp下载的代码和书中的代码有略微的不同，下载下来的代码实现中调用下面两个函数会有以下两个情况：
//1、如果是get函数出错就返回-2
//2、如果是套接字函数出错返回-1
//在我照着书中的程序实现后是以下两种情况
//1、get函数出错就打印错误信息并直接终止
//2、套接字API函数出错就调用unix_error
//个人认为书中的更好，比他自己在官网上放的好，但是他这么做有可能有其他理由，我不好说。

//如果出错那么返回-1，正常则返回连接好的文件描述符
int open_clientfd(char *hostname, char *port){
	int clientfd;
	struct addrinfo hints, *listp, *p;
	
	//作初始化设置，对get函数的输出做一些约束
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_flags |= AI_ADDRCONFIG;
	Getaddrinfo(hostname, port, &hints, &listp);

	//遍历输出的列表，找到我们能够连接的
	for(p = listp; p; p = p->ai_next){
		if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;
		
		if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
			break;
		Close(clientfd);
	}
	
	//清理掉get函数产生的列表
	Freeaddrinfo(listp);
	if(!p)
		return -1;
	else
		return clientfd;
}

int open_listenfd(char *port){
	struct addrinfo hints, *listp, *p;
	int lintenfd, optval = 1;
	
	//初始化和上面函数一样的操作
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;
	Getaddrinfo(NULL, port, &hints, &listp);

	for(p = listp; p; p = p->ai_next){
		if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, static_cast<void *>(&optval), sizeof(int));

		if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		Close(listenfd);
	}

	//清理内存
	Freeaddrinfo(listp);
	if(!p)
		return -1;
	
	if(listen(listenfd, LISTENQ) < 0){
		Close(listenfd);
		return -1;
	}
	return listenfd;
}

//应用程序接口

int Open_clientfd(char *hostname, char *port){
	int rc;
	
	if((rc = open_clientfd(hostname, port)) < 0)
		unix_error("open_clientfd error");
	return rc;
}
	
int Open_listenfd(char *port){
	int rc;

	if((rc = open_listenfd(port)) < 0)
		unix_error("open_listenfd error");
	return rc;
}

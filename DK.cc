#include "DK.h"

void unix_error(const string& message){
	cout << message << ": ";
	cout << strerror(errno) << endl;
	exit(0);
}

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


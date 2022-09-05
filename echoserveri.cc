#include "DK.h"

void echo(int connfd){
	size_t n;
	char *buf[MAXLINE];
	rio_t rio;

	rio_Readinitb(&rio, connfd);
	while((n = rio_Readlineb(&rio, buf, MAXLINE)) != 0){
		cout << "接受到" << static_cast<int>(n) << "个字符" << endl;
		rio_Writen(connfd, buf, n);
	}
}

int main(int argc, char **argv){
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	if(argc != 2){
		cerr << "usage: " << argv[0] << " <port>" << endl;
		return 0;
	}

	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(struct sockaddr_storage);
		//这里使用reinterpret_cast来转换sockaddr_storage变量，因为他们的大小并不一样，需要提供底层的重新解释，所以static_cast不管用
		connfd = Accept(listenfd, reinterpret_cast<SA *>(&clientaddr), &clientlen);
		Getnameinfo(reinterpret_cast<SA *>(&clientaddr), clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		cout << "连接到" << client_hostname << " " << client_port << endl;
		echo(connfd);
		Close(connfd);
	}
	return 0;
}

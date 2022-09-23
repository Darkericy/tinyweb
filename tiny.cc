#include "DK.h"


void read_requesthdrs(rio_t *rp);
bool parse_uri(string& uri, string& filename, string& cgiargs);
void serve_static(int fd, string& filename, int filesize);
void get_filetype(string& filename, string& filetype);
void serve_dynamic(int fd, string& filename, string& cgiargs);
void clienterror(int fd, const string& cause, const string& errnum, const string& shortmsg, const string& longmsg);
void doit(int fd);

sbuf usr_buf(4);
int main(int argc, char **argv){
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	if(argc != 2){
		cerr << "usage: " << argv[0] << " <port>" << endl;
		exit(1);
	}

	for(int i = 0; i < 4; ++i){
		thread temp(doit, 1);
		temp.detach();
	}

	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, reinterpret_cast<SA *>(&clientaddr), &clientlen);
		Getnameinfo(reinterpret_cast<SA *>(&clientaddr), clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		cout << "连接到：" << hostname << " " << port << " ";
		usr_buf.insert(connfd);
	}
	return 0;
}

void doit(int _){
	while(1){
		int temp = usr_buf.get();
		Mission cur_m(temp);
		cur_m.start();
	}
}

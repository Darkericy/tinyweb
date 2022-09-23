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

	//创建线程池	
	for(int i = 0; i < 4; ++i){
		thread temp(doit, 1);
		//cout << "创建一个线程" << endl;
		temp.detach();
	}

	//cout << "到达监听描述符之前" << endl;
	listenfd = Open_listenfd(argv[1]);
	while(1){
		//cout << "准备监听" << endl;
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, reinterpret_cast<SA *>(&clientaddr), &clientlen);
		//cout << "获得描述符：" << connfd << endl;
		Getnameinfo(reinterpret_cast<SA *>(&clientaddr), clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		cout << "连接到：" << hostname << " " << port << " ";
		usr_buf.insert(connfd);
		//cout << "插入描述符成功" << endl;
	}
	return 0;
}

void doit(int _){
	while(1){
		//cout << "这里是子线程" << endl;
		int temp = usr_buf.get();
		Mission cur_m(temp);
		cur_m.start();
	}
}

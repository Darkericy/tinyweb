#include "DK.h"


void read_requesthdrs(rio_t *rp);
bool parse_uri(string& uri, string& filename, string& cgiargs);
void serve_static(int fd, string& filename, int filesize);
void get_filetype(string& filename, string& filetype);
void serve_dynamic(int fd, string& filename, string& cgiargs);
void clienterror(int fd, const string& cause, const string& errnum, const string& shortmsg, const string& longmsg);
void doit(int fd);

int main(int argc, char **argv){
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	if(argc != 2){
		cerr << "usage: " << argv[0] << " <port>" << endl;
		exit(1);
	}

	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, reinterpret_cast<SA *>(&clientaddr), &clientlen);
		Getnameinfo(reinterpret_cast<SA *>(&clientaddr), clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		cout << "连接到：" << hostname << " " << port << " ";
		doit(connfd);
		Close(connfd);
	}
	return 0;
}

void doit(int fd){
	string errnum, shortmsg, longmsg;
	char buf[MAXLINE];
	rio_t rio;
	rio_Readinitb(&rio, fd);
	rio_Readlineb(&rio, buf, MAXLINE);
	cout << "请求报头：" << endl;
	cout << buf;
	
	string usrbuf(buf);
	istringstream input(usrbuf);
	string method, uri, version;
	input >> method >> uri >> version;
	if(method != "GET"){
		errnum = "501", shortmsg = "未实现", longmsg = "Tiny还没有实现这种请求";
		clienterror(fd, method, errnum, shortmsg, longmsg);
		return;
	}
	read_requesthdrs(&rio);

	string filename, cgiargs;
	struct stat sbuf;
	auto is_static = parse_uri(uri, filename, cgiargs);
	if(stat(filename.c_str(), &sbuf) < 0){
		errnum = "404", shortmsg = "未找到", longmsg = "Tiny没有这个文件";
		clienterror(fd, method, errnum, shortmsg, longmsg);
		return;
	}

	if(is_static){
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
			errnum = "403", shortmsg = "无权限", longmsg = "Tiny不能阅读这个文件";
			clienterror(fd, method, errnum, shortmsg, longmsg);
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else{
	        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
			errnum = "403", shortmsg = "无权限", longmsg = "Tiny不能执行这个文件";
                        clienterror(fd, method, errnum, shortmsg, longmsg);
                        return;
                }
		serve_dynamic(fd, filename, cgiargs);
	}
}

void clienterror(int fd, const string& cause, const string& errnum, const string& shortmsg, const string& longmsg){
	string buf = "", body = "";
	body += "<html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><title>错误！</title>\r\n";
	body += "<p>发生了错误，错误代号：" + errnum + "\r\n";
	body += "<p>错误信息：" + longmsg + "\r\n";
	body += "<hr><em>服主太懒了就写了两行错误信息</em>\r\n";

	buf = "HTTP/1.0 " + errnum + " " + shortmsg + "\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());
	buf = "Content-type: text/html\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());
	buf = "Content-length: " + to_string(body.size()) + "\r\n\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());
	rio_Writen(fd, const_cast<char*>(body.c_str()), body.size());
}

void read_requesthdrs(rio_t *rp){
	char buf[MAXLINE];

	rio_Readlineb(rp, buf, MAXLINE);
	while(strcmp(buf, "\r\n")){
		rio_Readlineb(rp, buf, MAXLINE);
		cout << buf;
	}
	return;
}

bool parse_uri(string& uri, string& filename, string& cgiargs){
	if(uri.find("cgi_bin") == string::npos){
		cgiargs = "";
		filename = "." + uri;
		if(uri.size() == 1 && uri[0] == '/'){
			filename += "home.html";
		}
		return true;
	}else{
		auto ptr = uri.find('?');
		if(ptr != string::npos){
			cgiargs = "";
		}else{
			cgiargs = "";
			ptr = uri.size();
		}
		filename = "." + uri.substr(0, ptr);
		return false;
	}
}

void serve_static(int fd, string& filename, int filesize){
	int srcfd;
	char *srcp;
	string filetype, buf = "";

	get_filetype(filename, filetype);
	buf += "HTTP/1.0 200 OK\r\n";
	buf += "Server: Tiny Web Server\r\n";
	buf += "Connection: close\r\n";
	buf += "Content-length: " + to_string(filesize) + "\r\n";
	buf += "Content-type: " + filetype + "\r\n\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());
	cout << "响应报头：" << endl;
	cout << buf;

	srcfd = Open(filename.c_str(), O_RDONLY, 0);
	srcp =  static_cast<char*>(Mmap(nullptr, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0));
	Close(srcfd);
	rio_Writen(fd, srcp, filesize);
	Munmap(srcp, filesize);
}

void get_filetype(string& filename, string& filetype){
	if(filename.find(".html") != string::npos)
		filetype = "text/html";
	else if(filename.find(".gif") != string::npos)
		filetype = ".image/gif";
	else if(filename.find(".png") != string::npos)
		filetype = "image/png";
	else if(filename.find(".jpg") != string::npos)
		filetype = "image/jpeg";
	else 
		filetype = "text/plain";
}

void serve_dynamic(int fd, string& filename, string& cgiargs){
	string buf = "";
	char *emptylist[] = {nullptr};
	buf = "HTTP/1.0 200 OK\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());
	buf = "Server: Tiny Web Server\r\n";
	rio_Writen(fd, const_cast<char*>(buf.c_str()), buf.size());

	if(Fork() == 0){
		setenv("QUERY_STRING", cgiargs.c_str(), 1);
		Dup2(fd, STDOUT_FILENO);
		Execve(filename.c_str(), emptylist, environ);
	}
	Wait(nullptr);
}


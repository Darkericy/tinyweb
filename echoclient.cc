#include "DK.h"

int main(int argc, char **argv){
	int clientfd;
	char *host, *port, buf[MAXLINE];
	rio_t rio;

	if(argc != 3){
		cerr << "usage: " << argc[0] << " <host> <port>" << endl;
		exit(0);
	}
	host = argv[1];
	port = argv[2];

	clientfd = Open_clientfd(host, port);
	rio_Readinitb(&rio, clientfd);

	while(Fgets(buf, MAXLINE, stdin) != NULL

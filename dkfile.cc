#include "DK.h"

int main(int argc, char **argv){
	int n;
	rio_t rio;
	char buf[MAXLINE];
	
	rio_Readinitb(&rio, STDIN_FILENO);
	while ((n = rio_Readlineb(&rio, buf, MAXLINE)) != 0 )
		rio_Writen(STDOUT_FILENO, buf, n);
	return 0;
}

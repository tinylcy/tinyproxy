#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

int open_listenfd(int port) {
	int listenfd = -1;
	struct sockaddr_in serveraddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serveraddr, 0, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
		return -1;
	}

	if(listen(listenfd, 1024) < 0) {
		return -1;
	}

	return listenfd;
}

int open_clientfd(char *hostname, int port) {
	int clientfd;
	struct sockaddr_in serveraddr;
	if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	if(inet_pton(AF_INET, hostname, &serveraddr.sin_addr) < 0) {
		return -1;
	}
	if(connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
		return -1;
	}

	return clientfd;
	
}

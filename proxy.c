#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include "rio.h"
#include "socketlib.h"

#ifndef MAXLEN
#define MAXLEN 8192
#endif

#define USERAGENT "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3"

void *do_it(int connfd);
int parse_uri(char *uri, char *hostname, char *pathname, int *port);

int main(int argc, char **argv) {
	
	int listenfd, connfd;
	char buf[MAXLEN];
	int nread;

	listenfd = open_listenfd(15213);
	if(listenfd == -1) {
		perror("open_listenfd");
		return -1;
	}

	while(1) {
		connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		do_it(connfd);
	}

	return 0;
}

void *do_it(int connfd) {
	rio_t rio;
	char line[MAXLEN];
	int nread;
	int ret;

	rio_readinitb(&rio, connfd);
	nread = rio_readlineb(&rio, line, MAXLEN);
	if(nread == -1) {
		perror("rio_readlineb");
		return NULL;
	}

	char method[MAXLEN], uri[MAXLEN], version[MAXLEN];
	sscanf(line, "%s %s %s", method, uri, version);
    printf("method: %s\n", method);
	printf("uri: %s\n", uri);
	printf("version: %s\n", version);
	
	char hostname[MAXLEN], pathname[MAXLEN];
	int port;
	ret = parse_uri(uri, hostname, pathname, &port);
	printf("hostname: %s\n", hostname);
	printf("pathname: %s\n", pathname);
	printf("port: %d\n", port);
	printf("\n");
	
	/* send request to server */
	rio_t client_rio;
	char req_buf[MAXLEN];
	int clientfd = open_clientfd(hostname, port);

	rio_readinitb(&client_rio, clientfd);
	sprintf(req_buf, "%s %s %s\r\n", method, pathname, version);
	rio_writen(clientfd, req_buf, strlen(req_buf));
	printf("%s", req_buf);
	int pro_conn_fnd = 0, conn_fnd = 0;
	while((nread = rio_readlineb(&rio, req_buf, MAXLEN)) > 2) {
		if(strstr(req_buf, "Proxy-Connection")) {
			pro_conn_fnd = 1;
			strcpy(req_buf, "Proxy-Connection: close\r\n");
		} else if(strstr(req_buf, "Connection")) {
			conn_fnd = 1;
			strcpy(req_buf, "Connection: close\r\n");
		}
		rio_writen(clientfd, req_buf, strlen(req_buf));
		printf("%s", req_buf);
	}
	
	if(!pro_conn_fnd) {
		strcpy(req_buf, "Proxy-Connection: close\r\n");
		rio_writen(clientfd, req_buf, strlen(req_buf));
		printf("%s", req_buf);
	}
	if(!conn_fnd) {
		sprintf(req_buf, "Connection: close\r\n");
		rio_writen(clientfd, req_buf, strlen(req_buf));
		printf("%s", req_buf);
	}
	
	rio_writen(clientfd, "\r\n", 2);

	printf("\n");

	/* read response content and send to client*/
	char resp_buf[MAXLEN];
	while((nread = rio_readlineb(&client_rio, resp_buf, MAXLEN)) != 0) {
		rio_writen(connfd, resp_buf, nread);
		printf("%s", resp_buf);
	}
	
	if(close(connfd) == -1) {
		perror("close");
	}
	if(close(clientfd) == -1) {
		perror("close");	
	}

	return NULL;
}

int parse_uri(char *uri, char *hostname, char *pathname, int *port) {
	char *host_begin;
	char *host_end;
	int host_len;
	char port_buf[8];
	int port_len;
	char *path_begin;

	/* extract the hostname */
	if(strncasecmp(uri, "http://", 7) != 0) {
		hostname[0] = '\0';
		return -1;
	}
	host_begin = uri + 7;
	if(strpbrk(host_begin, ":/") == NULL) {
		hostname[0] = '\0';
		return -1;
	}
	host_end = strpbrk(host_begin, ":/");
	host_len = host_end - host_begin;
	strncpy(hostname, host_begin, host_len);
	hostname[host_len] = '\0';

	/* extract the port */
	if(*host_end != ':') {
		*port = 80;
	} else {
		path_begin = strchr(host_end, '/');
		port_len = path_begin - host_end - 1;
		strncpy(port_buf, host_end + 1, port_len);
		port_buf[port_len] = '\0';
		*port = atoi(port_buf);
	}

	/* extract the pathname */
	if(*host_end == '/') {
		path_begin = host_end;
	} else {
		path_begin = host_end + port_len + 1;
	}
	strcpy(pathname, path_begin);

	return 0;

}












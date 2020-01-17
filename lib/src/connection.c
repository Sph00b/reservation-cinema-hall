#include "connection.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#define BACKLOG 4096
#define MSG_LEN 4096

int connection_listener_start(connection_t* connection, const char* address, uint16_t port){
	struct in_addr haddr;		//host address
	struct sockaddr_in saddr;	//socket address
	if (!inet_aton(address, &haddr)) {
		errno = EINVAL;
		return 1;
	}
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr = haddr;
	if ((*connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		return 1;
	}
	if (bind(*connection, (struct sockaddr*) &saddr, sizeof(saddr)) == -1){
		return 1;
	}
	if (listen(*connection, BACKLOG) == -1){
		return 1;
	}
	return 0;
}

int connection_close(const connection_t* connection){
	if (close(*connection) == -1){
		return 1;
	}
	return 0;
}

int connection_get_accepted(const connection_t* listener, connection_t* accepted){
	struct sockaddr addr;
	socklen_t addrlen;
	memset(&addrlen, 0, sizeof(socklen_t));
	if ((*accepted = accept(*listener, &addr, &addrlen)) == -1){
		return 1;
	}
	return 0;
}

int connection_recv(const connection_t* connection, char** buff) {
	ssize_t len;
	if ((*buff = (char*)malloc(sizeof(char) * MSG_LEN)) == NULL) {
		return 1;
	}
	len = recv(*connection, *buff, MSG_LEN, 0);
	if (len == -1 || len > MSG_LEN) {
		free(*buff);
		return 1;
	}
	if ((*buff)[len - 1] == '\n') {
		(*buff)[len - 1] = 0;
		len--;
	}
	if (realloc(*buff, len) == NULL) {
		free(*buff);
		return 1;
	}
	return 0;
}

int connection_send(const connection_t* connection, const char* buff) {
	if (send(*connection, buff, strlen(buff), 0) == -1) {
		return 1;
	}
	return 0;
}
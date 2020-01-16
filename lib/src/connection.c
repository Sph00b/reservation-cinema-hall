#include "connection.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#define BACKLOG 4096

int connection_listener_start(connection_t* connection, const char* address, uint16_t port){
	struct in_addr haddr;		//host address
	struct sockaddr_in saddr;	//socket address
	if (connection == NULL) {
		return 1;
	}
	if (!inet_aton(address, &haddr)) {
		errno = EINVAL;
		return 1;
	}
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr = haddr;
	if ((connection->passive_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		return 1;
	}
	if (bind(connection->passive_sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) == -1){
		return 1;
	}
	if (listen(connection->passive_sockfd, BACKLOG) == -1){
		return 1;
	}
	return 0;
}

int connection_listener_stop(connection_t* connection){
	if (connection == NULL) {
		return 1;
	}
	if (close(connection->passive_sockfd) == -1){
		return 1;
	}
	return 0;
}

int connection_accepted_getfd(connection_t* connection, int* sockfd){
	struct sockaddr addr;
	socklen_t addrlen;
	if (connection == NULL) {
		return 1;
	}
	if ((*sockfd = accept(connection->passive_sockfd, &addr, &addrlen)) == -1){
		return 1;
	}
	return 0;
}

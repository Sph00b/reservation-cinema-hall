#include "connection.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BACKLOG 4096

/*This version supports only a listening socket atm but it can be easly generalized*/

int *passive_sockfd = NULL;

int connection_listener_start(const char* address, uint16_t port){
	//function expects correct parameters
	if ((passive_sockfd = malloc(sizeof(int))) == NULL){
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(address);

	if ((passive_sockfd[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		return -1;
	}
	if (bind(passive_sockfd[0], (struct sockaddr*) &addr, sizeof(addr)) == -1){
		return -1;
	}
	if (listen(passive_sockfd[0], BACKLOG) == -1){
		return -1;
	}
	return passive_sockfd[0];
}

int connection_listener_stop(){
	if (passive_sockfd == NULL){
		return close(-1);
	}	
	if (close(passive_sockfd[0]) == -1){
		return -1;
	}
	free(passive_sockfd);
}

int connection_accepted_getfd(){
	if (passive_sockfd == NULL){
		return accept(-1, NULL, NULL);
	}
	int sockfd;
	struct sockaddr addr;
	socklen_t addrlen;
	if ((sockfd = accept(passive_sockfd[0], &addr, &addrlen)) == -1){
		return -1;
	}
	return sockfd;
}

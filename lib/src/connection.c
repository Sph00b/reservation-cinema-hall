#include "connection.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix__
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#include <basetsd.h>
#define ssize_t SSIZE_T
#endif

#define BACKLOG 4096
#define MSG_LEN 64

int connection_close(const connection_t* connection){
	if (close(*connection) == -1){
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
	if ((*buff)[len - 2] == '\n') {
		len--;
	}
	(*buff)[len - 1] = 0;
	/*	Resize if string is shorter than len	*/
	if (realloc(*buff, strlen(*buff) + 1) == NULL) {
		free(*buff);
		return 1;
	}
	return 0;
}

/* return number of bytes sended */

int connection_send(const connection_t* connection, const char* buff) {
	if (send(*connection, buff, strlen(buff), 0) == -1) {
		return 1;
	}
	return 0;
}

#ifdef __unix__

int connection_listener_start(connection_t* connection, const char* address, const uint16_t port) {
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
	if ((*connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		return 1;
	}
	if (bind(*connection, (struct sockaddr*) & saddr, sizeof(saddr)) == -1) {
		return 1;
	}
	if (listen(*connection, BACKLOG) == -1) {
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

#endif

//#include <stdio.h>
//
//#define max_line 4096
//
//int connection() {
//	WSADATA wsadata;
//	SOCKET conn_s;						/*	connection socket			*/
//	struct sockaddr_in servaddr;		/*	socket address structure	*/
//	unsigned short port = 55555;		/*	port number					*/
//	TCHAR* szaddress = "127.0.0.1";		/*	holds remote ip address		*/
//	TCHAR* buffer;						/*	character buffer			*/
//
//	if (wsastartup(makeword(1, 1), &wsadata) != 0) {
//		errorhandler();
//	}
//
//	/*	create the listening socket	*/
//
//	if ((conn_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
//		errorhandler();
//	}
//
//	/*	set all bytes in socket address structure to zero, and fill the relevant data members	*/
//
//	memset(&servaddr, 0, sizeof(servaddr));
//	servaddr.sin_family = AF_INET;
//	servaddr.sin_port = htons(port);
//	servaddr.sin_addr.s_addr = inet_addr(szaddress);
//
//	/* connect() to the remote echo server	*/
//
//	if (connect(conn_s, (struct sockaddr*) & servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
//		errorhandler();
//	}
//
//	/*prepare buffer and get string to echo from user	*/
//
//	if ((buffer = (TCHAR*)malloc(sizeof(TCHAR) * max_line)) == NULL) {
//		errorhandler();
//	}
//	memset(buffer, 0, max_line);
//	printf("inserire la stringa da spedire: ");
//	fgets(buffer, max_line, stdin);
//	buffer[strlen(buffer) - 1] = 0;
//
//	/*	send string to echo server, and retrive response	*/
//
//	if (send(conn_s, buffer, strlen(buffer) + 1, 0) < 0) {
//		errorhandler();
//	}
//	memset(buffer, 0, (sizeof(TCHAR)) * (strlen(buffer) + 1));
//	while (readline(conn_s, buffer, max_line - 1) == -1) {
//		errorhandler();
//	}
//
//	/*	output exhoed string	*/
//
//	printf("risposta del server: %s\n", buffer);
//
//	free(buffer);
//	wsacleanup();
//	return 0;
//}

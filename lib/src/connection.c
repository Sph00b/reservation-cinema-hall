#include "connection.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#ifndef _WINSOCKAPI_
		#define _WINSOCKAPI_
	#endif
	#include <Windows.h>
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#include <basetsd.h>
	#include <tchar.h>
#elif __unix__
	#include <unistd.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <sys/types.h>
	#include <errno.h>
	#define InetPton(Family, pszAddrString, pAddrBuf) inet_aton(pszAddrString, pAddrBuf)
	#define SSIZE_T ssize_t
	#define TCHAR char
#define _tcslen strlen
#define closesocket close
#endif

#define BACKLOG 4096
#define MSG_LEN 64

int connection_init(connection_t* connection, LPCTSTR address, const uint16_t port) {
	struct in_addr haddr;		//host address
#ifdef _WIN32
	if (WSAStartup(MAKEWORD(1, 1), &connection->WSAData) != 0) {
		return -1;
	}
#endif
	if ((connection->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		return -1;
	}
	if (!InetPton(AF_INET, address, &haddr)) {
		errno = EINVAL;
		return -1;
	}
	memset(&connection->addr, 0, sizeof(connection->addr));
	((struct sockaddr_in*) & connection->addr)->sin_family = AF_INET;
	((struct sockaddr_in*) & connection->addr)->sin_port = htons(port);
	((struct sockaddr_in*) & connection->addr)->sin_addr = haddr;
	connection->addrlen = sizeof(struct sockaddr_in);
	return 0;
}

int connection_close(const connection_t* connection){
	if (closesocket(connection->socket) == -1){
		return -1;
	}
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

int connection_recv(const connection_t* connection, LPTSTR* buff) {
	SSIZE_T len;
	char* utf8_buff;
	if ((utf8_buff = (char*)malloc(sizeof(char) * (MSG_LEN + 1))) == NULL) {
		return -1;
	}
	memset(utf8_buff, 0, sizeof(char) * (MSG_LEN + 1));
	len = recv(connection->socket, utf8_buff, MSG_LEN, 0);
	if (len == -1 || len > MSG_LEN) {
		free(utf8_buff);
		return -1;
	}
	utf8_buff[len] = 0;
	/*	Resize if string is shorter than len	*/
	if (realloc(utf8_buff, sizeof(char) * (strlen(utf8_buff) + 1)) == NULL) {
		free(utf8_buff);
		return -1;
	}
#ifdef _UNICODE
	int str_len;
	if (!(str_len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8_buff, -1, NULL, 0))) {
		free(utf8_buff);
		return -1;
	}
	if ((*buff = (LPWSTR)malloc(sizeof(WCHAR) * str_len)) == NULL) {
		free(utf8_buff);
		return -1;
	}
	if (!MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8_buff, -1, *buff, str_len)) {
		free(buff);
		free(utf8_buff);
		return -1;
	}
	free(utf8_buff);
#else
	*buff = utf8_buff;
#endif
	return len;
}

/* return number of bytes sended */

int connection_send(const connection_t* connection, LPCTSTR buff) {
	SSIZE_T len;
#ifdef _UNICODE
	char* utf8_buff = NULL;
	if (!(len = (SSIZE_T)WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buff, -1, NULL, 0, NULL, NULL))) {
		return -1;
	}
	if ((utf8_buff = (char*)malloc(sizeof(char) * len)) == NULL) {
		return -1;
	}
	if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buff, -1, utf8_buff, (int)len, NULL, NULL)) {
		free(utf8_buff);
		return -1;
	}
	if ((len = send(connection->socket, utf8_buff, strlen(utf8_buff), 0)) == -1) {
		free(utf8_buff);
		return -1;
	}
	free(utf8_buff);
#else
	if ((len = send(connection->socket, buff, strlen(buff), 0)) == -1) {
		return -1;
	}
#endif
	return len;
}

int connetcion_connect(const connection_t* connection) {
	if (connect(connection->socket, &connection->addr, connection->addrlen) == SOCKET_ERROR) {
		return -1;
	}
	return 0;
}

#ifdef __unix__

int connection_listener_start(connection_t* connection) {
	if (bind(connection->socket, &connection->addr, connection->addrlen) == -1) {
		return -1;
	}
	if (listen(connection->socket, BACKLOG) == -1) {
		return -1;
	}
	return 0;
}

int connection_get_accepted(const connection_t* listener, connection_t* accepted) {
	memset(&accepted->addrlen, 0, sizeof(socklen_t));
	if ((accepted->socket = accept(listener->socket, &accepted->addr, &accepted->addrlen)) == -1) {
		return -1;
	}
	return 0;
}

#endif
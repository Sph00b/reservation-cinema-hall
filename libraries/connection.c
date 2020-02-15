#include "connection.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
	#include <stddef.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <sys/types.h>
	#include <sys/un.h> 
	#include <errno.h>
	#define InetPton(Family, pszAddrString, pAddrBuf) inet_aton(pszAddrString, pAddrBuf)
	#define SSIZE_T ssize_t
	#define TCHAR char
#define _tcslen strlen
#define closesocket close
#endif

#define BACKLOG 4096
#define MSG_LEN 4096

int connection_init(connection_t* connection, LPCTSTR address, const uint16_t port) {
#ifdef __unix__
	if (!port) {
		struct sockaddr_un* paddr_un;
		if ((connection->socket = socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			return -1;
		}
		if ((paddr_un = malloc(sizeof(struct sockaddr_un))) == NULL) {
			return -1;
		}
		memset(paddr_un, 'x', sizeof(struct sockaddr_un));
		paddr_un->sun_family = AF_UNIX;
		paddr_un->sun_path[0] = '\0';
		strncpy(paddr_un->sun_path + 1, address, strlen(address));
		connection->addr = (struct sockaddr*)paddr_un;
		connection->addrlen = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + 1 + strlen(address));
		return 0;
	}
#endif
	struct sockaddr_in* paddr_in;
	struct in_addr haddr;		//host address
#ifdef _WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0) {
		return -1;
	}
#endif
	if ((connection->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		return -1;
	}
	if ((paddr_in = malloc(sizeof(struct sockaddr_in))) == NULL) {
		return -1;
	}
	if (!InetPton(AF_INET, address, &haddr)) {
		free(paddr_in);
		errno = EINVAL;
		return -1;
	}
	memset(&connection->addr, 0, sizeof(connection->addr));
	paddr_in->sin_family = AF_INET;
	paddr_in->sin_port = htons(port);
	paddr_in->sin_addr = haddr;
	connection->addr = (struct sockaddr*)paddr_in;
	connection->addrlen = sizeof(struct sockaddr_in);
	return 0;
}

int connection_close(const connection_t* connection){
	if (closesocket(connection->socket) == -1){
		return -1;
	}
	free(connection->addr);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

int connection_recv(const connection_t* connection, LPTSTR* buff) {
#ifdef _UNICODE
	char* utf8_buff;
#else
	#define utf8_buff (*buff)
#endif
	SSIZE_T len;
	if ((utf8_buff = malloc(sizeof(char) * (MSG_LEN + 1))) == NULL) {
		return -1;
	}
	memset(utf8_buff, 0, sizeof(char) * (MSG_LEN + 1));
	len = recv(connection->socket, utf8_buff, MSG_LEN, 0);
	if (len == -1 || len > MSG_LEN) {
		free(utf8_buff);
		utf8_buff = NULL;
		return -1;
	}
	/*	Resize if string is shorter than len	*/
	/*
	if (realloc(utf8_buff, sizeof(char) * (strlen(utf8_buff) + 1)) == NULL) {
		free(utf8_buff);
		utf8_buff = NULL;
		return -1;
	}
	*/
#ifdef _UNICODE
	int str_len;
	if (!(str_len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8_buff, -1, NULL, 0))) {
		free(utf8_buff);
		return -1;
	}
	if ((*buff = malloc(sizeof(WCHAR) * str_len)) == NULL) {
		free(utf8_buff);
		return -1;
	}
	if (!MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8_buff, -1, *buff, str_len)) {
		free(buff);
		free(utf8_buff);
		buff = NULL;
		return -1;
	}
	free(utf8_buff);
#else
	#undef utf8_buff
#endif
	return (int)len;
}

/* return number of bytes sended */

int connection_send(const connection_t* connection, LPCTSTR buff) {
	int len;
#ifdef _UNICODE
	char* utf8_buff = NULL;
	if (!(len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buff, -1, NULL, 0, NULL, NULL))) {
		return -1;
	}
	if ((utf8_buff = malloc(sizeof(char) * len)) == NULL) {
		return -1;
	}
	if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buff, -1, utf8_buff, (int)len, NULL, NULL)) {
		free(utf8_buff);
		return -1;
	}
	if ((len = send(connection->socket, utf8_buff, (int)strlen(utf8_buff), 0)) == -1) {
		free(utf8_buff);
		return -1;
	}
	free(utf8_buff);
#else
	if ((len = (int)send(connection->socket, buff, strlen(buff), 0)) == -1) {
		return -1;
	}
#endif
	return len;
}

int connetcion_connect(const connection_t* connection) {
	if (connect(connection->socket, connection->addr, connection->addrlen) == SOCKET_ERROR) {
		return -1;
	}
	return 0;
}

#ifdef __unix__

int connection_listen(connection_t* connection) {
	if (bind(connection->socket, connection->addr, connection->addrlen) == -1) {
		return -1;
	}
	if (listen(connection->socket, BACKLOG) == -1) {
		return -1;
	}
	return 0;
}

int connection_get_accepted(const connection_t* listener, connection_t* accepted) {
	memset(&accepted->addrlen, 0, sizeof(socklen_t));
	accepted->addr = malloc(sizeof(accepted->addrlen));
	if ((accepted->socket = accept(listener->socket, accepted->addr, &accepted->addrlen)) == -1) {
		return -1;
	}
	return 0;
}

#endif
#pragma once
#ifdef _WIN32
#pragma comment (lib, "Ws2_32.lib")
#endif

#include <stdint.h>

#ifdef _WIN32
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef struct {
	SOCKET socket;
	struct sockaddr* addr;
	socklen_t addrlen;
	WSADATA WSAData;
} connection_t;
#elif __unix__
#include <sys/socket.h>
#define LPTSTR char*
#define LPCTSTR const char*
#define INVALID_SOCKET -1
#define SOCKET_ERROR  -1
typedef struct {
	int socket;
	struct sockaddr* addr;
	socklen_t addrlen;
} connection_t;
#endif

/**/

int connection_init(connection_t* connection, LPCTSTR address, const uint16_t port);

/* Close connection return 1 and set properly errno on error */

extern int connection_close(const connection_t* connection);

extern int connetcion_connect(const connection_t* connection);

/* Get a malloc'd buffer wich contain a received message return number of byte read or -1 on error*/
extern int connection_recv(const connection_t* connection, LPTSTR* buff);

/**/
extern int connection_send(const connection_t* connection, LPCTSTR buff);

#ifdef __unix__

/* Initiazlize connection return 1 and set properly errno on error */

extern int connection_listen(connection_t*);

/* Get an accepted connection return 1 and set properly errno on error */

extern int connection_get_accepted(const connection_t*, connection_t*);

#endif
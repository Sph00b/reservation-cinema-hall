#pragma once
#ifdef _WIN32
	#pragma comment (lib, "Ws2_32.lib")
	#define WIN32_LEAN_AND_MEAN
	
	#ifndef _WINSOCKAPI_
		#define _WINSOCKAPI_
	#endif
	
	#include <Windows.h>
	#include <WinSock2.h>
	#include <WS2tcpip.h>
#endif

#include <stdint.h>

typedef void* connection_t;

#ifdef _WIN32
/**/

extern connection_t connection_init(LPCTSTR address, const uint16_t port);

/* Close connection return 1 and set properly errno on error */

extern int connection_close(const connection_t handle);

extern int connetcion_connect(const connection_t handle);

/* Get a malloc'd buffer wich contain a received message return number of byte read or -1 on error*/
extern int connection_recv(const connection_t handle, LPTSTR* buff);

/**/
extern int connection_send(const connection_t handle, LPCTSTR buff);

#elif __unix__
/**/

extern connection_t connection_init(const char* address, const uint16_t port);

/* Close connection return 1 and set properly errno on error */

extern int connection_close(const connection_t handle);

extern int connetcion_connect(const connection_t handle);

/* Get a malloc'd buffer wich contain a received message return number of byte read or -1 on error*/
extern int connection_recv(const connection_t handle, char** buff);

/* return number of bytes sended */

extern int connection_send(const connection_t handle, const char* buff);

/* Initiazlize connection return 1 and set properly errno on error */

extern int connection_listen(const connection_t handle);

/* Get an accepted connection return 1 and set properly errno on error */

extern connection_t connection_accepted(const connection_t handle);

#endif
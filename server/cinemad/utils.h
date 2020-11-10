#pragma once

#include <unistd.h>

#ifndef SYSV
#define SYSV
#endif

#ifdef SYSV
/*
* When a traditional SysV daemon starts, it should call this function as part 
* of the initialization. Note that this function is unnecessary for new-style 
* daemons, and should only be implemented if compatibility with SysV 
* is essential.
* 
* @return	On success sysv_daemon() returns 0. If an error occurs, 
*			sysv_daemon() returns -1 and sets errno to a proper error value.
*/
extern int sysv_daemon(void);
#define daemonize sysv_daemon
#else
#define daemonize daemon	// it probably needs a wrapper anyway
#endif

#define SIGANY -1

extern int signal_block_all(void);

extern int signal_wait(int signum);

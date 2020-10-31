#pragma once

#include <unistd.h>

#ifndef SYSV
#define SYSV
#endif

#ifdef SYSV
extern int sysv_daemon();
#define daemonize() sysv_daemon()
#else
#define daemonize() daemon(...)	// it probably needs a wrapper anyway
#endif

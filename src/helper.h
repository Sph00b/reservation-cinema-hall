#pragma once

#include <stdlib.h>
#include <stdarg.h>

#ifndef DEAMON
	#include <stdio.h>
	#define error_handler fprintf(stderr, "%m\n")
#else
	#include <syslog.h>
	#define error_handler syslog(LOG_ERR, "%m")
#endif

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		error_handler;\
		exit(EXIT_FAILURE);\
	}

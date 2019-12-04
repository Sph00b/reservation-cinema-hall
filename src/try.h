#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define try(foo, err_value)\
		if ((foo) == (err_value)){\
			fprintf(stderr, "%s\n", strerror(errno));\
			exit(EXIT_FAILURE);\
		}\


#pragma once

#include <stdint.h>

typedef struct {
	int passive_sockfd;
} connection_t;

/*	Initiazliza connection return 1 and set properly errno on error	*/

extern int connection_listener_start(connection_t*, const char*, uint16_t);

/*	Close connection return 1 and set properly errno on error */

extern int connection_listener_stop(connection_t*);

/*	Ge tfile descriptor of an accepted connection return 1 and set properly errno on error */

extern int connection_accepted_getfd(connection_t*, int*);

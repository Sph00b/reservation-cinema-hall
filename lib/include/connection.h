#pragma once

#include <stdint.h>

/* struct containing socket file descriptor and other info */

typedef int connection_t;

/* Initiazliza connection return 1 and set properly errno on error */

extern int connection_listener_start(connection_t*, const char*, const uint16_t);

/* Close connection return 1 and set properly errno on error */

extern int connection_close(const connection_t*);

/* Get an accepted connection return 1 and set properly errno on error */

extern int connection_get_accepted(const connection_t*, connection_t*);

/* Get a malloc'd buffer wich contain a received message */
extern int connection_recv(const connection_t*, char**);

/**/
extern int connection_send(const connection_t*, const char*);
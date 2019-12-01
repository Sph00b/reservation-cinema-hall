#pragma once
#include <inttypes.h>

struct cinema_config{
	char *ip;
	uint16_t port;
};

/*	Initiazliza database from file or create a new one, return 0 on success	*/
extern int database_init(char *filename);
/*	Execute a query and retrive the result, return NULL on failure	*/
extern const char* database_execute(const char *query);
extern int database_network_getconfig(struct cinema_config *);
extern const char* database_query_handler(const char *);

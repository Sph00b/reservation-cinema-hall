#pragma once

typedef void* database_t;

/*	Create database, return database handle on success or return NULL and set properly errno on error */

database_t database_init(const char *filename);

/*	Close database, return 0 on success or return 1 and set properly errno on error */

int database_close(const database_t handle);

/*	Execute the query received, set the result parameter, return 0 on success or return 1 and set properly errno on error */

int database_execute(const database_t handle, const char *query, char **result);

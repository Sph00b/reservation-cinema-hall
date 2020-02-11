#pragma once

#ifdef __unix__
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

/*
 *	query language:
 *	
 *				ADD [SECTION]
 *				ADD [KEY] FROM [SECTION]
 *				GET [KEY] FROM [SECTION]
 *				SET [KEY] FROM [SECTION] AS [VALUE]
 * */

#define DBMSG_SUCC "OPERATION SUCCEDED"
#define DBMSG_FAIL "OPERATION FAILED"
#define DBMSG_ERR "DATABASE FAILURE"

typedef struct {
    int fd;
    pthread_rwlock_t* lock;
} database_t;

/*	Initiazlize database from file return 1 and set properly errno on error	*/
extern int database_init(database_t* database, const char *filename);
/*	Close database return EOF and set properly errno on error */
extern int database_close(database_t* database);
/*	Execute a query return 1 and set properly errno on error */
extern int database_execute(database_t* database, const char *query, char **result);
#endif
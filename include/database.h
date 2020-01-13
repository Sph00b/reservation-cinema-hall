#pragma once

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

/*
 *	query language (too similar to SQL, chidish):
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
    FILE* dbstrm;	//stream to the database
    char* dbcache;	//database buffer cache
    uint8_t dbit;	//dirty bit
    pthread_mutex_t* mutex;
} database_t;

/*	Initiazliza database from file return 1 and set properly errno on error	*/
extern int database_init(database_t* database, const char *filename);
/*	Close database return EOF and set properly errno on error	*/
extern int database_close(database_t* database);
/*	Execute a query return DBMSG_ERR and set properly errno on error	*/
extern char* database_execute(database_t* database, const char *query);

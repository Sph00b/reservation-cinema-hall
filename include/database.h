#pragma once

/*
 *	query language (too similar to SQL, chidish):
 *	
 *				ADD [SECTION]
 *				ADD [KEY] FROM [SECTION]
 *				GET [KEY] FROM [SECTION]
 *				SET [KEY] FROM [SECTION] AS [VALUE]
 * */

#define DBMSG_SUCC "OPERATION SUCCEDE2D"
#define DBMSG_FAIL "OPERATION FAILED"
#define DBMSG_ERR "DATABASE FAILURE"

/*	Initiazliza database from file return 0 on success return 1 on failure and set properly errno	*/
extern int database_init(const char *filename);
/*	Close database return 0 on success	*/
extern int database_close();
/*	Execute a query and retrive the result	*/
extern char* database_execute(const char *query);

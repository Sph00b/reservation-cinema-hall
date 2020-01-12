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

/*	Initiazliza database from file return 1 and set properly errno on error	*/
extern int database_init(const char *filename);
/*	Close database return EOF and set properly errno on error	*/
extern int database_close();
/*	Execute a query return DBMSG_ERR and set properly errno on error	*/
extern char* database_execute(const char *query);

#pragma once

/*
 *	query language (too similar to SQL, chidish):
 *	
 *				ADD [SECTION]
 *				ADD [KEY] FROM [SECTION]
 *				GET [KEY] FROM [SECTION]
 *				SET [KEY] FROM [SECTION] AS [VALUE]
 * */

/*	Initiazliza database from file or create a new one, return 0 on success	*/
extern int database_init(char *filename);
/*	Execute a query and retrive the result, return NULL on failure	*/
/*	I should change it to an int method	*/
extern char* database_execute(const char *query);

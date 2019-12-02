#pragma once

/*
 *	query language:
 *				GET [VALUE] FROM [VALUE]
 *				SET [VALUE] FROM [VALUE] AS [VALUE]
 * */

/*	Initiazliza database from file or create a new one, return 0 on success	*/
extern int database_init(char *filename);
/*	Execute a query and retrive the result, return NULL on failure	*/
extern char* database_execute(const char *query);

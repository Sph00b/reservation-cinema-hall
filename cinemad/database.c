/*	Simple NoSQL Database	*/

#include "database.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#include "asprintf.h"
#include "resources.h"
#include "storage.h"

struct database {
	storage_t storage;
};

struct query {
	char key[WORDLEN + 1];
	char value[WORDLEN + 1];
};

int get_parsed_query(struct query* parsed_query, const char* query);
int	database_procedure_get(const database_t handle, const char* query, char** result);
int	database_procedure_set(const database_t handle, const char* query, char** result);
int	database_procedure_populate(const database_t handle, const char* query, char** result);
int	database_procedure_setup(const database_t handle, const char* query, char** result);
int	database_procedure_clean(const database_t handle, const char* query, char** result);
int	database_procedure_map(const database_t handle, const char* query, char** result);
int	database_procedure_book(const database_t handle, const char* query, char** result);
int	database_procedure_unbook(const database_t handle, const char* query, char** result);

database_t database_init(const char* filename) {
	struct database* database;
	if ((database = malloc(sizeof(struct database))) == NULL) {
		return NULL;
	}
	if ((database->storage = storage_init(filename)) == NULL) {
		free(database);
		return NULL;
	}
	return database;
}

/*	Close database return EOF and set properly errno on error */

int database_close(const database_t handle) {
	struct database* database = (struct database*)handle;
	storage_close(database->storage);
	free(database);
	return 0;
}

/*	Execute a query return 1 and set properly errno on error */

int database_execute(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	/*	Parse query	*/
	if (!strncmp(query, "SET ", 4)) {
		return database_procedure_set(database, query + 4, result);
	}
	else if (!strncmp(query, "GET ", 4)) {
		return database_procedure_get(database, query + 4, result);
	}
	else if (!strncmp(query, "PLT ", 4)) {
		return database_procedure_clean(database, query + 4, result);
	}
	else if (!strncmp(query, "STP ", 4)) {
		return database_procedure_clean(database, query + 4, result);
	}
	else if (!strncmp(query, "CLN ", 4)) {
		return database_procedure_clean(database, query + 4, result);
	}
	else if (!strncmp(query, "BOK ", 4)) {
		return database_procedure_book(database, query + 4, result);
	}
	else if (!strncmp(query, "DLT ", 4)) {
		return database_procedure_unbook(database, query + 4, result);
	}
	else if (!strncmp(query, "MAP ", 4)) {
		return database_procedure_map(database, query + 4, result);
	}
	else {
		*result = strdup(DBMSG_FAIL);
		return 0;
	}
}

/* return 0 on success, 1 on failure */

int get_parsed_query(struct query* parsed_query, const char* query) {
	int ret = 0;
	int ntoken = 0;
	char* buff;
	char* saveptr;
	char** token;
	memset(parsed_query->key, 0, WORDLEN + 1);
	memset(parsed_query->value, 0, WORDLEN + 1);
	if ((buff = strdup(query)) == NULL) {
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	if ((token[0] = strtok_r(buff, " ", &saveptr)) == NULL) {
		return 1;
	}
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	switch (ntoken) {
	case 1:
		if (strlen(token[0]) > WORDLEN) {
			ret = 1;
			break;
		}
		strncpy(parsed_query->key, token[0], WORDLEN + 1);
		break;
	case 3:
		if (strlen(token[0]) > WORDLEN || strlen(token[2]) > WORDLEN || strcmp(token[1], "AS")) {
			ret = 1;
			break;
		}
		strncpy(parsed_query->key, token[0], WORDLEN + 1);
		strncpy(parsed_query->value, token[2], WORDLEN + 1);
		break;
	default:
		ret = 1;
	}
	free(buff);
	free(token);
	if (ret) {
		return 1;
	}
	return 0;
}

int database_procedure_populate(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	char* msg_init[] = {
	"SET IP AS 127.0.0.1",
	"SET PORT AS 55555",
	"SET PID AS 0",
	"SET TIMESTAMP AS 0",
	"SET ROWS AS 1",
	"SET COLUMNS AS 1",
	"SET FILM AS Titolo",
	"SET SHOWTIME AS 00:00",
	"SET ID_COUNTER AS 0",
	"SET 0 AS 0",
	NULL
	};
	for (int i = 0; msg_init[i]; i++) {
		if (database_execute(database, msg_init[i], result)) {
			return 1;
		}
		free(*result);
	}
	return 0;
}

int database_procedure_setup(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int rows;
	int columns;
	int clean = 0;
	database_execute(database, "GET ROWS", result);
	rows = atoi(*result);
	free(*result);
	database_execute(database, "GET COLUMNS", result);
	columns = atoi(*result);
	free(*result);
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "GET %d", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, result) == 1) {
			return 1;
		}
		if (!strncmp(*result, DBMSG_FAIL, strlen(DBMSG_FAIL))) {
			clean = 1;
			free(query);
			free(*result);
			if (asprintf(&query, "SET %d AS 0", i) == -1) {
				return 1;
			}
			if (database_execute(database, query, result) == 1) {
				return 1;
			}
		}
		free(query);
		free(*result);
	}
	if (clean) {
		char* query = "";
		if (database_procedure_clean(database, query, result)) {
			return 1;
		}
	}
	return 0;
}

int database_procedure_get(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	struct query parsed_query;
	if (get_parsed_query(&parsed_query, query)) {
		return 1;
	}
	if (storage_lock_shared(database->storage, parsed_query.key)) {
		return 1;
	}
	if (storage_load(database->storage, parsed_query.key, result)) {
		return 1;
	}
	if (storage_unlock(database->storage, parsed_query.key)) {
		return 1;
	}
	return 0;
}

int database_procedure_set(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	struct query parsed_query;
	if (get_parsed_query(&parsed_query, query)) {
		return 1;
	}
	if (storage_lock_exclusive(database->storage, parsed_query.key)) {
		return 1;
	}
	if (storage_store(database->storage, parsed_query.key, parsed_query.value, result)) {
		return 1;
	}
	if (storage_unlock(database->storage, parsed_query.key)) {
		return 1;
	}
	return 0;
}

int database_procedure_clean(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int rows;
	int columns;
	database_execute(database, "GET ROWS", result);
	rows = atoi(*result);
	free(*result);
	database_execute(database, "GET COLUMNS", result);
	columns = atoi(*result);
	free(*result);
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "SET %d AS 0", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, result) == 1) {
			return 1;
		}
		free(query);
		free(*result);
	}
	if (database_execute(database, "SET ID_COUNTER AS 0", result) == 1) {
		return 1;
	}
	return 0;
}

int database_procedure_map(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int id = 0;
	int rows;
	int columns;
	char* tmp_query = NULL;
	char* buffer = NULL;
	char* ptr;
	id = !strlen(query) ? -1 : atoi(query);
	/*	Fetch rows and columns	*/
	database_execute(database, "GET ROWS", &buffer);
	rows = atoi(buffer);
	free(buffer);
	database_execute(database, "GET COLUMNS", &buffer);
	columns = atoi(buffer);
	free(buffer);
	if (!(rows && columns)) {
		asprintf(result, "");
		return 0;
	}
	database_execute(database, "GET 0", &buffer);
	if (atoi(buffer)) {
		if (atoi(buffer) == id) {
			free(buffer);
			asprintf(&buffer, "1");
		}
		else {
			free(buffer);
			asprintf(&buffer, "2");
		}
	}
	asprintf(result, "%s", buffer);
	for (int i = 1; i < rows * columns; i++) {
		asprintf(&tmp_query, "GET %d", i);
		database_execute(database, tmp_query, &buffer);
		ptr = &(**result);
		if (atoi(buffer)) {
			if (atoi(buffer) == id) {
				free(buffer);
				asprintf(&buffer, "1");
			}
			else {
				free(buffer);
				asprintf(&buffer, "2");
			}
		}
		asprintf(result, "%s %s", *result, buffer);
		free(ptr);
		free(tmp_query);
		free(buffer);
	}
	return 0;
}

int database_procedure_book(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int id;
	int ret;
	int ntoken;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;
	char** rquery = NULL;
	char** wquery = NULL;

	ntoken = 0;
	if ((buffer = strdup(query)) == NULL) {
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	if (!strcmp(token[0], "0")) {
		char* id_query;
		char* result;
		if (database_execute(database, "GET ID_COUNTER", &result) == 1) {
			return -1;
		}
		id = atoi(result);
		id++;
		free(result);
		if (asprintf(&id_query, "SET ID_COUNTER AS %d", id) == -1) {
			return -1;
		}
		if (database_execute(database, id_query, &result) == 1) {
			free(id_query);
			return -1;
		}
		free(id_query);
		free(result);
		if (id == -1) {
			return 1;
		}
	}
	else {
		id = atoi(token[0]);
	}
	ntoken--;
	/*	free memory on error	*/
	if ((rquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	if ((wquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	for (int i = 0; i < ntoken; i++) {
		if (asprintf(&(rquery[i]), "GET %s", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s AS %d", token[i + 1], id) == -1) {
			return 1;
		}
	}
	free(buffer);
	free(token);
	ret = 0;
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, rquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
		if (strcmp(*result, "0")) {
			ret = 1;
		}
		free(rquery[i]);
		free(*result);
		if (ret) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
	}
	free(rquery);
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, wquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(wquery[++j]));
			free(wquery);
			return 1;
		}
		free(wquery[i]);
		free(*result);
	}
	free(wquery);
	if (asprintf(result, "%d", id) == -1) {
		return 1;
	}
	return 0;
}

int database_procedure_unbook(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int ret;
	int ntoken;
	char* id;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;
	char** rquery = NULL;
	char** wquery = NULL;

	ntoken = 0;
	if ((buffer = strdup(query)) == NULL) {
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	id = strdup(*token);
	ntoken--;
	/*	free memory on error	*/
	if ((rquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	if ((wquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	for (int i = 0; i < ntoken; i++) {
		if (asprintf(&(rquery[i]), "GET %s", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s AS 0", token[i + 1]) == -1) {
			return 1;
		}
	}
	free(buffer);
	free(token);
	ret = 0;
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, rquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
		if (strcmp(*result, id)) {
			ret = 1;
		}
		free(rquery[i]);
		free(*result);
		if (ret) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			free(id);
			return 1;
		}
	}
	free(rquery);
	free(id);
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, wquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(wquery[++j]));
			free(wquery);
			return 1;
		}
		free(wquery[i]);
		free(*result);
	}
	free(wquery);
	if (asprintf(result, DBMSG_SUCC) == -1) {
		return 1;
	}
	return 0;
}

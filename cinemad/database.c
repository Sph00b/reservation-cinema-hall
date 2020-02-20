/*	Simple NoSQL Database	*/

#include "database.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#include "asprintf.h"
#include "resources.h"
#include "index_table.h"

struct database {
	int fd;
	index_table_t index_table;
};

struct query {
	char key[WORDLEN + 1];
	char value[WORDLEN + 1];
};

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

int database_read(FILE** strm, char** result);
int database_write(FILE** strm, char* data);
int database_get_stream(FILE** pstrm, int fd);
int get_parsed_query(struct query* parsed_query, const char* query);
int database_procedure_get(const database_t handle, FILE** stream, const char* query, char** result);
int database_procedure_set(const database_t handle, FILE** stream, const char* query, char** result);
int database_procedure_configure(const database_t handle, FILE** stream);
int database_procedure_clean(const database_t handle, FILE** stream, const char* query, char** result);
int database_procedure_map(const database_t handle, FILE** stream, const char* query, char** result);
int database_procedure_book(const database_t handle, FILE** stream, const char* query, char** result);
int database_procedure_unbook(const database_t handle, FILE** stream, const char* query, char** result);

database_t database_init(const char* filename) {
	struct database* database;
	FILE* stream;
	int new_database = 0;
	if ((database = malloc(sizeof(struct database))) == NULL) {
		return NULL;
	}
	if ((database->fd = open(filename, O_RDWR, 0666)) == -1) {
		if (errno != ENOENT) {
			return NULL;
		}
		if ((database->fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1) {
			return NULL;
		}
		new_database = 1;
	}
	if (flock(database->fd, LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		close(database->fd);
		free(database);
		return NULL;
	}
	if ((database->index_table = index_table_init()) == NULL) {
		close(database->fd);
		free(database);
		return NULL;
	}
	if (database_get_stream(&stream, database->fd)) {
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (fseek(stream, 0, SEEK_SET) == -1) {
		fclose(stream);
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (index_table_wrlock(database->index_table)) {
		fclose(stream);
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (index_table_load(database->index_table, (void**)&stream)) {
		fclose(stream);
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (index_table_unlock(database->index_table)) {
		fclose(stream);
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (database_procedure_configure(database, &stream)) {
		fclose(stream);
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (fclose(stream) == EOF) {
		index_table_destroy(database->index_table);
		close(database->fd);
		free(database);
		return NULL;
	}
	if (new_database) {
		char* result;
		for (int i = 0; msg_init[i]; i++) {
			if (database_execute(database, msg_init[i], &result)) {
				free(result);
				index_table_destroy(database->index_table);
				free(database);
				remove(filename);
				return NULL;
			}
			free(result);
		}
	}
	return database;
}

/*	Close database return EOF and set properly errno on error */

int database_close(const database_t handle) {
	struct database* database = (struct database*)handle;
	index_table_destroy(database->index_table);
	close(database->fd);
	free(database);
	return 0;
}

/*	Execute a query return 1 and set properly errno on error */

int database_execute(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int ret = 0;
	FILE* stream;
	int stream_open = 0;
	if (strlen(query) < 4) {
		ret = 1;
	}
	else {
		ret = database_get_stream(&stream, database->fd);
	}
	if (!ret) {
		stream_open = 1;
		if (!strncmp(query, "SET ", 4)) {
			ret = database_procedure_set(database, &stream, query + 4, result);
		}
		else if (!strncmp(query, "GET ", 4)) {
			ret = database_procedure_get(database, &stream, query + 4, result);
		}
		else if (!strncmp(query, "CLN ", 4)) {
			ret = database_procedure_clean(database, &stream, query + 4, result);
		}
		else if (!strncmp(query, "BOK ", 4)) {
			ret = database_procedure_book(database, &stream, query + 4, result);
		}
		else if (!strncmp(query, "DLT ", 4)) {
			ret = database_procedure_unbook(database, &stream, query + 4, result);
		}
		else if (!strncmp(query, "MAP ", 4)) {
			ret = database_procedure_map(database, &stream, query + 4, result);
		}
		else {
			*result = strdup(DBMSG_FAIL);
		}
	}
	if (stream_open) {
		if (fclose(stream) == EOF) {
			if (!ret) {
				free(*result);
			}
			ret = 1;
		}
	}
	if (ret) {
		*result = strdup(DBMSG_ERR);
		return 1;
	}
	return 0;
}

/* return 0 on success, 1 on failure */

int database_read(FILE** strm, char** result) {
	for (int i = 0; i < WORDLEN; i++) {
		if ((int)((*result)[i] = (char)fgetc(*strm)) == EOF) {
			return 1;
		}
	}
	(*result)[WORDLEN] = 0;
	return 0;
}

/* return 0 on success, 1 on failure */

int database_write(FILE** strm, char* data) {
	for (int i = 0; i < WORDLEN; i++) {
		if (fputc(data[i], *strm) == EOF) {
			return 1;
		}
	}
	fflush(*strm);
	return 0;
}

/*	return 0 on success, 1 on sys failure	*/

int database_get_stream(FILE** pstrm, int fd) {
	int newfd;
	while ((newfd = dup(fd)) == -1) {
		if (errno != EMFILE) {
			return 1;
		}
		sleep(1);
	}
	if ((*pstrm = fdopen(newfd, "r+")) == NULL) {
		return 1;
	}
	return 0;
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

int database_procedure_get(const database_t handle, FILE** stream, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	struct query parsed_query;
	if (get_parsed_query(&parsed_query, query)) {
		/*	What should I do?	*/
		*result = strdup(DBMSG_FAIL);
		return 0;
	}
	if (index_table_rdlock(database->index_table)) {
		return 1;
	}
	if (index_table_record_locate(database->index_table, parsed_query.key, (void**)stream)) {
		if (index_table_unlock(database->index_table)) {
			return 1;
		}
		*result = strdup(DBMSG_FAIL);
		return 0;
	}
	if ((*result = malloc(sizeof(char) * WORDLEN + 1)) == NULL) {
		return 1;
	}
	if (index_table_record_rdlock(database->index_table, parsed_query.key)) {
		free(*result);
		return 1;
	}
	if (database_read(stream, result)) {
		free(*result);
		return 1;
	}
	if (index_table_record_unlock(database->index_table, parsed_query.key)) {
		free(*result);
		return 1;
	}
	if (index_table_unlock(database->index_table)) {
		free(*result);
		return 1;
	}
	return 0;
}

int database_procedure_set(const database_t handle, FILE** stream, const char* query, char** result) {
	struct database* database = (struct database*)handle;
	int key_found;
	struct query parsed_query;
	if (get_parsed_query(&parsed_query, query)) {
		/*	What should I do?	*/
		*result = strdup(DBMSG_FAIL);
		return 0;
	}
	if (index_table_rdlock(database->index_table)) {
		return 1;
	}
	if ((key_found = index_table_record_locate(database->index_table, parsed_query.key, (void**)stream))) {
		if (index_table_unlock(database->index_table)) {
			return 1;
		}
		if (index_table_wrlock(database->index_table)) {
			return 1;
		}
		if ((key_found = index_table_record_locate(database->index_table, parsed_query.key, (void**)stream))) {
			if (fseek(*stream, 0, SEEK_END) == -1) {
				return 1;
			}
			if (database_write(stream, parsed_query.key)) {
				return 1;
			}
			if (database_write(stream, parsed_query.value)) {
				return 1;
			}
			if (fseek(*stream, -2 * WORDLEN, SEEK_END) == -1) {
				return 1;
			}
			if (index_table_update(database->index_table, (void**)stream)) {
				return 1;
			}
		}
	}
	if (!key_found) {
		if (index_table_record_wrlock(database->index_table, parsed_query.key)) {
			return 1;
		}
		if (database_write(stream, parsed_query.value)) {
			return 1;
		}
		if (index_table_record_unlock(database->index_table, parsed_query.key)) {
			return 1;
		}
	}
	if (index_table_unlock(database->index_table)) {
		return 1;
	}
	*result = strdup(DBMSG_SUCC);
	return 0;
}

int database_procedure_configure(const database_t handle, FILE** stream) {
	struct database* database = (struct database*)handle;
	int rows;
	int columns;
	int clean = 0;
	char* result;
	database_execute(database, "GET ROWS", &result);
	rows = atoi(result);
	free(result);
	database_execute(database, "GET COLUMNS", &result);
	columns = atoi(result);
	free(result);
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "GET %d", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, &result) == 1) {
			return 1;
		}
		if (!strncmp(result, DBMSG_FAIL, strlen(DBMSG_FAIL))) {
			clean = 1;
			free(query);
			free(result);
			if (asprintf(&query, "SET %d AS 0", i) == -1) {
				return 1;
			}
			if (database_execute(database, query, &result) == 1) {
				return 1;
			}
		}
		free(query);
		free(result);
	}
	if (clean) {
		char* query = "";
		char* result;
		if (database_procedure_clean(database, stream, query, &result)) {
			return 1;
		}
		free(result);
	}
	return 0;
}

int database_procedure_clean(const database_t handle, FILE** stream, const char* query, char** result) {
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

int database_procedure_map(const database_t handle, FILE** stream, const char* query, char** result) {
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

int database_procedure_book(const database_t handle, FILE** stream, const char* query, char** result) {
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

int database_procedure_unbook(const database_t handle, FILE** stream, const char* query, char** result) {
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
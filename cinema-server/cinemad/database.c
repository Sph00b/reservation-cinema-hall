/*	Simple NoSQL Database	*/

#include "database.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#include <resources.h>
#include <try.h>

#include "storage.h"

struct cinema_info {
	int rows;
	int columns;
};

struct database {
	storage_t storage;
	struct cinema_info cinema_info;
};

/*	Prototype declarations of functions included in this code module	*/

static int parse_query(const char* query, char*** parsed);
static int procedure_populate(const database_t handle, char** result);
static int procedure_setup(const database_t handle, char** result);
static int procedure_clean(const database_t handle, char** result);
static int procedure_get_id(const database_t handle, char** result);
static int procedure_get(const database_t handle, char** query, char** result);
static int procedure_set(const database_t handle, char** query, char** result);
static int procedure_map(const database_t handle, char** query, char** result);
static int procedure_book(const database_t handle, char** query, char** result);
static int procedure_unbook(const database_t handle, char** query, char** result);

extern database_t database_init(const char* filename) {
	struct database* database;
	database = calloc(1, sizeof * database);
	if (database) {
		try(database->storage = storage_init(filename), NULL, error);
		database->cinema_info.columns = 0;
		database->cinema_info.rows = 0;
	}
	return database;

error:
	free(database);
	return NULL;
}

extern int database_close(const database_t handle) {
	struct database* database = (struct database*)handle;

	try(storage_close(database->storage), 1, error);
	free(database);
	return 0;

error:
	return 1;
}

extern int database_execute(const database_t handle, const char* query, char** result) {
	struct database* database = (struct database*)handle;

	int ret;
	int argc;
	char** argv;
	if ((argc = parse_query(query, &argv)) == -1) {
		return 1;
	}
	else if (argc == 1 && !strcmp(argv[0], "POPULATE")) {
		ret = procedure_populate(database, result);
	}
	else if (argc == 1 && !strcmp(argv[0], "SETUP")) {
		ret = procedure_setup(database, result);
	}
	else if (argc == 1 && !strcmp(argv[0], "CLEAN")) {
		ret = procedure_clean(database, result);
	}
	else if (argc == 1 && !strcmp(argv[0], "ID")) {
		ret = procedure_get_id(database, result);
	}
	else if (argc == 2 && !strcmp(argv[0], "GET")) {
		ret = procedure_get(database, &(argv[1]), result);
	}
	else if (argc == 3 && !strcmp(argv[0], "SET")) {
		ret = procedure_set(database, &(argv[1]), result);
	}
	else if (argc == 2 && !strcmp(argv[0], "MAP")) {
		ret = procedure_map(database, &(argv[1]), result);
	}
	else if (argc > 2 && !strcmp(argv[0], "BOOK")) {
		ret = procedure_book(database, &(argv[1]), result);
	}
	else if (argc > 2 && !strcmp(argv[0], "DELETE")) {
		ret = procedure_unbook(database, &(argv[1]), result);
	}
	else {
		*result = strdup(MSG_FAIL);
		ret = 0;
	}
	free(*argv);
	free(argv);
	return ret;
}

/*
* Tokenize the received query in a string vector saved in parsed parameter.
* 
* @return	the number of token on success or return -1 and set properly 
*			errno on error
*/
static int parse_query(const char* query, char*** parsed) {
	int ntoken;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;

	try(buffer = strdup(query), NULL, error);
	try(token = malloc(sizeof * token), NULL, cleanup);
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken = 1;
	while (token[ntoken - 1] != NULL) {
		ntoken++;
		try(token = realloc(token, sizeof * token * (size_t)ntoken), NULL, cleanup);
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	}
	*parsed = token;
	return ntoken - 1;

cleanup:
	free(buffer);
error:
	return -1;
}

/*
* Populate the database executing a predefined set of query
*/
static int procedure_populate(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;

	char* msg_init[] = {
	"SET IP 127.0.0.1",
	"SET PORT 55555",
	"SET PID 0",
	"SET TIMESTAMP 0",
	"SET ROWS 1",
	"SET COLUMNS 1",
	"SET FILM Titolo",
	"SET SHOWTIME 00:00",
	"SET ID_COUNTER 0",
	"SET 0 0",
	NULL
	};
	for (int i = 0; msg_init[i]; i++) {
		try(database_execute(database, msg_init[i], result), !0, error);
		free(*result);
	}
	*result = strdup(MSG_SUCC);
	return 0;

error:
	return 1;
}

/*
* Setup the database and the database info
*/
static int procedure_setup(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;

	try(database_execute(database, "GET ROWS", result), 1, error);
	try(strtoi(*result, &database->cinema_info.rows), !0, cleanup);
	free(*result);
	try(database_execute(database, "GET COLUMNS", result), 1, error);
	try(strtoi(*result, &database->cinema_info.columns), !0, cleanup);
	free(*result);

	int clean = 0;
	int rows = database->cinema_info.rows;
	int cols = database->cinema_info.columns;
	for (int i = 0; i < rows * cols; i++) {
		char* query;
		try(asprintf(&query, "GET %d", i), -1, error);
		try(database_execute(database, query, result), 1, error);
		if (!strncmp(*result, MSG_FAIL, strlen(MSG_FAIL))) {
			clean = 1;
			free(query);
			free(*result);
			try(asprintf(&query, "SET %d 0", i), -1, error);
			try(database_execute(database, query, result), 1, error);
		}
		free(query);
		free(*result);
	}
	if (clean) {
		try(procedure_clean(database, result), !0, error);
		free(*result);
	}
	*result = strdup(MSG_SUCC);
	return 0;

cleanup:
	free(*result);
error:
	return 1;
}

/*
* Discard all booking
*/
static int procedure_clean(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;

	char* tmp;
	char* buffer;
	char** query;
	int rows = database->cinema_info.rows;
	int cols = database->cinema_info.columns;

	for (int i = 0; i < rows * cols; i++) {
		try(asprintf(&tmp, "%d 0", i), -1, error);
		try(parse_query(tmp, &query), -1, cleanup1);
		try(procedure_set(database, query, &buffer), !0, cleanup2);
		free(*query);
		free(query);
		free(tmp);
		free(buffer);
	}
	try(parse_query("ID_COUNTER 0", &query), -1, error);
	try(procedure_set(database, query, result), !0, cleanup3);
	free(*query);
	free(query);
	return 0;

cleanup3:
	free(*query);
	free(query);
	return 1;
cleanup2:
	free(*query);
	free(query);
cleanup1:
	free(tmp);
error:
	return 1;
}

/*
* Get a valid ID for a booking
*/
static int procedure_get_id(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;
	int current_id;
	char* new_id;
	char* buffer;

	try(storage_lock_exclusive(database->storage, "ID_COUNTER"), !0, error);
	try(storage_load(database->storage, "ID_COUNTER", &buffer), !0, error);
	try(strtoi(buffer, &current_id), !0, cleanup1);
	free(buffer);
	try(asprintf(&new_id, "%d", current_id + 1), -1, error);
	try(storage_store(database->storage, "ID_COUNTER", new_id, &buffer), !0, cleanup2);
	*result = strdup(new_id);
	free(new_id);
	free(buffer);
	try(storage_unlock(database->storage, "ID_COUNTER"), !0, error);
	return 0;

cleanup2:
	free(new_id);
	return 1;
cleanup1:
	free(buffer);
error:
	return 1;
}

/*
* Standard get procedure
*/
static int procedure_get(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	try(storage_lock_shared(database->storage, query[0]), !0, error);
	try(storage_load(database->storage, query[0], result), !0, error);
	try(storage_unlock(database->storage, query[0]), !0, error);
	return 0;
	
error:
	return 1;
}

/*
* Standard set procedure
*/
static int procedure_set(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	try(storage_lock_exclusive(database->storage, query[0]), !0, error);
	try(storage_store(database->storage, query[0], query[1], result), !0, error);
	try(storage_unlock(database->storage, query[0]), !0, error);
	return 0;

error:
	return 1;
}

/*
* Return the seats status map
*/
static int procedure_map(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	int n_seats;
	int id;
	char* map = NULL;

	n_seats = database->cinema_info.rows * database->cinema_info.columns;
	try(strtoi(query[0], &id), !0, fail);
	if (!n_seats) {
		goto fail;
	}
	try(map = malloc(sizeof * map * (size_t)n_seats * 2), NULL, error);
	memset(map, 0, (size_t)(n_seats * 2));

	int book_id;
	char* tmp;
	char* buffer;
	for (int i = 0; i < n_seats; i++) {
		try(asprintf(&tmp, "%d", i), -1, error);
		try(procedure_get(database, &tmp, &buffer), !0, error);
		free(tmp);
		try(strtoi(buffer, &book_id), !0, fail_cleanup);
		if (book_id) {
			free(buffer);
			if (book_id == id) {
				asprintf(&buffer, "1");
			}
			else {
				asprintf(&buffer, "2");
			}
		}
		map[2 * i] = buffer[0];
		map[(2 * i) + 1] = ' ';
		free(buffer);
	}
	map[(n_seats * 2) - 1] = 0;
	*result = map;
	return 0;

fail_cleanup:
	free(buffer);
fail:
	*result = strdup(MSG_FAIL);
	return 0;
error:
	return 1;
}

/*
* Return the ID on a successful operation
*/
static int procedure_book(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;

	char* id = query[0];
	char** ordered_request;
	size_t n_seats = 0;

	// order request to avoid deadlock
	// TODO: This should be extracted-------------------------------------------
	size_t* tmp;	//tmp variable to order request
	size_t rows = (size_t)database->cinema_info.rows;
	size_t cols = (size_t)database->cinema_info.columns;

	while (query[n_seats + 1]) {
		n_seats++;
	}
	try(tmp = calloc(1, sizeof * tmp * rows * cols), NULL, error);
	for (int i = 0; i < n_seats; i++) {
		int seat;
		try(strtoi(query[i + 1], &seat), !0, fail_cleanup);
		tmp[seat] = 1;
	}
	try(ordered_request = malloc(sizeof * ordered_request * n_seats), NULL, error);
	int n_req = 0;
	for (int i = 0; (i < rows * cols) && (n_req < n_seats); i++) {
		if (tmp[i]) {
			try(asprintf(&(ordered_request[n_req]), "%d", i), -1, error);
			n_req++;
		}
	}
	free(tmp);
	if (n_req != n_seats) {
		goto fail;
	}
	//--------------------------------------------------------------------------

	// 2PL locking
	char* seat_id;
	for (size_t i = 0; i < n_seats; i++) {
		try(storage_lock_exclusive(database->storage, ordered_request[i]), !0, error);
	}
	for (size_t i = 0; i < n_seats; i++) {
		try(storage_load(database->storage, ordered_request[i], &seat_id), !0, error);
		if (strcmp(seat_id, "0")) {	// TODO: implement in a self explaining named macro
			goto fail_unlock;
		}
		free(seat_id);
	}
	if (!strcmp(id, "-1")) {
		try(database_execute(database, "ID", &id), !0, error);
	}
	for (int i = 0; i < n_seats; i++) {
		char* buffer;
		try(storage_store(database->storage, ordered_request[i], id, &buffer), !0, error);
		free(buffer);
	}
	*result = strdup(id);
	for (int i = 0; i < n_seats; i++) {
		try(storage_unlock(database->storage, ordered_request[i]), !0, error);
	}
	for (int i = 0; i < n_seats; i++) {
		free(ordered_request[i]);
	}
	free(ordered_request);
	return 0;

fail_unlock:
	free(seat_id);
	for (size_t i = 0; i < n_seats; i++) {
		storage_unlock(database->storage, ordered_request[i]);
	}
	goto fail;
fail_cleanup:
	free(tmp);
fail:
	*result = strdup(MSG_FAIL);
	return 0;
error:
	return 1;
}

/*
* Remove a booking
*/
static int procedure_unbook(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	char* id = query[0];

	char* buffer;
	for (int i = 1; query[i]; i++) {
		try(procedure_get(database, &(query[i]), &buffer), !0, error);
		try(strcmp(id, buffer), !0, fail);
		free(buffer);
	}

	for (int i = 1; query[i]; i++) {
		char** parsed_query;
		char* tmp_query;
		char* buffer;
		try(asprintf(&tmp_query, "%s 0", query[i]), -1, error);
		try(parse_query(tmp_query, &parsed_query), -1, error);
		try(procedure_set(database, parsed_query, &buffer), !0, error);
		free(*parsed_query);
		free(parsed_query);
		free(tmp_query);
		free(buffer);
	}
	*result = strdup(MSG_SUCC);
	return 0;

fail:
	free(buffer);
	*result = strdup(MSG_FAIL);
	return 0;
error:
	return 1;
}

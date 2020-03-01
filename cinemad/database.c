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
#include "storage.h"

struct cinema_info {
	int rows;
	int columns;
};

struct database {
	storage_t storage;
	struct cinema_info cinema_info;
};

//static int parse_query(struct query* parsed_query, const char* query);
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

database_t database_init(const char* filename) {
	struct database* database;
	if ((database = malloc(sizeof(struct database))) == NULL) {
		return NULL;
	}
	if ((database->storage = storage_init(filename)) == NULL) {
		free(database);
		return NULL;
	}
	database->cinema_info.columns = 0;
	database->cinema_info.rows = 0;
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
	int ret;
	int n_param;
	char** parsed_query;
	if ((n_param = parse_query(query, &parsed_query)) == -1) {
		return 1;
	}
	else if (n_param == 1 && !strcmp(parsed_query[0], "POPULATE")) {
		ret = procedure_populate(database, result);
	}
	else if (n_param == 1 && !strcmp(parsed_query[0], "SETUP")) {
		ret = procedure_setup(database, result);
	}
	else if (n_param == 1 && !strcmp(parsed_query[0], "CLEAN")) {
		ret = procedure_clean(database, result);
	}
	else if (n_param == 1 && !strcmp(parsed_query[0], "ID")) {
		ret = procedure_get_id(database, result);
	}
	else if (n_param == 2 && !strcmp(parsed_query[0], "GET")) {
		ret = procedure_get(database, &(parsed_query[1]), result);
	}
	else if (n_param == 3 && !strcmp(parsed_query[0], "SET")) {
		ret = procedure_set(database, &(parsed_query[1]), result);
	}
	else if (n_param == 2 && !strcmp(parsed_query[0], "MAP")) {
		ret = procedure_map(database, &(parsed_query[1]), result);
	}
	else if (n_param > 2 && !strcmp(parsed_query[0], "BOOK")) {
		ret = procedure_book(database, &(parsed_query[1]), result);
	}
	else if (n_param > 2 && !strcmp(parsed_query[0], "DELETE")) {
		ret = procedure_unbook(database, &(parsed_query[1]), result);
	}
	else {
		*result = strdup(MSG_FAIL);
		ret = 0;
	}
	free(*parsed_query);
	free(parsed_query);
	return ret;
}

static int parse_query(const char* query, char*** parsed) {
	int ntoken;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;

	ntoken = 0;
	if ((buffer = strdup(query)) == NULL) {
		return -1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return -1;
	}
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return -1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	*parsed = token;
	return ntoken;
}

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
	"SET ID_COUNTER 1",
	"SET 0 0",
	NULL
	};
	for (int i = 0; msg_init[i]; i++) {
		if (database_execute(database, msg_init[i], result)) {
			return 1;
		}
		free(*result);
	}
	*result = strdup(MSG_SUCC);
	return 0;
}

static int procedure_setup(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;
	int clean = 0;

	database_execute(database, "GET ROWS", result);
	database->cinema_info.rows = atoi(*result);
	free(*result);
	database_execute(database, "GET COLUMNS", result);
	database->cinema_info.columns = atoi(*result);
	free(*result);
	for (int i = 0; i < database->cinema_info.rows * database->cinema_info.columns; i++) {
		char* query;
		if (asprintf(&query, "GET %d", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, result) == 1) {
			return 1;
		}
		if (!strncmp(*result, MSG_FAIL, strlen(MSG_FAIL))) {
			clean = 1;
			free(query);
			free(*result);
			if (asprintf(&query, "SET %d 0", i) == -1) {
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
		if (procedure_clean(database, result)) {
			return 1;
		}
		free(*result);
	}
	*result = strdup(MSG_SUCC);
	return 0;
}

static int procedure_clean(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;
	char** parsed_query;
	for (int i = 0; i < database->cinema_info.rows * database->cinema_info.columns; i++) {
		char* tmp_query;
		char* buffer;
		if (asprintf(&tmp_query, "%d 0", i) == -1) {
			return 1;
		}
		if (parse_query(tmp_query, &parsed_query) == -1) {
			return 1;
		}
		if (procedure_set(database, parsed_query, &buffer)) {
			return 1;
		}
		free(*parsed_query);
		free(parsed_query);
		free(tmp_query);
		free(buffer);
	}
	if (parse_query("ID_COUNTER 0", &parsed_query) == -1) {
		return 1;
	}
	if (procedure_set(database, parsed_query, result)) {
		return 1;
	}
	free(*parsed_query);
	free(parsed_query);
	return 0;
}

static int procedure_get_id(const database_t handle, char** result) {
	struct database* database = (struct database*)handle;
	if (storage_lock_exclusive(database->storage, "ID_COUNTER")) {
		return 1;
	}
	if (storage_load(database->storage, "ID_COUNTER", result)) {
		return 1;
	}
	char* new_id;
	char* buffer;
	if (asprintf(&new_id, "%d", atoi(*result) + 1) == -1) {
		return 1;
	}
	if (storage_store(database->storage, "ID_COUNTER", new_id, &buffer)) {
		return 1;
	}
	free(new_id);
	free(buffer);
	if (storage_unlock(database->storage, "ID_COUNTER")) {
		return 1;
	}
	return 0;
}

static int procedure_get(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	if (storage_lock_shared(database->storage, query[0])) {
		return 1;
	}
	if (storage_load(database->storage, query[0], result)) {
		return 1;
	}
	if (storage_unlock(database->storage, query[0])) {
		return 1;
	}
	return 0;
}

static int procedure_set(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	if (storage_lock_exclusive(database->storage, query[0])) {
		return 1;
	}
	if (storage_store(database->storage, query[0], query[1], result)) {
		return 1;
	}
	if (storage_unlock(database->storage, query[0])) {
		return 1;
	}
	return 0;
}

static int procedure_map(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	int n_seats;
	int id;
	char* map = NULL;

	n_seats = database->cinema_info.rows * database->cinema_info.columns;
	id = atoi(query[0]);
	if (!n_seats) {
		*result = strdup("");
		return 0;
	}
	if ((map = malloc(sizeof(char) * (size_t)(n_seats * 2))) == NULL) {
		return 1;
	}
	memset(map, 0, (size_t)(n_seats * 2));
	for (int i = 0; i < n_seats; i++) {
		char* tmp_query;
		char* buffer;
		if (asprintf(&tmp_query, "%d", i) == -1) {
			return 1;
		}
		if (procedure_get(database, &tmp_query, &buffer)) {
			return 1;
		}
		free(tmp_query);
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
		map[2 * i] = buffer[0];
		map[(2 * i) + 1] = ' ';
		free(buffer);
	}
	map[(n_seats * 2) - 1] = 0;
	*result = map;
	return 0;
}

static int procedure_book(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	int n_seats;
	char* id = query[0];
	char** ordered_request;
	int* tmp;	//tmp variable to order request
	int abort = 0;

	/*	order request to avoid deadlock	*/
	for (n_seats = 0; query[n_seats + 1]; n_seats++);	//get n_seats
	if ((ordered_request = malloc(sizeof(char*) * (size_t)n_seats)) == NULL) {
		return 1;
	}
	if ((tmp = malloc(sizeof(int) * (size_t)(database->cinema_info.rows * database->cinema_info.columns))) == NULL) {
		return 1;
	}
	memset(tmp, 0, sizeof(int) * (size_t)(database->cinema_info.rows * database->cinema_info.columns));
	for (int i = 0; i < n_seats; i++) {
		tmp[atoi(query[i + 1])] = 1;
	}
	int n_tmp = 0;
	for (int i = 0; i < database->cinema_info.rows * database->cinema_info.columns; i++) {
		if (tmp[i]) {
			asprintf(&(ordered_request[n_tmp++]), "%d", i);
			if (n_tmp == n_seats) {
				break;
			}
		}
	}
	free(tmp);
	if (n_tmp != n_seats) {
		*result = strdup(MSG_FAIL);
		return 0;
	}
	/*	2PL locking	*/
	for (int i = 0; i < n_seats; i++) {
		if (storage_lock_exclusive(database->storage, ordered_request[i])) {
			return 1;
		}
	}
	char* seat_id;
	for (int i = 0; i < n_seats; i++) {
		if (storage_load(database->storage, ordered_request[i], &seat_id)) {
			return 1;
		}
		if (atoi(seat_id)) {
			free(seat_id);
			abort = 1;
			break;
		}
		free(seat_id);
	}
	if (!abort) {
		if (!strcmp(id, "-1")) {
			if (database_execute(database, "ID", &id)) {
				return 1;
			}
		}
		for (int i = 0; i < n_seats; i++) {
			char* buffer;
			if (storage_store(database->storage, ordered_request[i], id, &buffer)) {
				return 1;
			}
			free(buffer);
		}
		*result = strdup(id);
	}
	else {
		*result = strdup(MSG_FAIL);
	}
	for (int i = 0; i < n_seats; i++) {
		if (storage_unlock(database->storage, ordered_request[i])) {
			return 1;
		}
	}
	for (int i = 0; i < n_seats; i++) {
		free(ordered_request[i]);
	}
	free(ordered_request);
	return 0;
}

static int procedure_unbook(const database_t handle, char** query, char** result) {
	struct database* database = (struct database*)handle;
	char* id = query[0];
	
	for (int i = 1; query[i]; i++) {
		char* buffer;
		if (procedure_get(database, &(query[i]), &buffer)) {
			return 1;
		}
		if (strcmp(id, buffer)) {
			free(buffer);
			*result = strdup(MSG_FAIL);
			return 0;
		}
		free(buffer);
	}
	for (int i = 1; query[i]; i++) {
		char** parsed_query;
		char* tmp_query;
		char* buffer;
		if (asprintf(&tmp_query, "%s 0", query[i]) == -1) {
			return 1;
		}
		if (parse_query(tmp_query, &parsed_query) == -1) {
			return 1;
		}
		if (procedure_set(database, parsed_query, &buffer)) {
			return 1;
		}
		free(*parsed_query);
		free(parsed_query);
		free(tmp_query);
		free(buffer);
	}
	*result = strdup(MSG_SUCC);
	return 0;
}

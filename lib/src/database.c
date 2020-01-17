#include "database.h"

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#define WORDLEN 16

#define LOCK(mutex, ret)\
			while ((ret = pthread_mutex_lock(mutex)) && errno == EINTR);\
			if (ret) {\
				return 1;\
			}

#define UNLOCK(mutex, ret)\
			while ((ret = pthread_mutex_unlock(mutex)) && errno == EINTR);\
			if (ret) {\
				return 1;\
			}


/*	IDK how to name
	undefined behaviour if the strings pointed are longer than WORDLEN
*/

struct info {
	char* section;
	char* key;
	char* value;
};

int refresh_cache(database_t* database) {
	int len;
	if (database->dbit) {
		if (fseek(database->dbstrm, 0, SEEK_END) == -1) {
			return 1;
		}
		if ((len = ftell(database->dbstrm)) == -1) {
			return 1;
		}
		if (fseek(database->dbstrm, 0, SEEK_SET) == -1) {
			return 1;
		}
		free(database->dbcache);
		if ((database->dbcache = (char*)malloc(sizeof(char) * len)) == NULL) {
			return 1;
		}
		if (fgets(database->dbcache, len, database->dbstrm) == NULL) {
			free(database->dbcache);
			return 1;
		}
		database->dbit = 0;
	}
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int get_info(struct info** info, const char* str) {
	int ret = 0;
	int ntoken = 1;
	char* buff;
	char* saveptr;
	if ((*info = (struct info*) malloc(sizeof(struct info))) == NULL) {
		return 1;
	}
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ') {
			ntoken++;
		}
	}
	if ((buff = strdup(str)) == NULL) {
		free(*info);
		return 1;
	}
	buff = strtok_r(buff, " ", &saveptr);
	if (ntoken == 1) {
		(*info)->section = strdup(buff);
		(*info)->key = NULL;
		(*info)->value = NULL;
	}
	if (ntoken == 3 || ntoken == 5) {
		(*info)->key = strdup(buff);
		if (!strcmp(strtok_r(NULL, " ", &saveptr), "FROM")) {
			(*info)->section = strdup(strtok_r(NULL, " ", &saveptr));
			(*info)->value = NULL;
		}
		else {
			ret = 1;
		}
	}
	if (ntoken == 5) {
		if (!strcmp(strtok_r(NULL, " ", &saveptr), "AS")) {
			(*info)->value = strdup(strtok_r(NULL, " ", &saveptr));
		}
		else {
			ret = 1;
		}
	}
	free(buff);
	if (ret) {
		free(*info);
		return -1;
	}
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int get_offset(database_t* database, const struct info* info, unsigned **offset) {
	char* tmp;
	if (refresh_cache(database)) {
		return 1;
	}
	if ((*offset = (unsigned*) malloc(sizeof(unsigned))) == NULL) {
		return 1;
	}
	do {
		if ((tmp = strstr(database->dbcache, info->section)) == NULL) {
			free(*offset);
			return -1;
		}
	} while (*(tmp - 1) == '<' && *(tmp + 1) == '>');
	do {
		if ((tmp = strstr(database->dbcache, info->key)) == NULL) {
			free(*offset);
			return -1;
		}
	} while (*(tmp - 1) == '[' && *(tmp + 1) == ']');
	**offset = ((tmp - database->dbcache) / sizeof(char)) + WORDLEN;
	return 0;
}

/* return 0 on success, 1 on sys failure */

int add(database_t* database, const struct info* info) {
	int ret;
	char* buff;
	if (info->section == NULL) {
		return -1;
	}
	if (strlen(info->section) > WORDLEN - 2) {
		return -1;
	}
	if (info->key == NULL) {
		if ((buff = (char*)malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
			return 1;
		}
		memset(buff, ' ', WORDLEN);
		buff[0] = '<';
		strcpy(buff + 1, info->section);
		buff[strlen(buff)] = '>';
		buff[WORDLEN] = 0;
	}
	else {
		/*TODO:
		move the seek to the last position of the section
		make room for data if the section is not in tail
		*/
		if (strlen(info->key) > WORDLEN - 2) {
			return -1;
		}
		if ((buff = (char*)malloc(sizeof(char) * (2 * WORDLEN + 1))) == NULL) {
			return 1;
		}
		memset(buff, ' ', WORDLEN * 2);
		buff[0] = '[';
		strcpy(buff + 1, info->key);
		buff[strlen(buff)] = ']';
		buff[WORDLEN * 2] = 0;
	}
	LOCK(&database->mutex_queue, ret);
	LOCK(&database->mutex_memory, ret);
	UNLOCK(&database->mutex_queue, ret);
	if (fprintf(database->dbstrm, buff) < 0) {
		UNLOCK(&database->mutex_memory, ret);
		free(buff);
		return 1;
	}
	fflush(database->dbstrm);
	database->dbit = 1;
	UNLOCK(&database->mutex_memory, ret);
	free(buff);
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int set(database_t* database, const struct info* info) {
	int ret;
	char* value;
	unsigned* offset = NULL;
	if (info->value == NULL) {
		return -1;
	}
	if (strlen(info->value) > WORDLEN) {
		return -1;
	}
	if ((value = (char*)malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	memset(value, ' ', WORDLEN);
	value[WORDLEN] = '\0';
	strcpy(value, info->value);
	if (strlen(value) < WORDLEN) {
		value[strlen(value)] = ' ';
	}
	LOCK(&database->mutex_queue, ret);
	LOCK(&database->mutex_memory, ret);
	UNLOCK(&database->mutex_queue, ret);
	if ((ret = get_offset(database, info, &offset))) {
		UNLOCK(&database->mutex_memory, ret);
		free(value);
		free(offset);
		return ret;
	}
	if (fseek(database->dbstrm, *offset, SEEK_SET) == -1) {
		UNLOCK(&database->mutex_memory, ret);
		free(value);
		free(offset);
		return 1;
	}
	if (fprintf(database->dbstrm, "%s", value) < 0) {
		UNLOCK(&database->mutex_memory, ret);
		free(value);
		free(offset);
		return 1;
	}
	fflush(database->dbstrm);
	database->dbit = 1;
	UNLOCK(&database->mutex_memory, ret);
	free(value);
	free(offset);
	return 0;
}

/* return 2 on success, 1 on sys failure, -1 if not found */

int get(database_t* database, const struct info* info, char** dest) {
	int ret;
	unsigned* offset = NULL;
	if ((*dest = (char*)malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	LOCK(&database->mutex_queue, ret);
	LOCK(&database->mutex_reader_count, ret);
	if (!database->reader_count) {
		LOCK(&database->mutex_memory, ret);
	}
	database->reader_count++;
	UNLOCK(&database->mutex_queue, ret);
	UNLOCK(&database->mutex_reader_count, ret);
	if ((ret = get_offset(database, info, &offset))) {
		UNLOCK(&database->mutex_memory, ret);
		free(*dest);
		return ret;
	}
	if (snprintf(*dest, WORDLEN, "%s", database->dbcache + *offset) < WORDLEN) {
		UNLOCK(&database->mutex_memory, ret);
		free(*dest);
		free(offset);
		return 1;
	}
	UNLOCK(&database->mutex_memory, ret);
	free(offset);
	return 2;
}

/*	Initiazliza database from file return 1 and set properly errno on error	*/

int database_init(database_t *database, const char* filename) {
	int ret;
	if (database == NULL) {
		return 1;
	}
	if (filename == NULL) {
		return 1;
	}
	database->dbcache = NULL;
	database->dbit = 1;
	database->reader_count = 0;
	/* mutex for each elemnt in the db */
	if (pthread_mutex_init(&database->mutex_queue, NULL)) {
		return 1;
	}
	if (pthread_mutex_init(&database->mutex_reader_count, NULL)) {
		return 1;
	}
	if (pthread_mutex_init(&database->mutex_memory, NULL)) {
		return 1;
	}
	UNLOCK(&database->mutex_queue, ret);
	UNLOCK(&database->mutex_reader_count, ret);
	UNLOCK(&database->mutex_memory, ret);
	if ((database->dbstrm = fopen(filename, "r+")) == NULL) {
		return 1;
	}
	if (flock(fileno(database->dbstrm), LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		fclose(database->dbstrm);
		return 1;
	}
	return 0;
}

/*	Close database return EOF and set properly errno on error */

int database_close(database_t *database) {
	if (database == NULL) {
		return 1;
	}
	free(database->dbcache);
	return fclose(database->dbstrm);
}

/*	Execute a query return 1 and set properly errno on error */

int database_execute(database_t* database, const char* query, char** result) {
	int ret = 0;
	struct info* qinfo = NULL;
	if (database == NULL) {
		ret = 1;
		errno = EINVAL;
	}
	if (!ret && query == NULL) {
		ret = -1;
	}
	if (!ret && result == NULL) {
		ret = -1;
	}
	if (!ret && strlen(query) < 5) {
		ret = -1;
	}
	if (!ret) {
		ret = get_info(&qinfo, query + 4);
	}
	if (!ret) {
		if (!strncmp(query, "ADD ", 4)) {
			ret = add(database, qinfo);
		}
		else if (!strncmp(query, "SET ", 4)) {
			ret = set(database, qinfo);
		}
		else if (!strncmp(query, "GET ", 4)) {
			ret = get(database, qinfo, result);
		}
		else {
			ret = -1;
		}
		free(qinfo);
	}
	switch (ret) {
	case -1:
		*result = DBMSG_FAIL;
		return 0;
	case 0:
		*result = DBMSG_SUCC;
		return 0;
	case 1:
		*result = DBMSG_ERR;
		return 1;
	default:
		strtok(*result, " ");	//souldn't be handled here
		return 0;
	}
}
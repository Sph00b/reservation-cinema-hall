#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#include "asprintf.h"

#define WORDLEN 16

/*	IDK how to name
	undefined behaviour if the strings pointed are longer than WORDLEN
*/

struct info {
	const char* section;
	const char* key;
	const char* value;
};

int refresh_cache(database_t* database) {
	if (database->dbit) {
		int len;
		if (fseek(database->dbstrm, 0, SEEK_END) == -1) {
			return 1;
		}
		if ((len = ftell(database->dbstrm)) == -1) {
			return 1;
		}
		free(database->dbcache);
		if ((database->dbcache = (char*)malloc(sizeof(char) * len)) == NULL) {
			return 1;
		}
		if (fseek(database->dbstrm, 0, SEEK_SET) == -1) {
			return 1;
		}
		if (fgets(database->dbcache, len, database->dbstrm) == NULL) {
			return 1;
		}
		database->dbit = 0;
	}
	return 0;
}

/* return NULL on sys failure or on unexpected string*/

int get_info(struct info** info, const char* str) {
	char* buff;
	int ntoken = 1;
	if (str == NULL) {
		return 1;
	}
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
	buff = strtok(buff, " ");
	switch (ntoken) {
	case 1:
		(*info)->section = buff;
		(*info)->key = NULL;
		(*info)->value = NULL;
		return 0;
	case 3:
		(*info)->key = buff;
		if (strcmp(strtok(NULL, " "), "FROM")) {
			break;
		}
		(*info)->section = strtok(NULL, " ");
		(*info)->value = NULL;
		return 0;
	case 5:
		(*info)->key = buff;
		if (strcmp(strtok(NULL, " "), "FROM")) {
			break;
		}
		(*info)->section = strtok(NULL, " ");
		if (strcmp(strtok(NULL, " "), "AS")) {
			break;
		}
		(*info)->value = strtok(NULL, " ");
		return 0;
	default:
		break;
	}
	free(*info);
	free(buff);
	return 1;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int get_offset(database_t* database, const struct info* info) {
	char* tmp;
	if (info == NULL) {
		return -1;
	}
	if (info->section == NULL) {
		return -1;
	}
	if (info->key == NULL) {
		return -1;
	}
	if (refresh_cache(database)) {
		return -1;
	}
	do {
		if ((tmp = strstr(database->dbcache, info->section)) == NULL) {
			return -1;
		}
	} while (*(tmp - 1) == '<' && *(tmp + 1) == '>');
	do {
		if ((tmp = strstr(database->dbcache, info->key)) == NULL) {
			return -1;
		}
	} while (*(tmp - 1) == '[' && *(tmp + 1) == ']');
	return ((tmp - database->dbcache) / sizeof(char)) + WORDLEN;
}

/* return 0 on success, 1 on sys failure */

int add(database_t* database, const struct info* info) {
	int ret;
	char* buff;
	if (database == NULL) {
		return 1;
	}
	if (info == NULL) {
		return 1;
	}
	if (info->section == NULL) {
		return 1;
	}
	if (strlen(info->section) > WORDLEN - 2) {
		return 1;
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
			return 1;
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
	while ((ret = pthread_mutex_lock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_lock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if (fprintf(database->dbstrm, buff) < 0) {
		free(buff);
		return 1;
	}
	fflush(database->dbstrm);
	while ((ret = pthread_mutex_unlock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	free(buff);
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int get(database_t* database, const struct info* info, char** dest) {
	int ret;
	int offset;
	if (info == NULL) {
		return 1;
	}
	while ((ret = pthread_mutex_lock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_lock(&database->read_count_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if (!database->reader_count) {
		while ((ret = pthread_mutex_lock(&database->memory_access)) && errno == EINTR);
		if (ret) {
			return 1;
		}
	}
	database->reader_count++;
	while ((ret = pthread_mutex_unlock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->read_count_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if ((offset = get_offset(database, info)) == -1) {
		return 1;
	}
	if (fseek(database->dbstrm, offset, SEEK_SET) == -1) {
		return 1;
	}
	if ((*dest = (char*) malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	if (fgets(*dest, WORDLEN, database->dbstrm) == NULL) {
		free(*dest);
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int set(database_t* database, const struct info* info) {
	int ret;
	int offset;
	char* value;
	if (info == NULL) {
		return 1;
	}
	if (info->value == NULL) {
		return 1;
	}
	while ((ret = pthread_mutex_lock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_lock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if ((offset = get_offset(database, info)) == -1) {
		return 1;
	}
	if (fseek(database->dbstrm, offset, SEEK_SET) == -1) {
		return 1;
	}
	if (strlen(info->value) > WORDLEN) {
		return 1;
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
	if (fprintf(database->dbstrm, "%s", value) < 0) {
		free(value);
		return 1;
	}
	fflush(database->dbstrm);
	database->dbit = 1;
	while ((ret = pthread_mutex_unlock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	free(value);
	return 0;
}

int database_execute(database_t* database, const char* query, char** result) {
	int ret = 0;
	struct info* qinfo = NULL;
	if (database == NULL) {
		ret = 1;
		errno;	//TODO: set properly errno
	}
	if (!ret && query == NULL) {
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
			if (!ret) {
				ret = 2;
			}
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
		strtok(*result, " ");	//soulfn't be handled here
		return 0;
	}
}

int database_init(database_t *database, const char* filename) {
	int ret;
	database->dbcache = NULL;
	database->dbit = 1;
	database->reader_count = 0;
	/* mutex for each elemnt in the db */
	if (pthread_mutex_init(&database->service_queue, NULL)) {
		return 1;
	}
	if (pthread_mutex_init(&database->read_count_access, NULL)) {
		return 1;
	}
	if (pthread_mutex_init(&database->memory_access, NULL)) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->service_queue)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->read_count_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&database->memory_access)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if ((database->dbstrm = fopen(filename, "r+")) == NULL) {
		return 1;
	}
	if (flock(fileno(database->dbstrm), LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		fclose(database->dbstrm);
		return 1;
	}
	return 0;
}

int database_close(database_t *database) {
	int ret;
	ret = fclose(database->dbstrm);
	free(database->dbcache);
	return ret;
}

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

struct info* get_info(const char* str) {
	struct info* info;
	char* buff;
	int ntoken = 1;
	if (str == NULL) {
		return NULL;
	}
	if ((info = (struct info*) malloc(sizeof(struct info))) == NULL) {
		return NULL;
	}
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ') {
			ntoken++;
		}
	}
	if ((buff = strdup(str)) == NULL) {
		free(info);
		return NULL;
	}
	buff = strtok(buff, " ");
	switch (ntoken) {
	case 1:
		info->section = buff;
		info->key = NULL;
		info->value = NULL;
		return info;
	case 3:
		info->key = buff;
		if (strcmp(strtok(NULL, " "), "FROM")) {
			break;
		}
		info->section = strtok(NULL, " ");
		info->value = NULL;
		return info;
	case 5:
		info->key = buff;
		if (strcmp(strtok(NULL, " "), "FROM")) {
			break;
		}
		info->section = strtok(NULL, " ");
		if (strcmp(strtok(NULL, " "), "AS")) {
			break;
		}
		info->value = strtok(NULL, " ");
		return info;
	default:
		break;
	}
	free(info);
	free(buff);
	return NULL;
}

/* return -1 on sys failure or if the searched section/key is not found*/

int get_offset(database_t* database, const struct info* info) {
	/* return -1 on error*/
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

int add(database_t* database, const struct info* info) {
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
	if (fprintf(database->dbstrm, buff) < 0) {
		free(buff);
		return 1;
	}
	fflush(database->dbstrm);
	free(buff);
	return 0;
}

int get(database_t* database, const struct info* info, char** dest) {
	int offset;
	if (info == NULL) {
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
	return 0;
}

int set(database_t* database, const struct info* info) {
	char* value;
	int offset;
	if (info == NULL) {
		return 1;
	}
	if (info->value == NULL) {
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
	free(value);
	database->dbit = 1;
	return 0;
}

char* database_execute(database_t *database, const char* query) {
	int ret = 1;
	if (database == NULL) {
		errno;	//TODO: set properly errno
		return DBMSG_ERR;
	}
	if (query == NULL) {
		return DBMSG_FAIL;
	}
	if (strlen(query) > 5) {
		struct info* qinfo = get_info(query + 4);
		if (qinfo == NULL) {
			ret = 1;
		}
		else {
			if (!strncmp(query, "ADD ", 4)) {
				ret = add(database, qinfo);
			}
			else if (!strncmp(query, "SET ", 4)) {
				ret = set(database, qinfo);
			}
			else if (!strncmp(query, "GET ", 4)) {
				char* result;
				ret = get(database, qinfo, &result);
				if (!ret) {
					strtok(result, " ");	//this should be deleted and handled elsewere
					return result;
				}
			}
		}
	}
	switch (ret) {
	case 0:
		return DBMSG_SUCC;
	case 1:
		return DBMSG_FAIL;
	default:
		return DBMSG_ERR;
	}
}

int database_init(database_t *database, const char* filename) {
	database->dbcache = NULL;
	database->dbit = 1;
	if ((database->dbstrm = fopen(filename, "r+")) == NULL) {
		return 1;
	}
	if (flock(fileno(database->dbstrm), LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		fclose(database->dbstrm);
		return 1;
	}
	/* mutex for each elemnt in the db */
	if ((database->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))) == NULL) {
		fclose(database->dbstrm);
		return 1;
	}
	if (pthread_mutex_init(database->mutex, NULL)) {
		fclose(database->dbstrm);
		free(database->mutex);
		return 1;
	}
	if (pthread_mutex_unlock(database->mutex)) {
		fclose(database->dbstrm);
		free(database->mutex);
		return 1;
	}
	return 0;
}

int database_close(database_t *database) {
	int ret;
	ret = fclose(database->dbstrm);
	free(database->dbcache);
	free(database->mutex);
	return ret;
}
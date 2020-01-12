#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#define WORDLEN 16

FILE* dbstrm = NULL;	//stream to the database
char* dbcache = NULL;	//database buffer cache
uint8_t dbit = 1;		//dirty bit
uint8_t rows = 0;		//number of seat's rows
uint8_t columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

/*	IDK how to name
	undefined behaviour if the strings pointed are longer than WORDLEN
*/

struct info {
	const char* section;
	const char* key;
	const char* value;
};

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

int get_offset(const struct info* info) {
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
	if (dbit) {
		int len;
		if (fseek(dbstrm, 0, SEEK_END) == -1) {
			return -1;
		}
		if ((len = ftell(dbstrm)) == -1) {
			return -1;
		}
		free(dbcache);
		if ((dbcache = (char*) malloc(sizeof(char) * len)) == NULL) {
			return -1;
		}
		if (fseek(dbstrm, 0, SEEK_SET) == -1) {
			return -1;
		}
		if (fgets(dbcache, len, dbstrm) == NULL) {
			return -1;
		}
		dbit = 0;
	}
	tmp = dbcache;
	do {
		if ((tmp = strstr(dbcache, info->section)) == NULL) {
			return -1;
		}
	} while (*(tmp - 1) == '<' && *(tmp + 1) == '>');
	do {
		if ((tmp = strstr(dbcache, info->key)) == NULL) {
			return -1;
		}
	} while (*(tmp - 1) == '[' && *(tmp + 1) == ']');
	return ((tmp - dbcache) / sizeof(char)) + WORDLEN;
}

int add(const struct info* info) {
	char* buff;
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
	if (fprintf(dbstrm, buff) < 0) {
		free(buff);
		return 1;
	}
	fflush(dbstrm);
	free(buff);
	return 0;
}

int get(const struct info* info, char** dest) {
	int offset;
	if (info == NULL) {
		return 1;
	}
	if ((offset = get_offset(info)) == -1) {
		return 1;
	}
	if (fseek(dbstrm, offset, SEEK_SET) == -1) {
		return 1;
	}
	if ((*dest = (char*) malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	if (fgets(*dest, WORDLEN, dbstrm) == NULL) {
		free(*dest);
		return 1;
	}
	return 0;
}

int set(const struct info* info) {
	char* value;
	int offset;
	if (info == NULL) {
		return 1;
	}
	if (info->value == NULL) {
		return 1;
	}
	if ((offset = get_offset(info)) == -1) {
		return 1;
	}
	if (fseek(dbstrm, offset, SEEK_SET) == -1) {
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
	if (fprintf(dbstrm, "%s", value) < 0) {
		free(value);
		return 1;
	}
	fflush(dbstrm);
	free(value);
	dbit = 1;
	return 0;
}

char* database_execute(const char* query) {
	int ret = -1;
	if (dbstrm == NULL) {
		errno = EBADF;
	}
	else {
		if (strlen(query) > 5) {
			struct info* qinfo = get_info(query + 4);
			if (qinfo == NULL) {
				ret = 1;
			}
			else{
				if (!strncmp(query, "ADD ", 4)) {
					ret = add(qinfo);
				}
				else if (!strncmp(query, "SET ", 4)) {
					ret = set(qinfo);
				}
				else if (!strncmp(query, "GET ", 4)) {
					char* result;
					ret = get(qinfo, &result);
					if (!ret) {
						strtok(result, " ");	//this should be deleted and handled elsewere
						return result;
					}
				}
			}
		}
		else {
			ret = 1;
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

int not_database_create(const char* filename) {
	char* msg_init[] = {
	"ADD NETWORK",
	"ADD IP FROM NETWORK",
	"SET IP FROM NETWORK AS 127.0.0.1",
	"ADD PORT FROM NETWORK",
	"SET PORT FROM NETWORK AS 55555",
	"ADD CONFIG",
	"ADD PID FROM CONFIG",
	"SET PID FROM CONFIG AS 0",
	"ADD ROWS FROM CONFIG",
	"SET ROWS FROM CONFIG AS 1",
	"ADD COLUMNS FROM CONFIG",
	"SET COLUMNS FROM CONFIG AS 1",
	"ADD DATA",
	NULL
	};
	int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
	close(dbfd);
	if ((dbstrm = fopen(filename, "r+")) == NULL) {
		return 1;
	}
	for (int i = 0; msg_init[i]; i++) {
		if (!strcmp(DBMSG_ERR, database_execute(msg_init[i]))) {
			remove(filename);
			return 1;
		}
	}
	return 0;
}

int database_init(const char* filename) {
	/*	Init the stream of the file, return 0 on success	*/
	if ((dbstrm = fopen(filename, "r+")) == NULL) {
		//return 1;
		if (not_database_create(filename)) {	//shouldn't be here
			return 1;
		}
	}
	rows = atoi(database_execute("GET ROWS FROM CONFIG"));
	columns = atoi(database_execute("GET COLUMNS FROM CONFIG"));
	if (pthread_mutex_init(&mutex, NULL)) {
		return 1;
	}
	if (pthread_mutex_unlock(&mutex)) {
		return 1;
	}
	return 0;
}

int database_close() {
	return fclose(dbstrm);
}
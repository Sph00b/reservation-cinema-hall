#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>


#include <syslog.h>

#define WORDLEN 16

FILE* dbstrm = NULL;	//stream to the database
short rows = 0;		//number of seat's rows
short columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

/*	IDK how to name it	*/
struct info {
	const char* section;
	const char* key;
	const char* value;
};

/*Buggy memory leack*/

struct info* get_info(const char* str) {
	struct info* info;
	char *buff;
	if (str == NULL) {
		return NULL;
	}
	if ((info = (struct info*) malloc(sizeof(struct info))) == NULL) {
		return NULL;
	}
	if ((buff = strdup(str)) == NULL) {
		return NULL;
	}
	if ((info->key = strtok(buff, " ")) == NULL) {
		return NULL;
	}
	char* tmp = strtok(NULL, " ");
	if (tmp == NULL) {
		info->section = info->key;
		info->key = NULL;
		return info;
	}
	if (strcmp("FROM", tmp)) {
		return NULL;
	}
	if ((info->section = strtok(NULL, " ")) == NULL) {
		return NULL;
	}
	if (((info->value = strtok(NULL, " ")) != NULL)) {
		if (strcmp("AS", info->value)) {
			return info;
		}
		if ((info->value = strtok(NULL, " ")) == NULL) {
			return NULL;
		}
		if (strtok(NULL, " ")) {
			return NULL;
		}
		if (strlen(info->value) > WORDLEN) {
			return NULL;
		}
		if (strstr(info->value, " ") - info->value < strlen(info->value)) {
			return NULL;
		}
	}
	return info;;
}

int get_offset(const struct info* info) {
	/* return -1 on error*/
	char* section;
	char* key;
	char* buff;
	int bufflen = 4096;
	int rltv_off = -bufflen;
	int rcrd_off = 0;
	char* psctn = NULL;
	char* prcrd = NULL;
	if (fseek(dbstrm, 0, SEEK_SET) == -1) {
		return -1;
	}
	if ((buff = (char*)malloc(sizeof(char) * bufflen)) == NULL) {
		return -1;
	}
	if ((section = (char*)malloc(sizeof(char) * WORDLEN)) == NULL) {
		return -1;
	}
	if ((key = (char*)malloc(sizeof(char) * WORDLEN)) == NULL) {
		return -1;
	}
	strcpy(section, "<\0");
	strcat(section, info->section);
	strcat(section, ">");
	strcpy(key, "[\0");
	strcat(key, info->key);
	strcat(key, "]");
	while (!prcrd) {
		if (fgets(buff, bufflen, dbstrm) == NULL) {
			return -1;
		}
		rltv_off += bufflen;
		if (!psctn) {
			if ((psctn = strstr(buff, section)) != NULL) {
				int offset = rltv_off + (psctn - buff) / sizeof(char) + strlen(section);
				fseek(dbstrm, offset, SEEK_SET);
				rltv_off -= bufflen - offset;
			}
		}
		else {
			char* psctn_next = strstr(buff, "<");
			prcrd = strstr(buff, key);
			if (psctn_next && prcrd && psctn_next < prcrd) {
				return -1;
			}
		}
	}
	rcrd_off = (prcrd - buff) / sizeof(char);
	return rltv_off + rcrd_off + WORDLEN;
}

int get(const struct info* src, char** dest) {
	int offset;
	char* msg;
	if (src == NULL) {
		return 1;
	}
	if ((msg = (char*)malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	if ((offset = get_offset(src)) == -1) {
		return 1;
	}
	if (fseek(dbstrm, offset, SEEK_SET) == -1) {
		return 1;
	}
	if (fgets(msg, WORDLEN, dbstrm) == NULL) {
		return 1;
	}
	*dest = malloc(sizeof(msg));
	strcpy(*dest, msg);
	return 0;
}

int set(const struct info* src) {
	char* msg;
	int offset;
	if (src == NULL) {
		return 1;
	}
	if (strlen(src->value) > WORDLEN) {
		return 1;
	}
	if ((msg = (char*)malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	memset(msg, ' ', WORDLEN);
	msg[WORDLEN] = 0;
	strcpy(msg, src->value);
	if (strlen(msg) < WORDLEN) {
		msg[strlen(msg)] = ' ';
	}
	if ((offset = get_offset(src)) == -1) {
		return 1;
	}
	if (fseek(dbstrm, offset, SEEK_SET) == -1) {
		return 1;
	}
	if (fprintf(dbstrm, "%s", msg) < 0) {
		return 1;
	}
	fflush(dbstrm);
	free(msg);
	return 0;
}

int add(const struct info* src) {
	char* content;
	int len = WORDLEN;
	if (src == NULL) {
		return 1;
	}
	if (strlen(src->section) > WORDLEN - 2) {
		return 1;
	}
	if (src->key) {
		len *= 2;
		if (strlen(src->key) > WORDLEN - 2) {
			return 1;
		}
	}
	if ((content = (char*)malloc(sizeof(char) * (len + 1))) == NULL) {
		return 1;
	}
	memset(content, ' ', len);
	content[len] = 0;
	if (src->key) {
		content[0] = '[';
		strcpy(content + 1, src->key);
		content[strlen(content)] = ']';
	}
	else {
		content[0] = '<';
		strcpy(content + 1, src->section);
		content[strlen(content)] = '>';
	}
	if (fprintf(dbstrm, content) < 0) {
		return 1;
	}
	fflush(dbstrm);
	free(content);
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
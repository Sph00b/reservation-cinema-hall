#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define WORDLEN 16

char *msg_succ = "OPERATION SUCCEDED";
char *msg_fail = "OPERATION FAILED";
char *msg_err = "DATABASE FAILURE";
char *msg_init[] = {
	"ADD NETWORK",
	"ADD IP FROM NETWORK",
	"SET IP FROM NETWORK AS 127.0.0.1",
	"ADD PORT FROM NETWORK",
	"SET PORT FROM NETWORK AS 55555",
	"ADD CONFIG",
	"ADD ROWS FROM CONFIG",
	"SET ROWS FROM CONFIG AS 1",
	"ADD COLUMNS FROM CONFIG",
	"SET COLUMNS FROM CONFIG AS 1",
	"ADD DATA",
	NULL
};

FILE* dbstrm = NULL;	//stream to the database
short rows = 0;		//number of seat's rows
short columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

/*	IDK how to name it	*/
struct info{
	const char *section;
	const char *key;
	const char *value;
};

/*Buggy*/

struct info *get_info(const char **query){
	struct info *info;
	char *buff;
	if (query == NULL){
		return NULL;
	}
	if ((info = (struct info *) malloc(sizeof(struct info))) == NULL){
		return NULL;
	}
	if ((buff = (char *) malloc(sizeof(char) * (strlen(*query) + 1))) == NULL){
		return NULL;
	}
	if (strcpy(buff, *query) == NULL){
		return NULL;
	}
	if ((info->key = strtok(buff, " ")) == NULL){
		return NULL;
	}
	char *tmp = strtok(NULL, " ");
	if (tmp == NULL){
		info->section = info->key;
		info->key = NULL;
		return info;
	}
	if (strcmp("FROM", tmp)){
		return NULL;
	}
	if ((info->section = strtok(NULL, " ")) == NULL){
		return NULL;
	}
	if (((info->value = strtok(NULL, " ")) != NULL)){
		if (strcmp("AS", info->value)){
			return info;
		}
		if ((info->value = strtok(NULL, " ")) == NULL){
			return NULL;
		}
		if (strtok(NULL, " ")){
			return NULL;
		}
		if (strlen(info->value) > WORDLEN){
			return NULL;
		}
		if (strstr(info->value, " ") - info->value < strlen(info->value)){
			return NULL;
		}
	}
	return info;;
}

/*I'm seriously thinking of use the line terminator character for parsing*/

int get_offset(const struct info *info){
	/* return -1 on error*/
	char *section;
	char *key;
	char *buff;
	int bufflen = 4096;
	int rltv_off = -bufflen;
	int rcrd_off = 0;
	char *psctn = NULL;
	char *prcrd = NULL;
	if (fseek(dbstrm, 0, SEEK_SET) == -1){
		return -1;
	}
	if ((buff = (char *) malloc(sizeof(char) * bufflen)) == NULL){
		return -1;
	}
	if ((section = (char *)malloc(sizeof(char) * WORDLEN)) == NULL){
		return -1;
	}
	if ((key = (char *)malloc(sizeof(char) * WORDLEN)) == NULL){
		return -1;
	}
	strcat(section, "<");
	strcat(section, info->section);
	strcat(section, ">");
	strcat(key, "[");
	strcat(key, info->key);
	strcat(key, "]");
	while(!prcrd){
		if (fgets(buff, bufflen, dbstrm) == NULL){
			return -1;
		}
		rltv_off += bufflen;
		if (!psctn){
			if(psctn = strstr(buff, section)){
				int offset = rltv_off + (psctn - buff) / sizeof(char) + strlen(section);
				fseek(dbstrm, offset, SEEK_SET);
				rltv_off -= bufflen - offset;
			}
		}
		else{
			char *psctn_next = strstr(buff, "<");
			prcrd = strstr(buff, key);
			if (psctn_next && prcrd && psctn_next < prcrd){
				return -1;
			}
		}
	}
	rcrd_off = (prcrd - buff) / sizeof(char);
	return rltv_off + rcrd_off + WORDLEN - 1;
}

int get(char **dest, const struct info *src){
	int ret = 0;
	int err = 0;
	int offset;
	char *msg;
	if (src == NULL){
		return 1;
	}
	if ((msg = (char *)malloc(sizeof(char) * (WORDLEN + 1))) == NULL){
		return 1;
	}
	while((err = pthread_mutex_lock(&mutex)) && errno == EINTR);	//try lock
	if (err){
		return -1;
	}
	if (!ret && (offset = get_offset(src)) == -1){
		ret = 1;
	}
	if (!ret && fseek(dbstrm, offset, SEEK_SET) == -1){
		ret = 1;
	}
	if (!ret && fgets(msg, WORDLEN, dbstrm) == NULL){
		ret = 1;
	}
	while((err = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
	if (err){
		return -1;
	}
	*dest = malloc(sizeof(msg));
	strcpy(*dest, msg);
	return ret;
}

int set(const struct info *src){
	int ret = 0;
	int err = 0;
	int offset;
	char *msg;
	if (src == NULL){
		return 1;
	}
	if ((msg = (char *)malloc(sizeof(char) * (WORDLEN + 1))) == NULL){
		return 1;
	}
	memset(msg, ' ', WORDLEN);
	strcpy(msg, src->value);
	if (strlen(msg) < WORDLEN){
		msg[strlen(msg)] = ' ';
	}
	while((err = pthread_mutex_lock(&mutex)) && errno == EINTR);	//try lock
	if (err){
		return -1;
	}
	if (!ret && (offset = get_offset(src)) == -1){
		ret = 1;
	}
	if (!ret && fseek(dbstrm, offset + 1, SEEK_SET) == -1){
		ret = 1;
	}
	if (!ret && fprintf(dbstrm, "%s", msg) < 0){
		ret = 1;
	}
	if (!ret){
		fflush(dbstrm);
	}
	while((err = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
	if (err){
		return -1;
	}
	return ret;
}

int add(const struct info *src){
	char *content;
	int len = WORDLEN;
	if (src == NULL){
		return 1;
	}
	if (src->key){
		len += WORDLEN;
	}
	if ((content = (char *)malloc(sizeof(char) * (len + 1))) == NULL){
		return 1;
	}
	if (strlen(src->section) > WORDLEN - 2){
		free(content);
		return 1;
	}
	memset(content, ' ', len);
	if (src->key){
		content[0] = '[';
		strcpy(content + 1, src->key);
		content[strlen(content)] = ']';
	}
	else{
		content[0] = '<';
		strcpy(content + 1, src->section);
		content[strlen(content)] = '>';
	}
	if (fprintf(dbstrm, content) < 0){
		return 1;
	}
	fflush(dbstrm);
	return 0;
}

/*	I'll change it to an int maybe	*/

char *database_execute(const char *query){
	int ret = -1;
	char *result = NULL;
	if (dbstrm == NULL){
		errno = EBADF;
	}
	else{
		const char *ref = query + 4;
		if (strlen(query) < 5){
			ret = 1;
		}
		else if (!strncmp(query, "ADD ", 4)){
			ret =add(get_info(&ref));
		}
		else if (!strncmp(query, "GET ", 4)){
			ret = get(&result, get_info(&ref));
			strtok(result, " ");	//this should be deleted and handled elsewere
		}
		else if (!strncmp(query, "SET ", 4)){
			ret = set(get_info(&ref));
		}
		else{
			return "INVALID REQUEST";
		}
	}
	switch (ret){
	case 0:
		if (!result){
			result = msg_succ;;
		}
		break;
	case 1:
		result = msg_fail;
		break;
	case -1:
		result = msg_err;
		break;
	}
	return result;
}

int database_init(char* filename){
	/*	Init the stream of the file, return 0 on success	*/
	if((dbstrm = fopen(filename, "r+")) == NULL){
		int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
		close(dbfd);
		if((dbstrm = fopen(filename, "r+")) == NULL){
			return 1;
		}
		for (int i = 0; msg_init[i]; i++){
			if (!strcmp(msg_err, database_execute(msg_init[i]))){
				return 1;
			}
		}
	}
	rows = atoi(database_execute("GET ROWS FROM CONFIG"));
	columns = atoi(database_execute("GET COLUMNS FROM CONFIG"));
	if (pthread_mutex_init(&mutex, NULL)){
		return 1;
	}
	if (pthread_mutex_unlock(&mutex)){
		return 1;
	}
	return 0;
}

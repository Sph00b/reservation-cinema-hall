#include "database.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define WORDLEN 16

const char *msg_succ = "Operation succeded";
const char *msg_fail = "Operaiton failed";

FILE* dbf = NULL;	//stream to the database
short rows = 0;		//number of seat's rows
short columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

struct reference{
	const char *section;
	const char *key;
	const char *value;
};

int parse_query(struct reference *dest, const char *query){
	char *buff;
	if ((buff = (char *) malloc(sizeof(char) * (strlen(query) + 1))) == NULL){
		return -1;
	}
	if (strcpy(buff, query) == NULL){
		return -1;
	}
	if ((dest->key = strtok(buff, " ")) == NULL){
		return -1;
	}
	if (strcmp("FROM", strtok(NULL, " "))){
		return -1;
	}
	if ((dest->section = strtok(NULL, " ")) == NULL){
		return -1;
	}
	char *substr;
	if ((substr = strtok(NULL, " ")) == NULL){
		dest->value = NULL;
	}
	else{
		if (strcmp("AS", substr)){
			return -1;
		}
		if ((dest->value = strtok(NULL, " ")) == NULL){
			return -1;
		}
		int len = strlen(dest->value);
		if (len > WORDLEN){
			return -1;
		}
		substr = malloc(sizeof(char) * (len + 1));
		strcpy(substr, dest->value);
		if (strtok(NULL, " ")){
			return -1;
		}
		substr = strtok(substr, " ");
		if (len != strlen(substr)){
			return -1;
		}
		free(substr);
	}
	return 0;
}

/*I'm seriously thinking of use the line terminator character for parsing*/

int get_offset(const struct reference *ref){
	/* return -1 on error*/
	char *section;
	char *key;
	char *buff;
	int bufflen = 4096;
	int rltv_off = -bufflen;
	int rcrd_off = 0;
	char *psctn = NULL;
	char *prcrd = NULL;
	if (fseek(dbf, 0, SEEK_SET) == -1){
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
	strcat(section, ref->section);
	strcat(section, ">");
	strcat(key, "[");
	strcat(key, ref->key);
	strcat(key, "]");
	while(!prcrd){
		/*Non ci preoccupiamo delle stringhe spezzate perché le stringhe hanno dimensione fissa*/
		if (fgets(buff, bufflen, dbf) == NULL){
			return -1;
		}
		rltv_off += bufflen;
		if (!psctn){
			if(psctn = strstr(buff, section)){
				int offset = rltv_off + (psctn - buff) / sizeof(char) + strlen(section);
				fseek(dbf, offset, SEEK_SET);
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

char *get(int offset){
	int ret;
	char *msg;
	if ((msg = (char *)malloc(sizeof(char) * (WORDLEN + 1))) == NULL){
		return NULL;
	}
	while((ret = pthread_mutex_lock(&mutex)) && errno == EINTR);	//try lock
	if (ret){
		return NULL;
	}
	if (fseek(dbf, offset, SEEK_SET) == -1){
		return NULL;
	}
	if (fgets(msg, WORDLEN, dbf) == NULL){
		return NULL;
	}
	while((ret = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
	if (ret){
		return NULL;
	}
	msg = strtok(msg, " ");
	return msg;
}

int set(int offset, const char *value){
	int ret;
	char *msg;
	if ((msg = (char *)malloc(sizeof(char) * (WORDLEN + 1))) == NULL){
		return 1;
	}
	memset(msg, ' ', WORDLEN);
	strcpy(msg, value);
	if (strlen(msg) < WORDLEN){
		msg[strlen(msg)] = ' ';
	}
	while((ret = pthread_mutex_lock(&mutex)) && errno == EINTR);	//try lock
	if (ret){
		return 1;
	}
	if (fseek(dbf, offset + 1, SEEK_SET) == -1){
		return 1;
	}
	if (fprintf(dbf, "%s", msg) < 0){
		return 1;
	}
	fflush(dbf);
	while((ret = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
	if (ret){
		return 1;
	}
	return 0;
}

char *database_execute(const char *query){
	char *result = NULL;
	struct reference r;
	if (dbf == NULL){
		errno = EBADF;
	}
	else{
		if (strlen(query) < 5){
			return "INVALID REQUEST";
		}
		if (parse_query(&r, query + 4)){
			return "INVALID REQUEST";
		}
		int offset;
		if ((offset = get_offset(&r)) == -1){
			return "INVALID REQUEST";
		}
		if (!strncmp(query, "GET ", 4)){
			result = get(offset);
		}
		else if (!strncmp(query, "SET ", 4)){
			if (set(offset, r.value)){
				result = "OPERATION FAILED";
			}
			else{
				result = "OPERATION COMPLETED";
			}
		}
		else{
			return "INVALID REQUEST";
		}
	}
	return result;
}

/* I should probably delete those files*/
int insert_section(const char *section){
	char *content;
	if ((content = (char *)malloc(sizeof(char) * (WORDLEN + 1))) == NULL){
		return 1;
	}
	if (strlen(section) > WORDLEN - 2){
		free(content);
		return 1;
	}
	memset(content, ' ', WORDLEN);
	content[0] = '<';
	strcpy(content + 1, section);
	content[strlen(content)] = '>';
	if (fprintf(dbf, content) < 0){
		return 1;
	}
	fflush(dbf);
	return 0;
}

int insert_key(const char *section, const char *key){
	char *content;
	if ((content = (char *)malloc(sizeof(char) * (WORDLEN * 2 + 1))) == NULL){
		return 1;
	}
	if (strlen(key) > WORDLEN - 2){
		free(content);
		return 1;
	}
	memset(content, ' ', WORDLEN * 2);
	content[0] = '[';
	strcpy(content + 1, key);
	content[strlen(content)] = ']';
	if (fprintf(dbf, content) < 0){
		return 1;
	}
	fflush(dbf);
	return 0;
}

int database_init(char* filename){
	/*	Init the stream of the file, return 0 on success	*/
	if((dbf = fopen(filename, "r+")) == NULL){
		int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
		close(dbfd);
		if((dbf = fopen(filename, "r+")) == NULL){
			return 1;
		}
		char *content;
		insert_section("NETWORK");
		insert_key("NETWORK", "IP");
		insert_key("NETWORK", "PORT");
		insert_section("CONFIG");
		insert_key("CONFIG", "ROWS");
		insert_key("CONFIG", "COLUMNS");
		insert_section("DATA");
		database_execute("SET IP FROM NETWORK AS 127.0.0.1");
		database_execute("SET PORT FROM NETWORK AS 55555");
		database_execute("SET ROWS FROM CONFIG AS 1");
		database_execute("SET COLUMNS FROM CONFIG AS 1");
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

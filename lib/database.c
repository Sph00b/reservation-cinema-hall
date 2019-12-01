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

FILE* stream = NULL;	//stream to the database
short rows = 0;		//number of seat's rows
short columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

struct query{
	const char *section;
	const char *record;
	const char *value;
};

int parse_query(struct query *query, const char *str){
	char *buff;
	if ((buff = (char *) malloc(sizeof(char) * (strlen(str) + 1))) == NULL){
		return -1;
	}
	if (strcpy(buff, str) == NULL){
		return -1;
	}
	if ((query->record = strtok(buff, " ")) == NULL){
		return -1;
	}
	if (strcmp("FROM", strtok(NULL, " "))){
		return -1;
	}
	if ((query->section = strtok(NULL, " ")) == NULL){
		return -1;
	}
	char *substr;
	if ((substr = strtok(NULL, " ")) == NULL){
		query->value = NULL;
	}
	else{
		if (strcmp("AS", substr)){
			return -1;
		}
		if ((query->value = strtok(NULL, " ")) == NULL){
			return -1;
		}
		int len = strlen(query->value);
		if (len > WORDLEN){
			return -1;
		}
		substr = malloc(sizeof(char) * (len + 1));
		strcpy(substr, query->value);
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


int get_offset(const struct query *query){
	/* return -1 on error*/
	char *section;
	char *record;
	char *buff;
	int bufflen = 4096;
	int rltv_off = -bufflen;
	int rcrd_off = 0;
	char *psctn = NULL;
	char *prcrd = NULL;
	if (fseek(stream, 0, SEEK_SET) == -1){
		return -1;
	}
	if ((buff = (char *) malloc(sizeof(char) * bufflen)) == NULL){
		return -1;
	}
	if ((section = (char *)malloc(sizeof(char) * (strlen (query->section) + 3))) == NULL){
		return -1;
	}
	if ((record = (char *)malloc(sizeof(char) * (strlen (query->record) + 5))) == NULL){
		return -1;
	}
	strcat(section, "[");
	strcat(section, query->section);
	strcat(section, "]");
	strcat(record, " ");
	strcat(record, query->record);
	strcat(record, " = ");
	while(!prcrd){
		/*Non ci preoccupiamo delle stringhe spezzate perché le stringhe hanno dimensione fissa*/
		if (fgets(buff, bufflen, stream) == NULL){
			return -1;
		}
		rltv_off += bufflen;
		if (!psctn){
			if(psctn = strstr(buff, section)){
				int offset = rltv_off + (psctn - buff) / sizeof(char) + strlen(section);
				fseek(stream, offset, SEEK_SET);
				rltv_off -= bufflen - offset;
			}
		}
		else{
			char *psctn_next = strstr(buff, "[");
			prcrd = strstr(buff, record);
			if (psctn_next && prcrd && psctn_next < prcrd){
				return -1;
			}
		}
	}
	rcrd_off = (prcrd - buff) / sizeof(char);
	return rltv_off + rcrd_off + strlen(record) - 1;
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
	if (fseek(stream, offset, SEEK_SET) == -1){
		return NULL;
	}
	if (fgets(msg, WORDLEN, stream) == NULL){
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
	if (fseek(stream, offset + 1, SEEK_SET) == -1){
		return 1;
	}
	if (fprintf(stream, "%s", msg) < 0){
		return 1;
	}
	fflush(stream);
	while((ret = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
	if (ret){
		return 1;
	}
	return 0;
}

char *database_execute(const char *request){
	char *result = NULL;
	struct query q;
	int offset;
	if (stream == NULL){
		errno = EBADF;
	}
	else{
		if (strlen(request) < 5){
			return "INVALID REQUEST";
		}
		if (parse_query(&q, request + 4)){
			return "INVALID REQUEST";
		}
		if ((offset = get_offset(&q)) == -1){
			return "INVALID REQUEST";
		}
		if (!strncmp(request, "GET ", 4)){
			result = get(offset);
		}
		else if (!strncmp(request, "SET ", 4)){
			if (set(offset, q.value)){
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

int create(char *filename, int capacity){
	if((stream = fopen(filename, "r+")) == NULL){
		return -1;
	}
	if (fprintf(stream, "\
[NETWORK]       \
          IP =  \
                \
        PORT =  \
                \
[CONFIG]        \
        ROWS =  \
                \
     COLUMNS =  \
                \
[DATA]          ") < 0){
		return -1;
	}
	fflush(stream);
	return 0;
}

int database_init(char* filename){
	/*	Init the stream of the file, return 0 on success	*/
	char *token;
	if((stream = fopen(filename, "r+")) == NULL){
		int datafd = open(filename, O_CREAT | O_EXCL, 0666);
		close(datafd);
		create(filename, 0);
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

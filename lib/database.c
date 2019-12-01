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

const char *msg_succ = "Operation succeded";
const char *msg_fail = "Operaiton failed";

FILE* stream = NULL;	//stream to the database
uint16_t rows = 0;	//number of seat's rows
uint16_t columns = 0;	//number of seat's columns

pthread_mutex_t mutex;

int create(char *filename, int capacity){
	if((stream = fopen(filename, "r+")) == NULL){
		return -1;
	}
	if (fprintf(stream, "[NETWORK] 10.0.2.15 55555 [CONFIG] 1 1 [DATA] 0") < 0){
		return -1;
	}
	fflush(stream);
	return 0;
}

char* token_fromsection(char* section){
	/*	Return NULL on failure	*/
	char *buff;
	char *substr;
	int bufflen = 4096;
	if (fseek(stream, 0, SEEK_SET) == -1){
		return NULL;
	}
	if ((buff = (char *) malloc(sizeof(char) * bufflen)) == NULL){
		return NULL;
	}
	do{
		if (fgets(buff, bufflen, stream) == NULL){
			return NULL;
		}
		substr = strtok(buff, " ");
		while(strcmp(substr, section)){
			if ((substr = strtok(NULL, " ")) == NULL){
				break;
			}
		}
	}while(strcmp(substr, section));
	return strtok(NULL, " ");
}

char *get_map(){
	char *buff;
	char *token;
	if ((token = token_fromsection("[DATA]")) == NULL){
		return NULL;
	}
	if ((buff = (char *) malloc(sizeof(char) * 2 * ((rows * columns) + 64))) == NULL){
		return NULL;
	}
	sprintf(buff, "%d %d ", rows, columns);
	for (int i = 0; i < rows * columns; i++){
		if (token == NULL){
			return NULL;
		}
		strcat(buff, token);
		strcat(buff, " ");
		token = strtok(NULL, " ");
	}
	return buff;
}

int add(const char *buff){
	/*	prendi il puntatore al vettore dei post*/
	/*	ottieni codice univoco	*/
	/*	scrivi codice univoco	*/
	/*	apporta modifiche	*/
	/*	comunica al client	*/
	return 0;
}

int rmv(const char *buff){
	/*	ricerca posti con codice identificativo	e sovrascrivi con NULL	*/
	/*	apporta modifiche	*/
	/*	comunica al client	*/
	return 0;
}

int database_init(char* filename){
	/*	Init the stream of the file, return 0 on success -1 on error	*/
	char *token;
	if((stream = fopen(filename, "r+")) == NULL){
		int datafd = open(filename, O_CREAT | O_EXCL, 0666);
		close(datafd);
		return create(filename, 0);
	}
	if ((token = token_fromsection("[CONFIG]")) == NULL){
		return -1;
	}
	rows = atoi(token);
	token = strtok(NULL, " ");
	columns = atoi(token);
	if (!pthread_mutex_init(&mutex, NULL)){
		return 1;
	}
	if (!pthread_mutex_unlock(&mutex)){
		return -1;
	}
	return 0;
}

int database_network_getconfig(struct cinema_config *conf){
	char *token;
	if (stream == NULL){
		errno = EBADF;
		return -1;
	}
	if ((token = token_fromsection("[NETWORK]")) == NULL){
		return -1;
	}
	if ((conf->ip = (char *) malloc(sizeof(char) * strlen(token))) == NULL){
		return -1;
	}
	strcpy(conf->ip, token);
	token = strtok(NULL, " ");
	conf->port = atoi(token);
	return 0;
}

const char *database_query_handler(const char *query){
	char *msg;
	if (stream == NULL){
		errno = EBADF;
		return NULL;
	}
	if (!strcmp(query, "map")){
		int ret;
		while((ret = pthread_mutex_lock(&mutex)) && errno == EINTR);	//try lock
		if (ret){
			return NULL;
		}
		msg = get_map();
		while((ret = pthread_mutex_unlock(&mutex)) && errno == EINTR);	//try unlock
		if (ret){
			return NULL;
		}
		return msg;
	}
	else if (!strncmp(query, "add ", strlen("add "))){
		if (!add(query + strlen("add "))){
			return msg_succ;
		}
	}
	else if (!strncmp(query, "rmv ", strlen("rmv "))){
		if (!rmv(query + strlen("rmv "))){
			return msg_succ;
		}
	}
	return msg_fail;
}

const char *get(const char *arg){
	return NULL;
}

const char *set(const char *arg){
	return NULL;
}

const char *database_execute(const char *query){
	const char *result = NULL;
	if (stream == NULL){
		errno = EBADF;
	}
	else{
		if (!strncmp(query, "GET ", 4) && strlen(query) > 4){
			result = get(query + 4);
		}
		else if (!strncmp(query, "SET ", 4) && strlen(query) > 4){
			result = set(query + 4);
		}
		else{
			return "INVALID REQUEST";
		}
	}
	return result;
}

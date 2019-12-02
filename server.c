#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include "lib/try.h"
#include "lib/database.h"
#include "lib/connection.h"

void *connection_handler(void *);
void *request_handler(void *);
int start();
int stop();

int status;

struct thread_arg{
	char *ip;
	char *port;
};

int main(int argc, char *argv[]){
	/*Add status, start, stop, change ip and port, change configuration*/
	char *cmd;
	status = 0;
try(
	database_init("data.dat"), (1)
)
	printf("Cinema reservation service\nDatabase initialized\nUse help for list commands\n");
	do{
		scanf("%ms", &cmd);
		if (!strcmp(cmd, "start")){
			start();
		}
		if (!strcmp(cmd, "status")){
			if(!status){
				printf("Server stopped\n");
			}
			else{
				printf("Server running\n");
			}
		}
		if (!strcmp(cmd, "stop")){
			stop();
		}
	} while (strcmp(cmd, "exit"));
	return 0;
}

int start(){
	/*	Start threads	*/
	pthread_t chndl_tid;
	struct thread_arg arg;
try(
	arg.ip = database_execute("GET IP FROM NETWORK"), (NULL)
)
try(
	arg.port = database_execute("GET PORT FROM NETWORK"), (NULL)
)
#ifdef DEBUG
	printf("Config loaded from 'data.dat': address=%s, port=%d\n", arg.ip, atoi(arg.port));
#endif
try(
	pthread_create(&chndl_tid, NULL, connection_handler, (void *)&arg), (!0)
)
	return 0;
}

int stop(){
	/*	Signal closure and wait for threads to stop	*/
	//	pthread_join(chndl_tid, NULL);
	return 0;
}

void *connection_handler(void *arg){
	struct thread_arg *data = (struct thread_arg *) arg;
	pthread_t *tid = NULL;
	int tn = 0;		//thread number
try(
	connection_listener_start(data->ip, atoi(data->port)), (-1)
)
#ifdef DEBUG
	printf("Starting listen\n");
#endif
	do{
		int fd;
try(
		fd = connection_accepted_getfd(), (-1)
)
#ifdef DEBUG
		printf("Connection accepted\n");
#endif
		tn++;
try(
		tid = realloc(tid, sizeof(pthread_t) * tn), (NULL)
)
try(
		pthread_create(&tid[tn - 1], NULL, request_handler, (void *)(long)fd), (!0)
)
	} while (1);
	for (int i = 0; i < tn; i++){
try(
		pthread_join(tid[i], NULL), (!0)
)
	free(tid);
try(
	connection_listener_stop(), (-1)
)
	pthread_exit(0);
	}
}

void *request_handler(void *arg){
	/*	todo: implement a timeout system	*/
	int fd = (int)(long)arg;
	char *buff;
	const char *msg;
try(
	buff = malloc(1024), (NULL)
)
try(
	recv(fd, buff, 1024, 0), (-1)
)
	printf("%s\n", buff);
try(
	msg = database_execute(buff), (NULL)
)
try(
	send(fd, msg, strlen(msg), 0), (-1)
)
try(
	close(fd), (-1)
)
	free(buff);
	pthread_exit(0);
}


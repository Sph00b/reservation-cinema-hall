#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "try.h"

#include "database.h"
#include "connection.h"

void *request_handler(void *);

int main(int argc, char *argv[]){
	int logfd;
	FILE *log;
	pid_t pid, sid;
	if (argc != 2){
		printf("Usage: [filename]\n");
		exit("EXIT_SUCCESS");
	}
try(
	logfd = open("etc/cinemad.log", O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666), (-1)
)
try(
	log = fdopen(logfd, "a"), (NULL)
)
try(	
	pid = fork(), (-1)
)
	if (pid != 0){
		exit(EXIT_SUCCESS);
	}
	// we're not the session leader of our old process group
	stderr = log;
try(
	sid = setsid(), (-1)
)
	// we're the session leader of our process new group
try(	
	pid = fork(), (-1)
)
	if (pid != 0){
		exit(EXIT_SUCCESS);
	}
	// we're not the session leader of our new process group and we lost control of terminal
try(
	chdir("/"), (-1)
)
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	// the process is now a deamon

	/*	Start do his job	*/
	pthread_t *tid = NULL;
	int tc = 0;		//thread counter
	char *ip:
	char *port;
try(
	database_init(argv[1]), (1)
)
try(
	ip = database_execute("GET IP FROM NETWORK"), (NULL)
)
try(
	port = database_execute("GET PORT FROM NETWORK"), (NULL)
)
try(
	connection_listener_start(data->ip, atoi(data->port)), (-1)
)
	do{
		int fd;
try(
		fd = connection_accepted_getfd(), (-1)
)
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
try(
	connection_listener_stop(), (-1)
)
	free(tid);
	close(logfd);
	return 0;
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

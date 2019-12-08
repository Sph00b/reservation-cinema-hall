#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>	//I shouldn't use it
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#define DAEMON

#include "helper.h"
#include "asprintf.h"
#include "database.h"
#include "connection.h"

void *request_handler(void *);
void daemonize();

int main(int argc, char *argv[]){
	daemonize();
	syslog(LOG_DEBUG, "Process demonized");
	pthread_t *tid = NULL;
	int tc = 0;	//thread counter
	char *pidq;	//pid query
	char *ip;
	char *port;
try(
	database_init("etc/data.dat"), (1)
)
	syslog(LOG_INFO, "Database file loaded");
try(
	asprintf(&pidq, "%s %d", "SET PID FROM CONFIG AS", getpid()), (-1)
)
try(	
	database_execute(pidq), (NULL)
)
	free(pidq);
try(
	ip = database_execute("GET IP FROM NETWORK"), (NULL)
)
try(
	port = database_execute("GET PORT FROM NETWORK"), (NULL)
)
try(
	connection_listener_start(ip, atoi(port)), (-1)
)
	syslog(LOG_INFO, "Listener socket started");
	do{
		int fd;
try(
		fd = connection_accepted_getfd(), (-1)
)
		tc++;
try(
		tid = realloc(tid, sizeof(pthread_t) * tc), (NULL)
)
try(
		pthread_create(&tid[tc - 1], NULL, request_handler, (void *)(long)fd), (!0)
)
	} while (1);
	for (int i = 0; i < tc; i++){
try(
		pthread_join(tid[i], NULL), (!0)
)
	}
try(
	connection_listener_stop(), (-1)
)
	free(tid);
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

void daemonize(){
	pid_t pid, sid;
	pid = fork();
	if (pid == -1){
		fprintf(stderr, "%m");
		exit(EXIT_FAILURE);
	}
	if (pid != 0){
		exit(EXIT_SUCCESS);
	}
try(
	sid = setsid(), (-1)
)
	syslog(LOG_DEBUG, "Session created");
try(	
	pid = fork(), (-1)
)
	if (pid != 0){
		exit(EXIT_SUCCESS);
	}
	syslog(LOG_DEBUG, "Process group leader terminated, lost constrol of terminal");
	char *cdir;
try(
	asprintf(&cdir, "%s%s", getenv("HOME"), "/.cinema"), (-1)
)
try(
	chdir(cdir), (-1)
)
try(
	setenv("PWD", cdir, 1), (!0)
)
	free(cdir);
	syslog(LOG_DEBUG, "Current working directory and env's pwd changed to %s", getenv("PWD"));
	umask(0);
	syslog(LOG_DEBUG, "Resetted umask");
try(
	fclose(stdin), (EOF)
)
try(
	fclose(stdout), (EOF)
)
try(	
	fclose(stderr), (EOF)
)
	syslog(LOG_DEBUG, "Standard streams closed");
try(
	mkdir(".tmp", 0775), (-1 * (errno != EEXIST))
)
	int pidfd;
try(
	pidfd = open(".tmp/cinemad.pid", O_CREAT, 0600), (-1)
)
try(
	flock(pidfd, LOCK_EX | LOCK_NB), (-1)	//instead of semget & ftok to avoid mix SysV and POSIX
)
try(
	mkdir("etc", 0775), (-1 * (errno != EEXIST))
)
}

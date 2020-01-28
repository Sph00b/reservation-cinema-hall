#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "connection.h"
#include "asprintf.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		fprintf(stderr, "%m\n");\
		exit(EXIT_FAILURE);\
	}

int daemon_start();
int daemon_stop();
int daemon_restart();
int daemon_query(char*, char**);

int main(int argc, char *argv[]){
	if (argc == 2 && !strncasecmp(argv[1], "start", 5)) {
try(
		daemon_start(), (1)
)
	}
	else if (argc == 2 && !strncasecmp(argv[1], "stop", 4)) {
try(
		daemon_stop(), (1)
)
	}
	else if (argc == 2 && !strncasecmp(argv[1], "restart", 7)) {
try(
		daemon_restart(), (1)
)
	}
	else if (argc == 3 && !strncasecmp(argv[1], "query", 5)) {
		char* buff;
try(
		daemon_query(argv[2], &buff), (1)
)
		printf("%s\n", buff);
	}
	else {
		printf("Usage:\t cinemactl [COMMAND]\n\n");
		exit(EXIT_SUCCESS);
	}
	return 0;
}

int daemon_start(){
	pid_t pid;
	char *filename;
	if (asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/bin/cinemad") == -1) {
		return 1;
	}
	pid = fork();
switch (pid) {
case 0:
	if (execl(filename, "cinemad", NULL) == -1) {
		return 1;
	}
case -1:
	return 1;
default:
	return 0;
	}

}

int daemon_stop(){
	char* pid;
try(
	daemon_query("GET PID FROM CONFIG", &pid), (1)
)
try(
	kill(atoi(pid), SIGKILL), (-1)
)
	printf("Server stopped\n");
	return 0;
}

int daemon_restart() {
	if (daemon_stop()) {
		return 1;
	}
	if (daemon_start()) {
		return 1;
	}
	return 0;
}

int daemon_query(char* query, char** result) {
	connection_t con;
	char* filename;
	if (asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/tmp/socket") == -1) {
		return 1;
	}
	if (connection_init(&con, filename, 0) == -1) {
		return 1;
	}
	free(filename);
	if (connetcion_connect(&con) == -1) {
		return 1;
	}
	if (connection_send(&con, query) == -1) {
		return 1;
	}
	if (connection_recv(&con, result) == -1) {
		return 1;
	}
	return 0;
}
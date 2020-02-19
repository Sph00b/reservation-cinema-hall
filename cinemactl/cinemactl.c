#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "connection.h"
#include "asprintf.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		fprintf(stderr, "%m\n");\
		exit(EXIT_FAILURE);\
	}

int server_start();
int server_stop();
int server_status();
int server_restart();
int server_query(char*, char**);

int main(int argc, char *argv[]){
	if (argc == 2 && !strncasecmp(argv[1], "start", 5)) {
try(
		server_start(), (1)
)
	}
	else if (argc == 2 && !strncasecmp(argv[1], "stop", 4)) {
try(
		server_stop(), (1)
)
	}
	else if (argc == 2 && !strncasecmp(argv[1], "status", 6)) {
try(
		server_status(), (1)
)
}
	else if (argc == 2 && !strncasecmp(argv[1], "restart", 7)) {
try(
		server_restart(), (1)
)
	}
	else if (argc == 3 && !strncasecmp(argv[1], "query", 5)) {
		char* buff;
try(
		server_query(argv[2], &buff), (1)
)
		printf("%s\n", buff);
	}
	else {
		printf("\nUsage:\n cinemactl [COMMAND]\n\n\
			\rCommands:\n\
			\r start\n\
			\r stop\n\
			\r status\n\
			\r restart\n\
			\r query [...]\n\n");
		exit(EXIT_SUCCESS);
	}
	return 0;
}

int server_start(){
	pid_t pid;
	char *filename;
	if (asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/bin/cinemad") == -1) {
		return 1;
	}
	pid = fork();
switch (pid) {
case 0:
	if (execl(filename, "cinemad", NULL) == -1) {
		free(filename);
		return 1;
	}
case -1:
	free(filename);
	return 1;
default:
	free(filename);
	return 0;
	}

}

int server_stop(){
	char* pid;
try(
	server_query("GET PID", &pid), (1)
)
try(
	kill(atoi(pid), SIGTERM), (-1)
)
	free(pid);
	return 0;
}

int server_restart() {
	if (server_stop()) {
		return 1;
	}
	sleep(10);	//should I change it?
	printf("Server shutting down please wait...\n");
	if (server_start()) {
		return 1;
	}
	printf("Server restarted\n");
	return 0;
}

int server_status() {
	char* pid = NULL;
	char* timestr = NULL;
	char* icon = NULL;
	char* status = NULL;
	if (server_query("GET PID", &pid) == 1 ||
		server_query("GET TIMESTAMP", &timestr) == 1)	{
		if (asprintf(&icon, "●") == -1) {
			return 1;
		}
		if (asprintf(&status, "inactive (dead)") == -1) {
			return 1;
		}
	}
	else {
		time_t rawtime;
		struct tm* timeinfo;
		rawtime = atoll(timestr);
		timeinfo = localtime(&rawtime);
		free(timestr);
		if ((timestr = malloc(sizeof(char) * 64)) == NULL) {
			return 1;
		}
		if (strftime(timestr, 64, "%a %F %T %Z", timeinfo) == -1) {
			return 1;
		}
		if (asprintf(&icon, "\e[0;92m●\e[0m") == -1) {
			return 1;
		}
		if (asprintf(&status, "\e[1;92mactive (running)\e[0m since %s", timestr) == -1) {
			return 1;
		}
		free(timestr);
	}
	printf("%s cinemad - The Reservation Cinema Server\n   Active: %s\n", icon, status);
	if (pid) {
		printf("Main PID : %s (cinemad)\n", pid);
		free(pid);
	}
	free(icon);
	free(status);
	return 0;
}

int server_query(char* query, char** result) {
	connection_t connection;
	char* filename;
	if (asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/tmp/socket") == -1) {
		return 1;
	}
	if ((connection = connection_init(filename, 0)) == NULL) {
		return 1;
	}
	free(filename);
	if (connetcion_connect(connection) == -1) {
		return 1;
	}
	if (connection_send(connection, query) == -1) {
		return 1;
	}
	if (connection_recv(connection, result) == -1) {
		return 1;
	}
	if (connection_close(connection) == -1) {
		return 1;
	}
	return 0;
}

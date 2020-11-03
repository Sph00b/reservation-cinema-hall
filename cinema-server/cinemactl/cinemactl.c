#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <connection.h>
#include <resources.h>
#include <try.h>

#define is_child(pid) !pid

#define COLOR_GREEN "\e[1;92m"
#define COLOR_DEFAULT "\e[0m"

static int usage();
static int server_start();
static int server_stop();
static int server_status();
static int server_query(char*, char**);

static int format_time_string(char** timestr);

int main(int argc, char* argv[]) {
	if (argc == 2 && !strncasecmp(argv[1], "start", 5)) {
		try(server_start(), 1, error);
	}
	else if (argc == 2 && !strncasecmp(argv[1], "stop", 4)) {
		try(server_stop(), 1, error);
	}
	else if (argc == 2 && !strncasecmp(argv[1], "status", 6)) {
		try(server_status(), 1, error);
	}
	else if (argc == 3 && !strncasecmp(argv[1], "query", 5)) {
		char* buff;
		try(server_query(argv[2], &buff), 1, error);
		printf("%s\n", buff);
		free(buff);
	}
	else {
		try(usage(), 1, error);
	}
	return 0;
error:
	fprintf(stderr, "%m\n");
	return 1;
}

static int usage() {
	try(
		fprintf(
			stderr,
			"\nUsage:\n cinemactl [COMMAND]\n\n\
			\rCommands:\n\
			\r start\n\
			\r stop\n\
			\r status\n\
			\r restart\n\
			\r query [...]\n\n"
		) < 0,
		!0,
		error
	);
	return 0;
error:
	return 1;
}

static int server_start(){
	pid_t pid;
	char *filename;
	try(asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/bin/cinemad"), -1, error);
	try(pid = fork(), -1, cleanup);
	if (is_child(pid)) {
		try(execl(filename, "cinemad", NULL), -1, cleanup);
	}
	return 0;
cleanup:
	free(filename);
error:
	return 1;
}

static int server_stop() {
	pid_t pid;
	char* result;

	try(server_query("GET PID", &result), 1, error);
	try(strtoi(result, &pid), 1, cleanup);
	free(result);

	try(kill(pid, SIGTERM), -1, error);

	return 0;
cleanup:
	free(result);
error:
	return 1;
}

/*
* This is seriously messed up, I guess I was drunk the day I designed it.
*/
static int server_status() {
	char* icon;
	char* status;

	char* result = NULL;
	int is_server_active = !server_query("TEST", &result);	// This must be changed
	free(result);

	if (is_server_active) {
		char* pid;
		char* timestr;
		try(server_query("GET PID", &pid), 1, error);
		try(server_query("GET TIMESTAMP", &timestr), 1, error);
		try(format_time_string(&timestr), 1, error);
		try(asprintf(&icon, COLOR_GREEN "●" COLOR_DEFAULT), -1, error);
		try(asprintf(&status, COLOR_GREEN "active (running) " COLOR_DEFAULT "since %s\nMain PID : %s (cinemad)", timestr, pid), -1, error);
		free(timestr);
		free(pid);
	}
	else {
		try(asprintf(&icon, "●"), -1, error);
		try(asprintf(&status, "inactive (dead)"), -1, error);	
	}
	try(printf("%s cinemad - The Reservation Cinema Server\n    Active: %s\n", icon, status) < 1, !0, error);
	free(icon);
	free(status);
	return 0;
error:
	return 1;
}

static int server_query(char* query, char** result) {
	connection_t connection;
	char* filename;
	try(asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/tmp/socket"), -1, error);
	try(connection = connection_init(filename, 0), NULL, cleanup);
	free(filename);
	try(connetcion_connect(connection), -1, error);
	try(connection_send(connection, query), -1, error);
	try(connection_recv(connection, result), -1, error);
	try(connection_close(connection), -1, error);
	return 0;
cleanup:
	free(filename);
error:
	return 1;
}

static int format_time_string(char** timestr) {
	time_t rawtime;
	struct tm* timeinfo;
	rawtime = atoll(*timestr);
	timeinfo = localtime(&rawtime);
	free(*timestr);
	try(*timestr = malloc(sizeof(char) * 64), NULL, error);
	try(strftime(*timestr, 64, "%a %F %T %Z", timeinfo), -1, error);
	return 0;
error:
	return 1;
}

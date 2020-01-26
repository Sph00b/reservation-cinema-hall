#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#include "asprintf.h"
#include "database.h"
#include "connection.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		syslog(LOG_ERR, "%m was generated from statment %s", #foo);\
		exit(EXIT_FAILURE);\
	}

database_t db;
connection_t con;

void *request_handler(void *);
int daemonize();
int dbcreate(database_t*, const char*);

int main(int argc, char *argv[]){
	pthread_t *tid = NULL;
	int tc = 0;	//thread counter
	int ret;
	char* r;
	char* ip;
	char* port;
	char* qpid;	//process id query
try(
	daemonize(), (1)
)
	syslog(LOG_DEBUG, "demonized");
try(
	mkdir("etc", 0775), (-1 * (errno != EEXIST))
)
try(
	ret = database_init(&db, "etc/data.dat"), (1 * (errno != ENOENT))
)
	if (ret && errno == ENOENT) {
try(
		dbcreate(&db, "etc/data.dat"), (1)
)
	}
	syslog(LOG_DEBUG, "connected to database");
try(
	asprintf(&qpid, "%s %d", "SET PID FROM CONFIG AS", getpid()), (-1)
)
try(
	database_execute(&db, qpid, &r), (1)
)
	free(qpid);
	syslog(LOG_DEBUG, "pid stored: %s", r);
try(
	database_execute(&db, "GET IP FROM NETWORK", &ip), (1)
)
try(
	database_execute(&db, "GET PORT FROM NETWORK", &port), (1)
)
try(
	connection_listener_start(&con, ip, atoi(port)), (1)
)
	syslog(LOG_DEBUG, "connected to the network");
	free(ip);
	free(port);
	syslog(LOG_INFO, "serving");
	do{
		connection_t accepted_connection;
try(
		connection_get_accepted(&con, &accepted_connection), (1)
)
		tc++;
try(
		tid = realloc(tid, sizeof(pthread_t) * tc), (NULL)
)
try(
		pthread_create(&tid[tc - 1], NULL, request_handler, (void*)&accepted_connection) , (!0)
)
	} while (1);
	for (int i = 0; i < tc; i++){
try(
		pthread_join(tid[i], NULL), (!0)
)
	}
try(
	connection_close(&con), (-1)
)
	free(tid);
try(
	database_close(&db), (!0)
)
	return 0;
}

void* request_handler(void* arg) {
	/*	todo: implement a timeout system	*/
	connection_t* connection;
	connection = (connection_t*)arg;
	char* buff;
	char* msg;
try(
	connection_recv(connection, &buff), (1)
)
try(
	database_execute(&db, buff, &msg), (1)
)
try(
	connection_send(connection, msg), (1)
)
try(
	connection_close(connection), (1)
)
	free(buff);
	free(msg);
	pthread_exit(0);
}

int daemonize() {
	pid_t pid;	//process id
	pid_t sid;	//session id
	char* wdir;	//working directory
	/* run process in backgound */
	if ((pid = fork()) == -1) {
		return 1;
	}
	if (pid != 0) {
		exit(EXIT_SUCCESS);
	}
	/* create a new session where process is group leader */
	if ((sid = setsid()) == -1) {
		return 1;
	}
	/* fork and kill group leader, lose control of terminal */
	if ((pid = fork()) == -1) {
		return 1;
	}
	if (pid != 0) {
		exit(EXIT_SUCCESS);
	}
	/* close standars stream */
	if (fclose(stdin) == EOF) {
		return 1;
	}
	if (fclose(stdout) == EOF) {
		return 1;
	}
	if (fclose(stderr) == EOF) {
		return 1;
	}
	/* change working directory */
	if (asprintf(&wdir, "%s%s", getenv("HOME"), "/.cinema") == -1) {
		return 1;
	}
	if (chdir(wdir) == -1) {
		return 1;
	}
	if (setenv("PWD", wdir, 1)) {
		return 1;
	}
	free(wdir);
	/* reset umask */
	umask(0);
	return 0;
}

int dbcreate(database_t* database, const char* filename) {
	char* r;
	char* msg_init[] = {
	"ADD NETWORK",
	"ADD IP FROM NETWORK",
	"SET IP FROM NETWORK AS 127.0.0.1",
	"ADD PORT FROM NETWORK",
	"SET PORT FROM NETWORK AS 55555",
	"ADD CONFIG",
	"ADD PID FROM CONFIG",
	"SET PID FROM CONFIG AS 0",
	"ADD ROWS FROM CONFIG",
	"SET ROWS FROM CONFIG AS 1",
	"ADD COLUMNS FROM CONFIG",
	"SET COLUMNS FROM CONFIG AS 1",
	"ADD DATA",
	NULL
	};
	int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
	close(dbfd);
	database_init(database, filename);
	for (int i = 0; msg_init[i]; i++) {
		if (database_execute(database, msg_init[i], &r)) {
			remove(filename);
			return 1;
		}
	}
	return 0;
}

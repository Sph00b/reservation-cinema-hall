#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "asprintf.h"
#include "database.h"
#include "connection.h"
#include "stack.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		syslog(LOG_ERR, "%m was generated from statment %s", #foo);\
		exit(EXIT_FAILURE);\
	}

/*	Data structures	*/

struct connection_info {
	connection_t* pconnection;
	char* address;
	char* port;
};

/*	Global variables	*/

database_t db;
_stack_t tid_stack;

/*	Prototype declarations of functions included in this code module	*/

void* thread_joiner(void* arg);
void* connection_mngr(void* arg);
void* request_handler(void* arg);
int daemonize();
int dbcreate(database_t* database, const char* filename);

void timeout(int sig) {
	pthread_exit(0);
}

int main(int argc, char *argv[]){
	pthread_t joiner_tid;
	pthread_t internet_mngr_tid;
	pthread_t internal_mngr_tid;
	connection_t internet_con;
	connection_t internal_con;
	struct connection_info internet_info;
	struct connection_info internal_info;
	sigset_t sigset;
	long server_status = 1;
	
	/*	Ignore all signals	*/
try(
	sigfillset(&sigset), (-1)
)
try(
	pthread_sigmask(SIG_BLOCK, &sigset, NULL), (!0)
)
	/*	Daemonize	*/
try(
	daemonize(), (1)
)
	syslog(LOG_DEBUG, "demonized");
	/*	Start joiner thread	*/
	tid_stack = NULL;
try(
	pthread_create(&joiner_tid, NULL, thread_joiner, (void*)&server_status), (!0)
)
	/*	Create directory tree	*/
try(
	mkdir("etc", 0775), (-1 * (errno != EEXIST))
)
try(
	mkdir("tmp", 0775), (-1 * (errno != EEXIST))
)
	/*	Start database	*/
	int ret;
try(
	ret = database_init(&db, "etc/data.dat"), (1 * (errno != ENOENT))
)
	if (ret && errno == ENOENT) {
try(
		dbcreate(&db, "etc/data.dat"), (1)
)
	}
	syslog(LOG_DEBUG, "Connected to database");
	/*	Register PID in database	*/
	char* qpid;
	char* result;
try(
	asprintf(&qpid, "%s %d", "SET PID FROM CONFIG AS", getpid()), (-1)
)
try(
	database_execute(&db, qpid, &result), (1)
)
	free(qpid);
	syslog(LOG_DEBUG, "PID stored: %s", result);
	/*	Prepare connection info variabales	*/
	internet_info.pconnection = &internet_con;
	internal_info.pconnection = &internal_con;
try(
	database_execute(&db, "GET IP FROM NETWORK", &internet_info.address), (1)
)
try(
	database_execute(&db, "GET PORT FROM NETWORK", &internet_info.port), (1)
)
try(
	asprintf(&internal_info.address, "%s%s", getenv("HOME"), "/.cinema/tmp/socket"), (-1)
)
try(
	asprintf(&internal_info.port, "0"), (-1)
)
	/*	Start connection manager threads	*/
try(
	pthread_create(&internet_mngr_tid, NULL, connection_mngr, (void*)&internet_info), (!0)
)
try(
	pthread_create(&internal_mngr_tid, NULL, connection_mngr, (void*)&internal_info), (!0)
)
	syslog(LOG_INFO, "Service started");
	/*	Wait for SIGTERM becomes pending	*/
	int sig;
try(
	sigemptyset(&sigset), (-1)
)
try(
	sigaddset(&sigset, SIGTERM), (-1)
)
try(
	sigwait(&sigset, &sig), (!0)
)
	/*	Send SIGALRM signal to connection manager threads	*/
try(
	pthread_kill(internet_mngr_tid, SIGALRM), (!0)
)
try(
	pthread_kill(internal_mngr_tid, SIGALRM), (!0)
)
	/*	Wait for threads return	*/
try(
	pthread_join(internet_mngr_tid, NULL), (!0)
)
try(
	pthread_join(internal_mngr_tid, NULL), (!0)
)
	server_status = 0;
try(
	pthread_join(joiner_tid, NULL), (!0)
)
	/*	Free memory	*/
	free(internet_info.address);
	free(internet_info.port);
	free(internal_info.address);
	free(internal_info.port);
	/*	Close connections and database	*/
try(
	connection_close(&internet_con), (-1)
)
try(
	connection_close(&internal_con), (-1)
)
try(
	database_close(&db), (!0)
)
	syslog(LOG_INFO, "Service stopped");
	return 0;
}

void* thread_joiner(void* arg) {
	long* server_status = (long*)arg;
	while (*server_status) {
		while(!stack_is_empty(&tid_stack)){
try(
			pthread_join(*(pthread_t*)stack_pop(&tid_stack), NULL), (!0)
)
		}
	}
	pthread_exit(0);
}

void* connection_mngr(void* arg) {
	struct connection_info* cinfo;
	cinfo = (struct connection_info*)arg;

	/*	Handle SIGALRM signal	*/
	sigset_t sigset;
	struct sigaction sigact;
try(
	sigemptyset(&sigset), (-1)
)
try(
	sigaddset(&sigset, SIGALRM), (-1)
)
try(
	pthread_sigmask(SIG_UNBLOCK, &sigset, NULL), (!0)
)
	sigact.sa_handler = timeout;
try(
	sigemptyset(&sigact.sa_mask), (-1)
)
	sigact.sa_flags = 0;
try(
	sigaction(SIGALRM, &sigact, NULL), (-1)
)
	/*	Setup connection	*/
try(
	connection_init(cinfo->pconnection, cinfo->address, atoi(cinfo->port)), (-1)
)
try(
	connection_listen(cinfo->pconnection), (-1)
)
	/*	Start threads to handle request	*/
	do {
		connection_t accepted_connection;
		pthread_t* tid;
try(
		connection_get_accepted(cinfo->pconnection, &accepted_connection), (-1)
)
try(
		tid = (pthread_t*)malloc(sizeof(pthread_t)), (NULL)
)
try(
		stack_push(&tid_stack, (void*)tid), (1)
)
try(
		pthread_create(tid, NULL, request_handler, (void*)&accepted_connection), (!0)
)
	} while (1);
}

void* request_handler(void* arg) {
	/*	todo: implement a timeout system alarm(time)?	*/
	connection_t* connection;
	connection = (connection_t*)arg;
	char* buff;
	char* msg;
try(
	connection_recv(connection, &buff), (-1)
)
try(
	database_execute(&db, buff, &msg), (-1)
)
try(
	connection_send(connection, msg), (-1)
)
try(
	connection_close(connection), (-1)
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

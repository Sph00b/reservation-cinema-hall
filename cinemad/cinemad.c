#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "asprintf.h"
#include "database.h"
#include "connection.h"
#include "queue.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		syslog(LOG_ERR, "%m was generated from statment %s", #foo);\
		exit(EXIT_FAILURE);\
	}

#define TIMEOUT 5

/*	Data structures	*/

struct connection_info {
	connection_t* pconnection;
	char* address;
	char* port;
};

struct request_info {
	pthread_t* ptid;
	connection_t* paccepted_connection;
};

/*	Global variables	*/

database_t db;
queue_t request_queue;
pthread_mutex_t request_queue_mutex;

/*	Prototype declarations of functions included in this code module	*/

void*	thread_joiner(void* arg);
void*	thread_timer(void* arg);
void*	connection_mngr(void* arg);
void*	request_handler(void* arg);
void	thread_exit(int sig) { pthread_exit(NULL); }	//SIAGALRM handler
int		daemonize();
int		dbcreate(database_t* database, const char* filename);

int main(int argc, char *argv[]){
	pthread_t joiner_tid;
	pthread_t internet_mngr_tid;
	pthread_t internal_mngr_tid;
	connection_t internet_con;
	connection_t internal_con;
	struct connection_info internet_info;
	struct connection_info internal_info;
	long server_status = 1;
	int ret;
	/*	Ignore all signals	*/
	sigset_t sigset;
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
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDemonized");
#endif
	/*	Initialize request queue and start joiner thread	*/
try(
	queue_init(&request_queue), (1)
)
try(
	pthread_mutex_init(&request_queue_mutex, NULL), (!0)
)
try(
	pthread_create(&joiner_tid, NULL, thread_joiner, (void*)&server_status), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tJoiner thread started");
#endif
	/*	Create directory tree	*/
try(
	mkdir("etc", 0775), (-1 * (errno != EEXIST))
)
try(
	mkdir("tmp", 0775), (-1 * (errno != EEXIST))
)
	/*	Start database	*/
try(
	ret = database_init(&db, "etc/data.dat"), (1 * (errno != ENOENT))
)
	if (ret && errno == ENOENT) {
try(
		dbcreate(&db, "etc/data.dat"), (1)
)
#ifdef _DEBUG
		syslog(LOG_DEBUG, "Main thread:\tDatabase created");
#endif
	}
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDatabase connected");
#endif
	/*	Register timestamp and PID in database	*/
	char* qpid;
	char* qtsp;
	char* result;
try(
	asprintf(&qpid, "%s %d", "SET PID FROM CONFIG AS", getpid()), (-1)
)
try(
	asprintf(&qtsp, "%s %llu", "SET TIMESTAMP FROM CONFIG AS", (long long)time(NULL)), (-1)
)
try(
	database_execute(&db, qpid, &result), (1)
)	
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tPID stored: %s", result);
#endif
try(
	database_execute(&db, qtsp, &result), (1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tTIMESTAMP stored: %s", result);
#endif
	free(qpid);
	free(qtsp);
	free(result);
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
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tConnection manager threads started");
#endif
	syslog(LOG_INFO, "Service started");
	/*	Wait for SIGTERM becomes pending	*/
	int sig;
try(
	sigemptyset(&sigset), (-1)
)
try(
	sigaddset(&sigset, SIGTERM), (-1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tWait for SIGTERM");
#endif
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
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tConnection manager threads joined");
#endif
	server_status = 0;
try(
	pthread_join(joiner_tid, NULL), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tAll thread joined");
#endif
	/*	Free connection info variabales	*/
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
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tClosed connections");
#endif
try(
	pthread_mutex_destroy(&request_queue_mutex), (!0)
)
try(
	database_close(&db), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tClosed database");
#endif
	syslog(LOG_INFO, "Service stopped");
	return 0;
}

void* thread_joiner(void* arg) {
	long* server_status = (long*)arg;
	struct request_info* request;
	while (*server_status) {
#ifdef _DEBUG
		syslog(LOG_DEBUG, "Joiner thread:\tQueue empty: %d", queue_is_empty(&request_queue));
#endif
		while(!queue_is_empty(&request_queue)){
#ifdef _DEBUG
			syslog(LOG_DEBUG, "Joiner thread:\tQueue empty: %d", queue_is_empty(&request_queue));
#endif
try(
			pthread_mutex_lock(&request_queue_mutex), (!0)
)
			request = queue_pop(&request_queue);
#ifdef _DEBUG
			syslog(LOG_DEBUG, "Joiner thread:\tClosing request thread %ul", *(request->ptid));
#endif
try(
			pthread_join(*(request->ptid), NULL), (!0)
)
			free(request->ptid);
			free(request->paccepted_connection);
			free(request);
try(
			pthread_mutex_unlock(&request_queue_mutex), (!0)
)
		}
		sleep(TIMEOUT);
	}

#ifdef _DEBUG
	syslog(LOG_DEBUG, "Joiner thread:\tClosing joiner thread, queue empty: %d", queue_is_empty(&request_queue));
#endif
	return NULL;
}

void* thread_timer(void* parent_tid) {
	/*	Capture SIGALRM signal	*/
	sigset_t sigalrm;
try(
	sigemptyset(&sigalrm), (-1)
)
try(
	sigaddset(&sigalrm, SIGALRM), (-1)
)
try(
	pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	/*	Send SIGALRM after TIMEOUT elapsed	*/
	sleep(TIMEOUT);
try(
	pthread_kill((pthread_t)parent_tid, SIGALRM), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Timer thread:\tSended SIGALRM to request thread %ul", (pthread_t)parent_tid);
#endif
	/*	Detach thread	*/
try(
	pthread_detach((pthread_t)parent_tid), (!0)
)
	return NULL;
}

void* connection_mngr(void* arg) {
	struct connection_info* cinfo;
	cinfo = (struct connection_info*)arg;

	/*	Setup SIGALRM signal handler	*/
	sigset_t sigalrm;
	struct sigaction sigact;
try(
	sigemptyset(&sigalrm), (-1)
)
try(
	sigaddset(&sigalrm, SIGALRM), (-1)
)
	sigact.sa_handler = thread_exit;
	sigact.sa_mask = sigalrm;
	sigact.sa_flags = 0;
try(
	sigaction(SIGALRM, &sigact, NULL), (-1)
)
	/*	Capture SIGALRM signal	*/
try(
	pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	/*	Setup connection	*/
try(
	connection_init(cinfo->pconnection, cinfo->address, (uint16_t)atoi(cinfo->port)), (-1)
)
try(
	connection_listen(cinfo->pconnection), (-1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "CntMng thread:\tlistening on socket");
#endif
	/*	Start request handler threads	*/
	do {
		struct request_info* request;
try(
		request = malloc(sizeof(struct request_info)), (NULL)
)
try(
		request->paccepted_connection = malloc(sizeof(connection_t)), (NULL)
)
try(
		connection_get_accepted(cinfo->pconnection, request->paccepted_connection), (-1)
)
	/*	Ignore SIGALRM signal	*/
try(
		pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), (!0)
)
	/*	Create a new thread and register it in the queue	*/
try(
		request->ptid = malloc(sizeof(pthread_t)), (NULL)
)
try(
		pthread_mutex_lock(&request_queue_mutex), (!0)
)
try(
		queue_push(&request_queue, (void*)request), (1)
)
try(
		pthread_mutex_unlock(&request_queue_mutex), (!0)
)
try(
		pthread_create(request->ptid, NULL, request_handler, (void*)request), (!0)
)
#ifdef _DEBUG
		syslog(LOG_DEBUG, "CntMng thread:\tRequest thread %ul spowned", *(request->ptid));
#endif
	/*	Capture SIGALRM signal	*/
try(
		pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	} while (1);
}

void* request_handler(void* arg) {
	pthread_t timer_tid;
	struct request_info* info;
	info = (struct request_info*)arg;
	char* buff;
	char* msg;
	/*	Start timeout thread	*/
try(
	pthread_create(&timer_tid, NULL, thread_timer, (void*)*(info->ptid)), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\tCreated timer thread");
#endif
/*	Capture SIGALRM signal	*/
	sigset_t sigalrm;
try(
	sigemptyset(&sigalrm), (-1)
)
try(
	sigaddset(&sigalrm, SIGALRM), (-1)
)
try(
	pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	/*	Get the request	*/
try(
	connection_recv(info->paccepted_connection, &buff), (-1)
)
	/*	Ignore SIGALRM	and stop timeout thread	*/
try(
	pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), (!0)
)
try(
	pthread_kill(timer_tid, SIGALRM), (!0)
)
try(
	pthread_join(timer_tid, NULL), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\tStopped timer thread");
#endif
	/*	Elaborate the response */
try(
	database_execute(&db, buff, &msg), (-1)
)
	free(buff);
	/*	Send the response	*/
try(
	connection_send(info->paccepted_connection, msg), (-1)
)
	free(msg);
	/*	Close connection	*/
try(
	connection_close(info->paccepted_connection), (-1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%ul ready to exit", *(info->ptid));
#endif
	return NULL;
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
	"ADD TIMESTAMP FROM CONFIG",
	"SET TIMESTAMP FROM CONFIG AS 0",
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

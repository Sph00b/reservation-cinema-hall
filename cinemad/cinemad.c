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
#include "concurrent_queue.h"
#include "concurrent_flag.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		syslog(LOG_ERR, "%m was generated from statment %s", #foo);\
		exit(EXIT_FAILURE);\
	}

#define TIMEOUT 5

struct request_info {
	pthread_t tid;
	connection_t connection;
};

/*	Global variables	*/

database_t database;
concurrent_queue_t request_queue;

/*	Prototype declarations of functions included in this code module	*/

void	thread_exit(int sig) { pthread_exit(NULL); }	//SIAGALRM handler
void*	thread_joiner(void* arg);
void*	thread_timer(void* arg);
void*	connection_mngr(void* arg);
void*	request_handler(void* arg);
int		daemonize();

int main(int argc, char *argv[]){
	pthread_t joiner_tid;
	pthread_t internet_mngr_tid;
	pthread_t internal_mngr_tid;
	connection_t internet_connection;
	connection_t internal_connection;
	concurrent_flag_t server_status_flag;
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
	/*	Initialize status flag, request queue and start joiner thread	*/
try(
	request_queue = concurrent_queue_init(), (NULL)
)
try(
	server_status_flag = concurrent_flag_init(), (NULL)
)
try(
	concurrent_flag_set(server_status_flag), (1)
)
try(
	pthread_create(&joiner_tid, NULL, thread_joiner, server_status_flag), (!0)
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
	char* result;
try(
	database = database_init("etc/data.dat"), (NULL || (errno == ENOENT))
)
	if (!database) {
		int fd;
try(
		fd = open("etc/data.dat", O_RDWR | O_CREAT | O_EXCL, 0666), (-1)
)
try(
		database = database_init("etc/data.dat"), (NULL)
)
try(
		database_execute(database, "POPULATE", &result), (1)
)
		free(result);
	}
try(
	database_execute(database, "SETUP", &result), (1)
)
	free(result);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDatabase connected");
#endif
	/*	Register timestamp and PID in database	*/
	char* qpid;
	char* qtsp;
try(
	asprintf(&qpid, "%s %d", "SET PID", getpid()), (-1)
)
try(
	asprintf(&qtsp, "%s %llu", "SET TIMESTAMP", (long long)time(NULL)), (-1)
)
try(
	database_execute(database, qpid, &result), (1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tPID stored: %s", result);
#endif
	free(result);
try(
	database_execute(database, qtsp, &result), (1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tTIMESTAMP stored: %s", result);
#endif
	free(result);
	free(qpid);
	free(qtsp);
	/*	Setup connections	*/
	char* address;
	char* port;
try(
	database_execute(database, "GET IP", &address), (1)
)
try(
	database_execute(database, "GET PORT", &port), (1)
)
try(
	internet_connection = connection_init(address, (uint16_t)atoi(port)), (NULL)
)
	free(address);
	free(port);
try(
	asprintf(&address, "%s%s", getenv("HOME"), "/.cinema/tmp/socket"), (-1)
)
try(
	internal_connection = connection_init(address, 0), (NULL)
)
	free(address);
	/*	Start connection manager threads	*/
try(
	pthread_create(&internet_mngr_tid, NULL, connection_mngr, internet_connection), (!0)
)
try(
	pthread_create(&internal_mngr_tid, NULL, connection_mngr, internal_connection), (!0)
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
try(
	concurrent_flag_unset(server_status_flag), (1)
)
try(
	pthread_join(joiner_tid, NULL), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tAll thread joined");
#endif
	/*	Free queue and flag	*/
try(
	concurrent_queue_destroy(request_queue), (1)
)
try(
	concurrent_flag_destroy(server_status_flag), (1)
)
	/*	Close connections and database	*/
try(
	connection_close(internet_connection), (-1)
)
try(
	connection_close(internal_connection), (-1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tClosed connections");
#endif
try(
	database_close(database), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tClosed database");
#endif
	syslog(LOG_INFO, "Service stopped");
	return 0;
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

void* thread_joiner(void* arg) {
	concurrent_flag_t server_status_flag = arg;
	int status;
	int queue_empty;
try(
	concurrent_flag_status(server_status_flag, &status), (1)
)
try(
	concurrent_queue_is_empty(request_queue, &queue_empty), (1)
)
	while (status || !queue_empty) {
		while(!queue_empty){
			struct request_info* info;
try(
			concurrent_queue_dequeue(request_queue, (void**)&info), (1)
)
try(
			pthread_join(info->tid, NULL), (!0)
)
try(
			connection_close(info->connection), (-1)
)
#ifdef _DEBUG
syslog(LOG_DEBUG, "Joiner thread:\tJoined request thread %ul", info->tid);
#endif
			free(info);
try(
			concurrent_queue_is_empty(request_queue, &queue_empty), (1)
)
		}
		sleep(1);
try(
		concurrent_flag_status(server_status_flag, &status), (1)
)
try(
		concurrent_queue_is_empty(request_queue, &queue_empty), (1)
)
	}
#ifdef _DEBUG
try(
	concurrent_queue_is_empty(request_queue, &queue_empty), (1)
)
	syslog(LOG_DEBUG, "Joiner thread:\tClosing joiner thread, queue empty: %d", queue_empty);
#endif
	return NULL;
}

void* thread_timer(void* arg) {
	pthread_t parent_tid = (pthread_t)arg;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Timer thread:\tSpowned");
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
	/*	Send SIGALRM after TIMEOUT elapsed	*/
	sleep(TIMEOUT);
try(
	pthread_kill(parent_tid, SIGALRM), (!0)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Timer thread:\tSended SIGALRM to request thread %ul", parent_tid);
#endif
	/*	Detach thread	*/
try(
	pthread_detach(parent_tid), (!0)
)
	return NULL;
}

void* connection_mngr(void* arg) {
	connection_t connection = arg;

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
	/*	Start listen on connection	*/
try(
	connection_listen(connection), (-1)
)
#ifdef _DEBUG
	syslog(LOG_DEBUG, "CntMng thread:\tlistening on socket");
#endif
	while (1) {
	/*	Wait for incoming connection	*/
		connection_t accepted_connection;
try(
		accepted_connection = connection_accepted(connection), (NULL)
)
	/*	Ignore SIGALRM signal	*/
try(
		pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), (!0)
)
	/*	Create request handler thread	*/
		struct request_info* accepted_request;
try(
		accepted_request = malloc(sizeof(struct request_info)), (NULL)
)
		accepted_request->connection = accepted_connection;
try(
		pthread_create(&accepted_request->tid, NULL, request_handler, accepted_connection), (!0)
)
	/*	Register thread in the queue	*/
try(
		concurrent_queue_enqueue(request_queue, (void*)accepted_request), (1)
)
	/*	Capture SIGALRM signal	*/
try(
		pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	}
}

void* request_handler(void* arg) {
	connection_t connection = arg;
	pthread_t timer_tid;
	char* buff;
	char* msg;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%ul spowned", pthread_self());
#endif
	/*	Start timeout thread	*/
try(
	pthread_create(&timer_tid, NULL, thread_timer, (void*)pthread_self()), (!0)
)
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
	connection_recv(connection, &buff), (-1)
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
	database_execute(database, buff, &msg), (1)
)
	free(buff);
	/*	Send the response	*/
try(
	connection_send(connection, msg), (-1)	//ECONNRESET?
)
	free(msg);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%ul ready to exit", pthread_self());
#endif
	return NULL;
}

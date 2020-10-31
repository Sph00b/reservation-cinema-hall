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

#include "utils.h"
#include "database.h"

#include <resources.h>
#include <connection.h>
#include <data-structure/concurrent_flag.h>
#include <data-structure/concurrent_queue.h>

#include "try.h"

#define TIMEOUT 5

struct request_info {
	pthread_t tid;
	connection_t connection;
};

// Global variables

database_t database;
concurrent_queue_t request_queue;

// Prototype declarations of functions included in this code module

void thread_exit(int sig) { pthread_exit(NULL); }	// SIAGALRM handler
void* thread_joiner(void* arg);
void* thread_timer(void* arg);
void* connection_mngr(void* arg);
void* request_handler(void* arg);

static int setup_workspace(void);
static int connect_database(void);
static int setup_database(void);
static int setup_internet_connection(connection_t *connection);
static int setup_internal_connection(connection_t *connection);

int main(int argc, char *argv[]){
	pthread_t joiner_tid;
	pthread_t internet_mngr_tid;
	pthread_t internal_mngr_tid;
	connection_t internet_connection;
	connection_t internal_connection;
	concurrent_flag_t is_server_running;

	try(request_queue = concurrent_queue_init(), NULL);
	try(is_server_running = concurrent_flag_init(), NULL);
	try(daemonize(), 1);

	try(signal_ignore_all(), 1);
	try(setup_workspace(), 1);
	try(connect_database(), 1);
	try(setup_database(), 1);
	try(setup_internal_connection(&internal_connection), 1);
	try(setup_internet_connection(&internet_connection), 1);
	try(concurrent_flag_set(is_server_running), 1);
	try(pthread_create(&joiner_tid, NULL, thread_joiner, is_server_running), !0);
	try(pthread_create(&internal_mngr_tid, NULL, connection_mngr, internal_connection), !0);
	try(pthread_create(&internet_mngr_tid, NULL, connection_mngr, internet_connection), !0);

	syslog(LOG_INFO, "Service started");

	try(signal_wait(SIGTERM), 1);

	try(pthread_kill(internet_mngr_tid, SIGALRM), !0);
	try(pthread_kill(internal_mngr_tid, SIGALRM), !0);
	try(pthread_join(internet_mngr_tid, NULL), !0);
	try(pthread_join(internal_mngr_tid, NULL), !0);

	try(concurrent_flag_unset(is_server_running), 1);
	try(pthread_join(joiner_tid, NULL), !0);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tAll thread joined");
#endif
	try(connection_close(internet_connection), -1);
	try(connection_close(internal_connection), -1);
	try(database_close(database), !0);
	try(concurrent_flag_destroy(is_server_running), 1);
	try(concurrent_queue_destroy(request_queue), 1);

	syslog(LOG_INFO, "Service stopped");

	return 0;
}

static int setup_workspace(void) {
	try(mkdir("etc", 0775), -1 * (errno != EEXIST), error);
	try(mkdir("tmp", 0775), -1 * (errno != EEXIST), error);
	return 0;
error:
	return 1;
}

static int connect_database(void) {
	try(database = database_init("etc/data.dat"), NULL || (errno == ENOENT), error);
	if (!database) {
		int fd;
		try(fd = open("etc/data.dat", O_RDWR | O_CREAT | O_EXCL, 0660), -1, error);
		try(close(fd), -1, error);

		try(database = database_init("etc/data.dat"), NULL, error);

		char* result;
		try(database_execute(database, "POPULATE", &result), 1, error);
		free(result);
	}

#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDatabase connected");
#endif
	return 0;
error:
	return 1;
}

static int setup_database(void) {
	char* result;
	char* qpid;
	char* qtsp;
	try(asprintf(&qpid, "%s %d", "SET PID", getpid()), -1, error);
	try(asprintf(&qtsp, "%s %llu", "SET TIMESTAMP", (long long)time(NULL)), -1, error);

	try(database_execute(database, "SETUP", &result), 1, error);
	free(result);
	try(database_execute(database, qpid, &result), 1, error);
	free(result);
	try(database_execute(database, qtsp, &result), 1, error);
	free(result);
	free(qpid);
	free(qtsp);

#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDatabase ready");
#endif
	return 0;
error:
	return 1;
}

static int setup_internet_connection(connection_t* connection) {
	char* address;
	char* port;
	try(database_execute(database, "GET IP", &address), 1, error);
	try(database_execute(database, "GET PORT", &port), 1, error);
	int port_value;
	try(strtoi(port, &port_value), 1, error);
	try(*connection = connection_init(address, (uint16_t)port_value), NULL, error);
	free(address);
	free(port);
	return 0;
error:
	return 1;
}

static int setup_internal_connection(connection_t* connection) {
	char* address;
	try(asprintf(&address, "%s%s", getenv("HOME"), "/.cinema/tmp/socket"), -1, error);
	try(*connection = connection_init(address, 0), NULL, error);
	free(address);
	return 0;
error:
	return 1;
}

void* thread_joiner(void* arg) {
	concurrent_flag_t server_status_flag = arg;
	int status;
	int queue_empty;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Joiner thread:\tstarted");
#endif
	try(concurrent_flag_status(server_status_flag, &status), 1);
	try(concurrent_queue_is_empty(request_queue, &queue_empty), 1);
	while (status || !queue_empty) {
		while (!queue_empty) {
			struct request_info* info;
			try(concurrent_queue_dequeue(request_queue, (void**)&info), 1);
			try(pthread_join(info->tid, NULL), !0);
			try(connection_close(info->connection), -1);
#ifdef _DEBUG
			syslog(LOG_DEBUG, "Joiner thread:\tJoined request thread %lud", (unsigned long int)info->tid);
#endif
			free(info);
			try(concurrent_queue_is_empty(request_queue, &queue_empty), 1);
		}
		sleep(1);
		try(concurrent_flag_status(server_status_flag, &status), 1);
		try(concurrent_queue_is_empty(request_queue, &queue_empty), 1);
	}
#ifdef _DEBUG
	try(concurrent_queue_is_empty(request_queue, &queue_empty), 1);
	syslog(LOG_DEBUG, "Joiner thread:\tClosing joiner thread, queue empty: %d", queue_empty);
#endif
	return NULL;
}

void* thread_timer(void* arg) {
	pthread_t parent_tid = (pthread_t)arg;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Timer thread:\tSpowned");
#endif
	// Capture SIGALRM signal
	sigset_t sigalrm;
	try(sigemptyset(&sigalrm), -1);
	try(sigaddset(&sigalrm, SIGALRM), -1);
	try(pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), !0);
	// Send SIGALRM after TIMEOUT elapsed
	sleep(TIMEOUT);
	try(pthread_kill(parent_tid, SIGALRM), !0);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Timer thread:\tSended SIGALRM to request thread %lud", (unsigned long int)parent_tid);
#endif
	try(pthread_detach(parent_tid), !0);
	return NULL;
}

void* connection_mngr(void* arg) {
	connection_t connection = arg;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tConnection manager thread started");
#endif
	// Setup SIGALRM signal handler
	sigset_t sigalrm;
	struct sigaction sigact;
	try(sigemptyset(&sigalrm), -1);
	try(sigaddset(&sigalrm, SIGALRM), -1);
	sigact.sa_handler = thread_exit;
	sigact.sa_mask = sigalrm;
	sigact.sa_flags = 0;
	try(sigaction(SIGALRM, &sigact, NULL), -1);

	try(pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), !0);

	try(connection_listen(connection), -1);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "CntMng thread:\tlistening on socket");
#endif
	while (1) {
		connection_t accepted_connection;
		try(accepted_connection = connection_accepted(connection), NULL);

		try(pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), !0);

		// Create request handler thread
		struct request_info* accepted_request;
		try(accepted_request = malloc(sizeof * accepted_connection), NULL);
		accepted_request->connection = accepted_connection;
		try(pthread_create(&accepted_request->tid, NULL, request_handler, accepted_connection), !0);
		try(concurrent_queue_enqueue(request_queue, (void*)accepted_request), 1);

		try(pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), !0);
	}
}

void* request_handler(void* arg) {
	connection_t connection = arg;
	pthread_t timer_tid;
	sigset_t sigalrm;
	char* request;
	char* response;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%lud spowned", (unsigned long int)pthread_self());
#endif
	try(sigemptyset(&sigalrm), -1);
	try(sigaddset(&sigalrm, SIGALRM), -1);

	try(pthread_create(&timer_tid, NULL, thread_timer, (void*)pthread_self()), !0);

	try(pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), !0);
	try(connection_recv(connection, &request), -1);
	try(pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), !0);

	try(pthread_kill(timer_tid, SIGALRM), !0);
	try(pthread_join(timer_tid, NULL), !0);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\tStopped timer thread");
#endif
	// Elaborate the response
	try(database_execute(database, request, &response), 1);
	free(request);
	// Send the response
	try(connection_send(connection, response), -1 - (errno == ECONNRESET) - (errno == EPIPE));
	free(response);
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%lud ready to exit", (unsigned long int)pthread_self());
#endif
	return NULL;
}

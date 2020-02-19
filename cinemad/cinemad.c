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
pthread_rwlock_t db_serializing_mutex;
concurrent_queue_t request_queue;
int rows;
int columns;

/*	Prototype declarations of functions included in this code module	*/

void*	thread_joiner(void* arg);
void*	thread_timer(void* arg);
void*	connection_mngr(void* arg);
void*	request_handler(void* arg);
void	thread_exit(int sig) { pthread_exit(NULL); }	//SIAGALRM handler
int		daemonize();
int		db_create(const char* filename);
int		db_configure();
int		db_clean_data();
int		db_book(const char* request, char** result);
int		db_unbook(const char* request, char** result);
int		db_send_status(const char* request, char** result);

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
try(
	database = database_init("etc/data.dat"), (NULL || (errno == ENOENT))
)
	if (!database) {
try(
		db_create("etc/data.dat"), (1)
)
#ifdef _DEBUG
		syslog(LOG_DEBUG, "Main thread:\tDatabase created");
#endif
	}
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Main thread:\tDatabase connected");
#endif
	char* result;
try(
	database_execute(database, "GET ROWS", &result), (1)
)
	rows = atoi(result);
	free(result);
try(
	database_execute(database, "GET COLUMNS", &result), (1)
)
	columns = atoi(result);
	free(result);

try(
	db_configure(), (1)
)
	/*	Register timestamp and PID in database	*/
	char* qpid;
	char* qtsp;
try(
	asprintf(&qpid, "%s %d", "SET PID AS", getpid()), (-1)
)
try(
	asprintf(&qtsp, "%s %llu", "SET TIMESTAMP AS", (long long)time(NULL)), (-1)
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

	do {
		pthread_t tid;
		connection_t accepted_connection;
	/*	Wait for incoming connection	*/
try(
		accepted_connection = connection_accepted(connection), (NULL)
)
	/*	Ignore SIGALRM signal	*/
try(
		pthread_sigmask(SIG_BLOCK, &sigalrm, NULL), (!0)
)
	/*	Create request handler thread	*/
try(
		pthread_create(&tid, NULL, request_handler, accepted_connection), (!0)
)
	/*	Capture SIGALRM signal	*/
try(
		pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL), (!0)
)
	} while (1);
}

void* request_handler(void* arg) {
	struct request_info* info;
	connection_t connection = arg;
	pthread_t timer_tid;
	char* buff;
	char* msg;
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%ul spowned", pthread_self());
#endif
	/*	Reister thread in the queue	*/
	/*	Put value in a structure	*/
try(
	info = malloc(sizeof(struct request_info)), (NULL)
)
	info->tid = pthread_self();
	info->connection = connection;
try(
	concurrent_queue_enqueue(request_queue, (void*)info), (1)
)
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
{
	if (buff[0] == '#') {
try(
		pthread_rwlock_wrlock(&db_serializing_mutex), (!0)
)
try(
		db_book(buff + 1, &msg), (1)
)
	}
	else if (buff[0] == '@') {
try(
		pthread_rwlock_wrlock(&db_serializing_mutex), (!0)
)
try(
		db_unbook(buff + 1, &msg), (1)
)
	}
	else if (buff[0] == '~') {
try(
		pthread_rwlock_rdlock(&db_serializing_mutex), (!0)
)
try(
		db_send_status(buff + 1, &msg), (1)
)
	}
	else {
try(
		pthread_rwlock_rdlock(&db_serializing_mutex), (!0)
)
try(
		database_execute(database, buff, &msg), (1)
)
	}
try(
	pthread_rwlock_unlock(&db_serializing_mutex), (!0)
)
	free(buff);
}
	/*	Send the response	*/
	connection_send(connection, msg);
	free(msg);
	/*	Close connection	*/
#ifdef _DEBUG
	syslog(LOG_DEBUG, "Request thread:\t%ul ready to exit", pthread_self());
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

int db_create(const char* filename) {
	char* msg_init[] = {
	"SET IP AS 127.0.0.1",
	"SET PORT AS 55555",
	"SET PID AS 0",
	"SET TIMESTAMP AS 0",
	"SET ROWS AS 1",
	"SET COLUMNS AS 1",
	"SET FILM AS Titolo",
	"SET SHOWTIME AS 00:00",
	"SET ID_COUNTER AS 0",
	"SET 0 AS 0",
	NULL
	};
	int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
	close(dbfd);
	if ((database = database_init(filename)) == NULL) {
		return 1;
	}
	char* result;
	for (int i = 0; msg_init[i]; i++) {
		if (database_execute(database, msg_init[i], &result)) {
			remove(filename);
			free(result);
			return 1;
		}
		free(result);
	}
	return 0;
}

int db_configure() {
	char* result;
	int clean = 0;
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "GET %d", i) == -1) {
			return 1;	
		}
		if (database_execute(database, query, &result) == 1) {
			return 1;
		}
		if (!strncmp(result, DBMSG_FAIL, strlen(DBMSG_FAIL))) {
			clean = 1;
			free(query);
			free(result);
			if (asprintf(&query, "SET %d AS 0", i) == -1) {
				return 1;
			}
			if (database_execute(database, query, &result) == 1) {
				return 1;
			}
		}
		free(query);
		free(result);
	}
	if (clean) {
		if (db_clean_data(database)) {
			return 1;
		}
	}
	return 0;
}

int db_clean_data() {
	char* result;
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "SET %d AS 0", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, &result) == 1) {
			return 1;
		}
		free(query);
		free(result);
	}
	if (database_execute(database, "SET ID_COUNTER AS 0", &result) == 1) {
		return 1;
	}
	free(result);
	return 0;
}

int db_get_id() {
	int id;
	char* query;
	char* result;
	if (database_execute(database, "GET ID_COUNTER", &result) == 1) {
		return -1;
	}
	id = atoi(result);
	id++;
	free(result);
	if (asprintf(&query, "SET ID_COUNTER AS %d", id) == -1) {
		return -1;
	}
	if (database_execute(database, query, &result) == 1) {
		free(query);
		return -1;
	}
	free(query);
	free(result);
	return id;
}

/*	Should I start threads to increase performance?	*/

int db_send_status(const char* request, char** result) {
	int id = 0;
	char* query = NULL;
	char* buffer = NULL;
	char* ptr;
	id = !strlen(request) ? -1 : atoi(request);
	if (!(rows && columns)) {
		asprintf(result, "");
		return 0;
	}
	database_execute(database, "GET 0", &buffer);
	if (atoi(buffer)) {
		if (atoi(buffer) == id) {
			free(buffer);
			asprintf(&buffer, "1");
		}
		else {
			free(buffer);
			asprintf(&buffer, "2");
		}
	}
	asprintf(result, "%s", buffer);
	for (int i = 1; i < rows * columns; i++) {
		asprintf(&query, "GET %d", i);
		database_execute(database, query, &buffer);
		ptr = &(**result);
		if (atoi(buffer)) {
			if (atoi(buffer) == id) {
				free(buffer);
				asprintf(&buffer, "1");
			}
			else {
				free(buffer);
				asprintf(&buffer, "2");
			}
		}
		asprintf(result, "%s %s", *result, buffer);
		free(ptr);
		free(query);
		free(buffer);
	}
	return 0;
}

/*	# ID LIST OF DESIRED SEATS SEPARED WITH SPACE, ID = 0 IF NO ID*/
int db_book(const char* request, char **result) {
	int id;
	int ret;
	int ntoken;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;
	char** rquery = NULL;
	char** wquery = NULL;

	ntoken = 0;
	if ((buffer = strdup(request)) == NULL) {
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	if (!strcmp(token[0],"0")) {
		if ((id = db_get_id(database)) == -1) {
			return 1;
		}
	}
	else {
		id = atoi(token[0]);
	}
	ntoken--;
	/*	free memory on error	*/
	if ((rquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	if ((wquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	for (int i = 0; i < ntoken; i++) {
		if (asprintf(&(rquery[i]), "GET %s", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s AS %d", token[i + 1], id) == -1) {
			return 1;
		}
	}
	free(buffer);
	free(token);
	ret = 0;
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, rquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
		if (strcmp(*result, "0")) {
			ret = 1;
		}
		free(rquery[i]);
		free(*result);
		if (ret) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
	}
	free(rquery);
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, wquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(wquery[++j]));
			free(wquery);
			return 1;
		}
		free(wquery[i]);
		free(*result);
	}
	free(wquery);
	if (asprintf(result, "%d", id) == -1) {
		return 1;
	}
	return 0;
}

int db_unbook(const char* request, char** result) {
	int ret;
	int ntoken;
	char* id;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;
	char** rquery = NULL;
	char** wquery = NULL;

	ntoken = 0;
	if ((buffer = strdup(request)) == NULL) {
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	token[0] = strtok_r(buffer, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	id = strdup(*token);
	ntoken--;
	/*	free memory on error	*/
	if ((rquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	if ((wquery = malloc(sizeof(char*) * (size_t)(ntoken))) == NULL) {
		return 1;
	}
	for (int i = 0; i < ntoken; i++) {
		if (asprintf(&(rquery[i]), "GET %s", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s AS 0", token[i + 1]) == -1) {
			return 1;
		}
	}
	free(buffer);
	free(token);
	ret = 0;
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, rquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			return 1;
		}
		if (strcmp(*result, id)) {
			ret = 1;
		}
		free(rquery[i]);
		free(*result);
		if (ret) {
			for (int j = i; j < ntoken; free(rquery[++j]));
			for (int j = 0; j < ntoken; free(wquery[j++]));
			free(rquery);
			free(wquery);
			free(id);
			return 1;
		}
	}
	free(rquery);
	free(id);
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, wquery[i], result) == 1) {
			for (int j = i; j < ntoken; free(wquery[++j]));
			free(wquery);
			return 1;
		}
		free(wquery[i]);
		free(*result);
	}
	free(wquery);
	if (asprintf(result, DBMSG_SUCC) == -1) {
		return 1;
	}
	return 0;
}

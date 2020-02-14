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
pthread_rwlock_t db_serializing_mutex;
queue_t request_queue;
pthread_mutex_t request_queue_mutex;
int rows;
int columns;

/*	Prototype declarations of functions included in this code module	*/

void*	thread_joiner(void* arg);
void*	thread_timer(void* arg);
void*	connection_mngr(void* arg);
void*	request_handler(void* arg);
void	thread_exit(int sig) { pthread_exit(NULL); }	//SIAGALRM handler
int		daemonize();
int		db_create(database_t* database, const char* filename);
int		db_configure(database_t* database);
int		db_clean_data(database_t* database);
int		db_book(database_t* database, const char* request, char** result);
int		db_unbook(database_t* database, const char* request, char** result);
int		db_send_status(database_t* database, const char* request, char** result);

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
		db_create(&db, "etc/data.dat"), (1)
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
	database_execute(&db, "GET ROWS FROM CONFIG", &result), (1)
)
	rows = atoi(result);
	free(result);
try(
	database_execute(&db, "GET COLUMNS FROM CONFIG", &result), (1)
)
	columns = atoi(result);
	free(result);

try(
	db_configure(&db), (1)
)
	/*	Register timestamp and PID in database	*/
	char* qpid;
	char* qtsp;
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
	free(result);
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
		while(!queue_is_empty(&request_queue)){
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
		sleep(1);
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
	if (buff[0] == '#') {
try(
		pthread_rwlock_wrlock(&db_serializing_mutex), (!0)
)
try(
		db_book(&db, buff + 1, &msg), (1)
)
	}
	else if (buff[0] == '@') {
try(
		pthread_rwlock_wrlock(&db_serializing_mutex), (!0)
)
try(
		db_unbook(&db, buff + 1, &msg), (1)
)
	}
	else if (buff[0] == '~') {
try(
		pthread_rwlock_rdlock(&db_serializing_mutex), (!0)
)
try(
		db_send_status(&db, buff + 1, &msg), (1)
)
	}
	else {
try(
		pthread_rwlock_rdlock(&db_serializing_mutex), (!0)
)
try(
		database_execute(&db, buff, &msg), (-1)
)
	}
try(
	pthread_rwlock_unlock(&db_serializing_mutex), (!0)
)
	free(buff);
	/*	Send the response	*/
	/*	Handle broken pipe!	*/
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

int db_create(database_t* database, const char* filename) {
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
	"ADD FILM FROM CONFIG",
	"ADD SHOWTIME FROM CONFIG",
	"SET SHOWTIME FROM CONFIG AS 00:00",
	"ADD ID_COUNTER FROM CONFIG",
	"SET ID_COUNTER FROM CONFIG AS 0",
	"ADD DATA",
	"ADD 0 FROM DATA",
	"SET 0 FROM DATA AS 0",
	NULL
	};
	int dbfd = open(filename, O_CREAT | O_EXCL, 0666);
	close(dbfd);
	database_init(database, filename);
	char* r;
	for (int i = 0; msg_init[i]; i++) {
		if (database_execute(database, msg_init[i], &r)) {
			remove(filename);
			free(r);
			return 1;
		}
		free(r);
	}
	return 0;
}

int db_configure(database_t* database) {
	char* result;
	int clean = 0;
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "GET %d FROM DATA", i) == -1) {
			return 1;	
		}
		if (database_execute(database, query, &result) == 1) {
			return 1;
		}
		if (!strncmp(result, DBMSG_FAIL, strlen(DBMSG_FAIL))) {
			clean = 1;
			free(query);
			free(result);
			if (asprintf(&query, "ADD %d FROM DATA", i) == -1) {
				return 1;
			}
			if (database_execute(database, query, &result) == 1) {
				return 1;
			}
			free(query);
			free(result);
			if (asprintf(&query, "SET %d FROM DATA AS 0", i) == -1) {
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

int db_clean_data(database_t* database) {
	char* result;
	for (int i = 0; i < rows * columns; i++) {
		char* query;
		if (asprintf(&query, "SET %d FROM DATA AS 0", i) == -1) {
			return 1;
		}
		if (database_execute(database, query, &result) == 1) {
			return 1;
		}
		free(query);
		free(result);
	}
	if (database_execute(database, "SET ID_COUNTER FROM CONFIG AS 0", &result) == 1) {
		return 1;
	}
	free(result);
	return 0;
}

int db_get_id(database_t* database) {
	int id;
	char* query;
	char* result;
	if (database_execute(database, "GET ID_COUNTER FROM CONFIG", &result) == 1) {
		return -1;
	}
	id = atoi(result);
	id++;
	free(result);
	if (asprintf(&query, "SET ID_COUNTER FROM CONFIG AS %d", id) == -1) {
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

int db_send_status(database_t* database, const char* request, char** result) {
	int id = 0;
	char* query = NULL;
	char* buffer = NULL;
	char* ptr;
	id = !strlen(request) ? -1 : atoi(request);
	if (!(rows && columns)) {
		asprintf(result, "");
		return 0;
	}
	database_execute(database, "GET 0 FROM DATA", &buffer);
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
		asprintf(&query, "GET %d FROM DATA", i);
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
int db_book(database_t* database, const char* request, char **result) {
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
		if (asprintf(&(rquery[i]), "GET %s FROM DATA", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s FROM DATA AS %d", token[i + 1], id) == -1) {
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
/*
int db_unbook(database_t* database, const char* request, char** result) {
	int id;
	int ntoken = 0;
	char* buffer = NULL;
	char* saveptr = NULL;
	char** token = NULL;
	char** query = NULL;

	ntoken = 0;
	buffer = strdup(request);
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

	id = atoi(token[0]);
	ntoken--;

	if ((query = malloc(sizeof(char*) * (size_t)ntoken)) == NULL) {
		return 1;
	}
	for (int i = 0; i < ntoken; i++) {
		if (asprintf(&(query[i]), "SET %s FROM DATA AS 0", token[i + 1]) == -1) {
			return 1;
		}
	}
	free(buffer);
	free(token);
	for (int i = 0; i < ntoken; i++) {
		if (database_execute(database, query[i], result) == 1) {
			for (int j = i; j < ntoken; free(query[++j]));
			free(query);
			return 1;
		}
		free(query[i]);
		free(*result);
	}
	free(query);
	if (asprintf(result, DBMSG_SUCC) == -1) {
		return 1;
	}
	return 0;
}
*/
int db_unbook(database_t* database, const char* request, char** result) {
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
		if (asprintf(&(rquery[i]), "GET %s FROM DATA", token[i + 1]) == -1) {
			return 1;
		}
		if (asprintf(&(wquery[i]), "SET %s FROM DATA AS 0", token[i + 1]) == -1) {
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

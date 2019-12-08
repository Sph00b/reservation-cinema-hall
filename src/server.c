#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>

#include "helper.h"
#include "asprintf.h"
#include "database.h"

int parseCmdLine(int argc, char *argv[], short *op);
void daemon_start();
int daemon_stop();
char *getpath(const char *rltvp);

int main(int argc, char *argv[]){
	/*Add status, start, stop, change ip and port, change configuration*/
	short op;
try(
	parseCmdLine(argc, argv, &op), (1)
)
	switch(op){
case(0):
	daemon_start();
case(1):
	daemon_stop();
default:
	break;
	}
	return 0;
}

void daemon_start(){
	char *filename;
try(
	asprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/bin/cinemad"), (-1)
)
try(
	execl(filename, "cinemad", NULL), (-1)
)
}

int daemon_stop(){
	/*send a SIGTERM*/
	char *pfilename;
	char *dfilename;
	char *pid;
	int pidfd;
try(
	asprintf(&pfilename, "%s%s", getenv("HOME"), "/.cinema/.tmp/cinemad.pid"), (-1)
)
try(
	pidfd = open(pfilename, 0600), (-1)
)
	if (flock(pidfd, LOCK_EX | LOCK_NB) == 0){
		printf("Server already stopped\n");
		flock(pidfd, LOCK_UN);
		return 0;
	}
	free(pfilename);
try(
	asprintf(&dfilename, "%s%s", getenv("HOME"), "/.cinema/etc/data.dat"), (-1)
)
try(
	database_init(dfilename), (1)
)
	free(dfilename);
try(
	pid = database_execute("GET PID FROM CONFIG"), (NULL)
)
try(
	kill(atoi(pid), SIGKILL), (-1)
)
	printf("Server stopped\n");
	return 0;
}

int parseCmdLine(int argc, char *argv[], short *op){
	if (argc < 2){
		printf("Usage:\t name [COMMAND] [-h]\n\n");
		exit(EXIT_SUCCESS);
	}
	for (int n = 1; n < argc; n++){
		if (!strncasecmp(argv[n], "-start", 6)){
			*op = 0;
		}
		else if (!strncasecmp(argv[n], "-stop", 5)){
			*op = 1;
		}
		else{
			printf("Usage:\t name [COMMAND] [-h]\n\n");
			exit(EXIT_SUCCESS);
		}
	}
	return 0;
}

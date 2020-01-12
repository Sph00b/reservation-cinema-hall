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

#include "asprintf.h"
#include "database.h"

#define try(foo, err_value)\
	if ((foo) == (err_value)){\
		fprintf(stderr, "%m\n");\
		exit(EXIT_FAILURE);\
	}

int parseCmdLine(int argc, char *argv[], short *op);
void daemon_start();
int daemon_stop();

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

	/*	Stub	*/
	system("killall -s KILL cinemad");
	return 0;
	/*			*/

	/* retirve pid with a query */
	/*send a SIGTERM to pid*/
	char *pid;
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

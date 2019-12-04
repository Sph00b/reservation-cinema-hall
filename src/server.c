#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "try.h"
#include "database.h"

int parseCmdLine(int argc, char *argv[], short *op);
int cinemad_start();
int stop();

int status;

int main(int argc, char *argv[]){
	/*Add status, start, stop, change ip and port, change configuration*/
	char *cmd;
	short op;
	status = 0;
try(
	database_init("data.dat"), (1)
)
try(
	parseCmdLine(argc, argv, &op), (1)
)
	switch(op){
case(0):
	cinemad_start();
case(1):
	stop();
default:
	break;
	}
	return 0;
}

int cinemad_start(){
	/*	execve cinemad deamon	*/
	status = 1;
	return 0;
}

int parseCmdLine(int argc, char *argv[], short *op){
	int n = 1;
	if (argc < 2){
		printf("Usage:\t name [COMMAND] [-h]\n\n");
		exit(EXIT_SUCCESS);
	}
	while(n < argc){
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

int stop(){
	/*	Signal closure and wait for threads to stop	*/
	//	pthread_join(chndl_tid, NULL);
	return 0;
}

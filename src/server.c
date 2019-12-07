#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "helper.h"
#include "database.h"

int parseCmdLine(int argc, char *argv[], short *op);
void cinemad_start();
int stop();
char *getpath(const char *rltvp);

int main(int argc, char *argv[]){
	/*Add status, start, stop, change ip and port, change configuration*/
	short op;
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

void set_foo(){
	char *filename;
try(
	msprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/etc/data.dat"), (-1)
)
try(
	database_init(filename), (1)
)
	free(filename);
}

void cinemad_start(){
	char *filename;
try(
	msprintf(&filename, "%s%s", getenv("HOME"), "/.cinema/bin/cinemad"), (-1)
)
try(
	execl(filename, "cinemad", NULL), (-1)
)
}

int stop(){
	system("killall -s KILL cinemad");
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

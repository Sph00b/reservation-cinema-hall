#include "utils.h"

#include <stdlib.h>

#include <stdio.h>

#include <unistd.h>

#include <signal.h>

#include <string.h>

#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/file.h>

#include <resources.h>
#include <try.h>

#define is_child(pid) !pid
#define MSG "Initialization complete"

static int close_file_descriptors();
static int reset_signal_mask_and_handlers();
static int reset_env_variables();
static int redirect_standard_streams();
static int store_pid(const char* name);
static int drop_privileges(const char* name);

extern inline int sysv_daemon(void) {
	pid_t pid;
	
	try(close_file_descriptors(), 1, error);
	try(reset_signal_mask_and_handlers(), 1, error);
	try(reset_env_variables(), 1, error);

	int pfd[2];
	try(pipe(pfd), -1, error);

	try(pid = fork(), -1, error);		// create a background process
	if (is_child(pid)) {
		setsid();						// detach from any terminal and create an independent session
		try(pid = fork(), -1, error);	// ensure that the daemon can never re-acquire a terminal again
		if (is_child(pid)) {
			try(redirect_standard_streams(), 1, error);
			try(store_pid("cinemad"), -1, error);
			try(drop_privileges("cinema"), 1, error);
			try(close(pfd[0]), -1, error);
			try(write(pfd[1], MSG, strlen(MSG)), -1, error);	//notify the original process started that initialization is complete
			try(close(pfd[1]), -1, error);
			return 0;
		}
		else {
			exit(EXIT_SUCCESS);			// ensures that the daemon process is re-parented to init / PID 1
		}
	}
	char buff[32];
	try(close(pfd[1]), -1, error);
	try((read(pfd[0], buff, strlen(MSG)) == strlen(MSG)), 0, error);	//notify the original process started that initialization is complete
	try(close(pfd[0]), -1, error);
	exit(EXIT_SUCCESS);
error:
	return 1;
}

extern inline int signal_block_all(void) {
	sigset_t sigset;
	try(sigfillset(&sigset), -1, error);
	try(pthread_sigmask(SIG_BLOCK, &sigset, NULL), !0, error);
	return 0;
error:
	return 1;
}

extern int signal_wait(int signum) {
	int sig;
	sigset_t sigset;
	if (signum == SIGANY) {
		try(sigfillset(&sigset), -1, error);
	}
	else {
		try(sigemptyset(&sigset), -1, error);
		try(sigaddset(&sigset, signum), -1, error);
	}
	try(sigwait(&sigset, &sig), !0, error);
	return 0;
error:
	return 1;
}

/*
* Close all open file descriptors except standard input, output, and error 
* (i.e. the first three file descriptors 0, 1, 2). 
* This ensures that no accidentally passed file descriptor stays around in the 
* daemon process.
* 
* @return	On success, this function returns zero.  On error, 1 is returned, 
*			and errno is set appropriately.
*/
static int close_file_descriptors() {
	struct rlimit rlim;
	try(getrlimit(RLIMIT_NOFILE, &rlim), -1, error);
	for (int i = 3; i < rlim.rlim_cur; i++) {
		try(close(i), -1 && errno != EBADF, error);
	}
	return 0;
error:
	return 1;
}

/*
* Reset the signal mask and all signal handlers to their default.
* 
* @return	On success, this function returns zero.  On error, 1 is returned, 
*			and errno is set appropriately.
*/
static int reset_signal_mask_and_handlers() {
	struct sigaction sact;
	memset(&sact, 0, sizeof * &sact);
	try(sigemptyset(&sact.sa_mask), -1, error);
	sact.sa_handler = SIG_DFL;
	sact.sa_flags = SA_RESETHAND;
	for (int i = 1; i < __SIGRTMIN; i++) {
		if (i == SIGKILL || i == SIGSTOP) {
			continue;
		}
		try(sigaction(i, &sact, NULL), -1, error);
	}
	return 0;
error:
	return 1;
}

/*
* Sanitize the environment block, removing or resetting environment variables 
* that might negatively impact daemon runtime.
*/
static inline int reset_env_variables() {
	//try(clearenv(), !0, error);	// idk, interface description is quite vague
	return 0;
error:
	return 1;
}

/*
* Connect /dev/null to standard input, output, and error.
*/
static inline int redirect_standard_streams() {
	int null_fd;
	try(null_fd = open("/dev/null", 0), -1, error);
	try(dup2(null_fd, STDIN_FILENO), -1, error);
	try(dup2(null_fd, STDOUT_FILENO), -1, error);
	try(dup2(null_fd, STDERR_FILENO), -1, error);
	return 0;
error:
	return 1;
}

/*
* Write the daemon PID to a PID file, to ensure that the daemon cannot be 
* started more than once. This must be implemented in race-free fashion so that 
* the PID file is only updated when it is verified at the same time that the PID 
* previously stored in the PID file no longer exists or belongs to a foreign 
* process.
*/
static int store_pid(const char* name) {
	int fd;
	char* filename;
	size_t size;
	
	size = strlen("/run/") + strlen(name) + strlen(".pid") + 1;
	try(filename = calloc(1, sizeof * filename * size), NULL, error);
	strcat(filename, "/tmp/");
	strcat(filename, name);
	strcat(filename, ".pid");
	
	try(fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0660), -1, cleanup);
	try(flock(fd, LOCK_EX | LOCK_NB), -1, cleanup);
	
	int fd_dup;
	FILE* stream;
	try(fd_dup = dup(fd), -1, cleanup);
	try(stream = fdopen(fd_dup, "w"), NULL, cleanup);
	try(fprintf(stream, "%ld", (long) getpid()) < 0, !0, cleanup);	// need some love
	try(fclose(stream), EOF, cleanup);

	return 0;
cleanup:
	free(filename);
error:
	return -1;
}

static int drop_privileges(const char* name) {
	int is_root = !getuid() || !getgid();
	if (is_root) {
		try(setgid(!0), -1, error);
		try(setuid(!0), -1, error);
	}
	char* wdir;
	try(asprintf(&wdir, "%s%s%s", getenv("HOME"), "/.", name), -1, error);
	try(chdir(wdir), -1, cleanup);
	try(setenv("PWD", wdir, 1), !0, cleanup);
	free(wdir);
	return 0;
cleanup:
	free(wdir);
error:
	return 1;
}

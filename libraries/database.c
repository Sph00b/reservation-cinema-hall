#include "database.h"

#ifdef __unix__

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#define WORDLEN 16

#define LOCK(mutex, ret) if (!ret) while ((ret = pthread_mutex_lock(mutex)) && errno == EINTR);
#define UNLOCK(mutex, ret) if (!ret) while ((ret = pthread_mutex_unlock(mutex)) && errno == EINTR);

/*	IDK how to name
	undefined behaviour if the strings pointed are longer than WORDLEN
*/

struct info {
	char* section;
	char* key;
	char* value;
};

/* return 0 on success, 1 on sys failure, -1 if not found */

int get_info(struct info** info, const char* str) {
	int ret = 0;
	int ntoken = 1;
	char* buff;
	char* saveptr;
	char** token = NULL;
	if ((*info = malloc(sizeof(struct info))) == NULL) {
		return 1;
	}
	ntoken = 0;
	if ((buff = strdup(str)) == NULL) {
		free(*info);
		return 1;
	}
	if ((token = malloc(sizeof(char*))) == NULL) {
		return 1;
	}
	token[0] = strtok_r(buff, " ", &saveptr);
	ntoken++;
	do {
		ntoken++;
		if ((token = realloc(token, sizeof(char*) * (size_t)ntoken)) == NULL) {
			return 1;
		}
		token[ntoken - 1] = strtok_r(NULL, " ", &saveptr);
	} while (token[ntoken - 1] != NULL);
	ntoken--;
	if (ntoken == 1) {
		(*info)->section = strdup(token[0]);
		(*info)->key = NULL;
		(*info)->value = NULL;
	}
	if (ntoken == 3 || ntoken == 5) {
		(*info)->key = strdup(token[0]);
		if (!strcmp(token[1], "FROM")) {
			(*info)->section = strdup(token[2]);
			(*info)->value = NULL;
		}
		else {
			ret = 1;
		}
	}
	if (ntoken == 5) {
		if (!strcmp(token[3], "AS")) {
			(*info)->value = strdup(token[4]);
		}
		else {
			ret = 1;
		}
	}
	free(buff);
	free(token);
	if (ret) {
		free(*info);
		return -1;
	}
	return 0;
}

/*	return 0 on success, 1 on sys failure, set offset to -1 if not found */
/*	PRONE TO SEGMENTATION FAULT!	*/
int get_offset(int** offset, FILE* strm, const struct info* info) {
	char* buff;
	char* pbuff;
	int strm_size;
	if (fseek(strm, 0, SEEK_END) == -1) {
		return 1;
	}
	if ((strm_size = (int)ftell(strm)) == -1) {
		return 1;
	}
	if ((buff = malloc(sizeof(char) * (size_t)strm_size)) == NULL) {
		return 1;
	}
	if (fseek(strm, 0, SEEK_SET) == -1) {
		return 1;
	}
	if (fgets(buff, strm_size, strm) == NULL) {
		free(buff);
		return 1;
	}
	if ((*offset = malloc(sizeof(int))) == NULL) {
		free(buff);
		return 1;
	}
	pbuff = buff;
	do {
		if ((pbuff = strstr(pbuff, info->section)) == NULL) {
			**offset = -1;
			return 0;
		}
		pbuff++;
	} while (!(pbuff[-2] == '<' && pbuff[strlen(info->section) - 1] == '>'));
	pbuff--;
	do {
		if ((pbuff = strstr(pbuff, info->key)) == NULL) {
			**offset = -1;
			return 0;
		}
		pbuff++;
	} while (!(pbuff[-2] == '[' && pbuff[ strlen(info->key) - 1] == ']'));
	pbuff--;
	**offset = (int)((size_t)(pbuff - buff) / sizeof(char)) + WORDLEN - 1;
	free(buff);
	return 0;
}

/*	return 0 on success, 1 on sys failure	*/

int get_stream(FILE** pstrm, int fd) {
	int newfd;
	while ((newfd = dup(fd)) == -1) {
		if (errno != EMFILE) {
			return 1;
		}
	}
	if ((*pstrm = fdopen(newfd, "r+")) == NULL) {
		return 1;
	}
	return 0;
}

/* return 0 on success, 1 on sys failure */

int add(int fd, const struct info* info) {
	FILE* strm;
	char* buff;
	if (get_stream(&strm, fd)) {
		return 1;
	}
	if (info->section == NULL) {
		return -1;
	}
	if (strlen(info->section) > WORDLEN - 2) {
		return -1;
	}
	if (info->key == NULL) {
		if ((buff = malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
			return 1;
		}
		memset(buff, ' ', WORDLEN);
		buff[0] = '<';
		strcpy(buff + 1, info->section);
		buff[strlen(buff)] = '>';
		buff[WORDLEN] = 0;
	}
	else {
		/*TODO:
		move the seek to the last position of the section
		make room for data if the section is not in tail
		*/
		if (strlen(info->key) > WORDLEN - 2) {
			return -1;
		}
		if ((buff = malloc(sizeof(char) * (2 * WORDLEN + 1))) == NULL) {
			return 1;
		}
		memset(buff, ' ', WORDLEN * 2);
		buff[0] = '[';
		strcpy(buff + 1, info->key);
		buff[strlen(buff)] = ']';
		buff[WORDLEN * 2] = 0;
	}
	if (fprintf(strm, buff) < 0) {
		free(buff);
		return 1;
	}
	fflush(strm);
	if (fclose(strm)) {
		free(buff);
		return 1;
	}
	free(buff);
	return 0;
}

/* return 0 on success, 1 on sys failure, -1 if not found */

int set(int fd, const struct info* info) {
	FILE* strm;
	int* offset;
	char* value;
	if (get_stream(&strm, fd)) {
		return 1;
	}
	if (info->value == NULL) {
		return -1;
	}
	if (strlen(info->value) > WORDLEN) {
		return -1;
	}
	if ((value = malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		return 1;
	}
	memset(value, ' ', WORDLEN);
	value[WORDLEN] = '\0';
	strcpy(value, info->value);
	if (strlen(value) < WORDLEN) {
		value[strlen(value)] = ' ';
	}
	if (get_offset(&offset, strm, info)) {
		free(value);
		return 1;
	}
	if (fseek(strm, *offset, SEEK_SET) == -1) {
		free(value);
		free(offset);
		return 1;
	}
	if (fprintf(strm, "%s", value) < 0) {
		free(value);
		free(offset);
		return 1;
	}
	fflush(strm);
	if (fclose(strm)) {
		return 1;
	}
	free(value);
	free(offset);
	return 0;
}

/* return 2 on success, 1 on sys failure, -1 if not found */

int get(int fd, const struct info* info, char** dest) {
	FILE* strm;
	int* offset;
	if (get_stream(&strm, fd)) {
		return 1;
	}
	if (get_offset(&offset, strm, info)) {
		return 1;
	}
	if (*offset == -1) {
		return -1;
	}
	if (fseek(strm, *offset, SEEK_SET) == -1) {
		free(offset);
		return 1;
	}
	if ((*dest = malloc(sizeof(char) * (WORDLEN + 1))) == NULL) {
		free(offset);
		return 1;
	}
	if (fgets(*dest, WORDLEN + 1, strm) == NULL) {
		free(*dest);
		free(offset);
		return 1;
	}
	if (fclose(strm)) {
		return 1;
	}
	free(offset);
	return 2;
}

/*	Initiazliza database from file return 1 and set properly errno on error	*/

int database_init(database_t *database, const char* filename) {
	int ret = 0;
	database->fd = open(filename, O_RDWR, 0666);
	if (flock(database->fd, LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		close(database->fd);
		return 1;
	}
	if ((database->lock = malloc(sizeof(pthread_rwlock_t))) == NULL) {
		close(database->fd);
		return 1;
	}
	while ((ret = pthread_rwlock_init(database->lock, NULL)) && errno == EINTR);
	if (ret && errno == EINTR) {
		close(database->fd);
		free(database->lock);
		return 1;
	}
	return 0;
}

/*	Close database return EOF and set properly errno on error */

int database_close(database_t *database) {
	int ret;
	while ((ret = pthread_rwlock_destroy(database->lock)) && errno == EINTR);
	if (ret && errno == EINTR) {
		return 1;
	}
	free(database->lock);
	return close(database->fd);
}

/*	Execute a query return 1 and set properly errno on error */

int database_execute(database_t* database, const char* query, char** result) {
	int ret = 0;
	int mret = 0;
	struct info* qinfo = NULL;
	if (database == NULL) {
		ret = 1;
		errno = EINVAL;
	}
	if (!ret && query == NULL) {
		ret = -1;
	}
	if (!ret && result == NULL) {
		ret = -1;
	}
	if (!ret && strlen(query) < 5) {
		ret = -1;
	}
	if (!ret) {
		ret = get_info(&qinfo, query + 4);
	}
	if (!ret) {
		if (!strncmp(query, "ADD ", 4)) {
			while ((mret = pthread_rwlock_wrlock(database->lock)) && errno == EINTR);
			if (!mret) {
				ret = add(database->fd, qinfo);
				while ((mret = pthread_rwlock_unlock(database->lock)) && errno == EINTR);
			}
		}
		else if (!strncmp(query, "SET ", 4)) {
			while ((mret = pthread_rwlock_wrlock(database->lock)) && errno == EINTR);
			if (!mret) {
				ret = set(database->fd, qinfo);
				while ((mret = pthread_rwlock_unlock(database->lock)) && errno == EINTR);
			}
		}
		else if (!strncmp(query, "GET ", 4)) {
			while ((mret = pthread_rwlock_rdlock(database->lock)) && errno == EINTR);
			if (!mret) {
				ret = get(database->fd, qinfo, result);
				while ((mret = pthread_rwlock_unlock(database->lock)) && errno == EINTR);
			}
		}
		else {
			ret = -1;
		}
		free(qinfo->section);
		free(qinfo->key);
		free(qinfo->value);
		free(qinfo);
	}
	if (mret) {
		ret = 1;
	}
	switch (ret) {
	case -1:
		*result = strdup(DBMSG_FAIL);
		return 0;
	case 0:
		*result = strdup(DBMSG_SUCC);
		return 0;
	case 1:
		*result = strdup(DBMSG_ERR);
		return 1;
	default:
		strtok(*result, " ");	//souldn't be handled here
		return 0;
	}
}

#endif
#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>

#include "avl_tree.h"

#define MAXLEN 16

#define MSG_SUCC "OPERATION SUCCEDED"
#define MSG_FAIL "OPERATION FAILED"

struct index_record {
	long offset;
	pthread_rwlock_t lock;
};

struct storage {
	int fd;
	avl_tree_t index_table;
	pthread_mutex_t mutex_new_record;
};

static FILE* get_stream(const storage_t handle);
static int lexicographical_comparison(void* key1, void* key2);
static int load_table(const storage_t handle);
static int get_record(const storage_t handle, struct index_record** result, const char* key);
static int update_record(const storage_t handle, const char* key, const int offset);

storage_t storage_init(const char* filename) {
	struct storage* storage;
	if ((storage = malloc(sizeof(struct storage))) == NULL) {
		return NULL;
	}
	if ((storage->fd = open(filename, O_RDWR, 0666)) == -1) {
		free(storage);
		return NULL;
	}
	if (flock(storage->fd, LOCK_EX | LOCK_NB) == -1) {	//replace with fcntl?
		close(storage->fd);
		free(storage);
		return NULL;
	}
	if ((storage->index_table = avl_tree_init(&lexicographical_comparison)) == NULL) {
		close(storage->fd);
		free(storage);
		return NULL;
	}
	if (load_table(storage)) {
		avl_tree_destroy(storage->index_table);
		close(storage->fd);
		free(storage);
		return NULL;
	}
	int ret;
	while ((ret = pthread_mutex_init(&storage->mutex_new_record, NULL)) && errno == EINTR);
	if (ret) {
		avl_tree_destroy(storage->index_table);
		close(storage->fd);
		free(storage);
		return NULL;
	}
	return storage;
}

int storage_close(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	//index_table_destroy(storage->index_table);
	close(storage->fd);
	free(storage);
	return 0;
}

int storage_store(const storage_t handle, char* key, char* value, char** result) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	FILE* stream;
	if ((stream = get_stream(storage)) == NULL) {
		return 1;
	}
	if (get_record(storage, &record, key)) {
		return 1;
	}
	if (record->offset == -1) {
		int ret;
		while ((ret = pthread_mutex_lock(&storage->mutex_new_record)) && errno == EINTR);
		if (ret) {
			return 1;
		}
		/*	check if in the meantime other threads created the record	*/
		if (get_record(storage, &record, key)) {
			return 1;
		}
		if (record->offset == -1) {
			int offset;
			if (fseek(stream, 0, SEEK_END) == -1) {
				return 1;
			}
			if ((offset = ftell(stream)) == -1) {
				return 1;
			}
			update_record(storage, key, offset + MAXLEN);
			for (int i = 0; i < MAXLEN; i++) {
				if (fputc(key[i], stream) == EOF) {
					return 1;
				}
			}
			for (int i = 0; i < MAXLEN; i++) {
				if (fputc(0, stream) == EOF) {
					return 1;
				}
			}
			fflush(stream);
			if (fseek(stream, MAXLEN * -1, SEEK_END) == -1) {
				return 1;
			}
		}
		while ((ret = pthread_mutex_unlock(&storage->mutex_new_record)) && errno == EINTR);
		if (ret) {
			return 1;
		}
	}
	else {
		if (fseek(stream, record->offset, SEEK_SET) == -1) {
			return 1;
		}
	}
	for (int i = 0; i < MAXLEN; i++) {
		if (fputc(value[i], stream) == EOF) {
			return 1;
		}
	}
	fflush(stream);
	*result = strdup(MSG_SUCC);
	return 0;
}

int storage_load(const storage_t handle, char* key, char** result) {
	/*
	if (strlen(key) > MAXLEN) {
		*result = NULL;
		return 1;
	}
	*/
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	FILE* stream;
	if ((stream = get_stream(storage)) == NULL) {
		return 1;
	}
	if (get_record(storage, &record, key)) {
		return 1;
	}
	if (record->offset == -1) {
		*result = strdup(MSG_FAIL);
		return 0;
	}
	if (fseek(stream, record->offset, SEEK_SET) == -1) {
		return 1;
	}
	if ((*result = malloc(sizeof(char) * MAXLEN + 1)) == NULL) {
		return 1;
	}
	for (int i = 0; i < MAXLEN; i++) {
		if ((int)((*result)[i] = (char)fgetc(stream)) == EOF) {
			return 1;
		}
	}
	(result)[MAXLEN] = 0;
	return 0;
}

int storage_lock_shared(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	if (get_record(storage, &record, key)) {
		return 1;
	}
	if (record == NULL) {
		return 0;
	}
	while ((ret = pthread_rwlock_rdlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int storage_lock_exclusive(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	if (get_record(storage, &record, key)) {
		return 1;
	}
	if (record == NULL) {
		return 0;
	}
	while ((ret = pthread_rwlock_wrlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int storage_unlock(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	if (get_record(storage, &record, key)) {
		return 1;
	}
	if (record == NULL) {
		return 0;
	}
	while ((ret = pthread_rwlock_unlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

static FILE* get_stream(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	int newfd;
	FILE* stream;
	while ((newfd = dup(storage->fd)) == -1) {
		if (errno != EMFILE) {
			return NULL;
		}
		sleep(1);
	}
	if ((stream = fdopen(newfd, "r+")) == NULL) {
		return NULL;
	}
	if (fseek(stream, 0, SEEK_SET) == -1) {
		fclose(stream);
		return NULL;
	}
	return stream;
}

static int lexicographical_comparison(void* key1, void* key2) {
	char* str1 = (char*)key1;
	char* str2 = (char*)key2;
	for (int i = 0; ; i++) {
		if (!str1[i] && !str2[i]) {
			return 0;
		}
		else if (str1[i] && !str2[i]) {
			return 1;
		}
		else if (!str1[i] && str2[i]) {
			return -1;
		}
		else if (str1[i] < str2[i]) {
			return -1;
		}
		else if (str1[i] > str2[i]) {
			return 1;
		}
	}
}

static int load_table(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	FILE* stream;
	int c;
	if ((stream = get_stream(storage)) == NULL) {
		return 1;
	}
	while ((c = fgetc(stream)) != EOF) {
		if (fseek(stream, -1, SEEK_CUR) == -1) {
			return 1;
		}
		char* current_key;
		struct index_record* current_record;
		if ((current_key = malloc(sizeof(char) * (MAXLEN + 1))) == NULL) {
			return 1;
		}
		for (int i = 0; i < MAXLEN; i++) {
			if ((int)(current_key[i] = (char)fgetc(stream)) == EOF) {
				fclose(stream);
				return 1;
			}
		}
		current_key[MAXLEN] = 0;
		if ((current_record = malloc(sizeof(struct index_record))) == NULL) {
			return 1;
		}
		if ((current_record->offset = ftell(stream)) == -1) {
			free(current_record);
			fclose(stream);
			return 1;
		}
		int ret;
		while ((ret = pthread_rwlock_init(&current_record->lock, NULL)) && errno == EINTR);
		if (ret) {
			free(current_record);
			return 1;
		}
		if (avl_tree_insert(storage->index_table, current_key, current_record)) {
			free(current_key);
			pthread_rwlock_destroy(&current_record->lock);
			free(current_record);
			return 1;
		}
		if (fseek(stream, MAXLEN, SEEK_CUR) == -1) {
			return 1;
		}
	}
	if (fclose(stream) == EOF) {
		return 1;
	}
	return 0;
}

static int get_record(const storage_t handle, struct index_record** result, const char* key) {
	struct storage* storage = (struct storage*)handle;
	if (avl_tree_search(storage->index_table, key) == NULL) {
		struct index_record* record;
		char* record_key;
		if ((record_key = malloc(sizeof(char) * (MAXLEN + 1))) == NULL) {
			*result = NULL;
			return 1;
		}
		strncpy(record_key, key, MAXLEN);
		record_key[MAXLEN] = 0;
		if ((record = malloc(sizeof(struct index_record))) == NULL) {
			free(record_key);
			*result = NULL;
			return 1;
		}
		record->offset = -1;
		int ret;
		while ((ret = pthread_rwlock_init(&record->lock, NULL)) && errno == EINTR);
		if (ret) {
			free(record_key);
			free(record);
			*result = NULL;
			return 1;
		}
		if (avl_tree_insert(storage->index_table, record_key, record)) {
			free(record_key);
			pthread_rwlock_destroy(&record->lock);
			free(record);
			*result = NULL;
			return 1;
		}
	}
	*result = avl_tree_search(storage->index_table, key);
	return 0;
}

static int update_record(const storage_t handle, const char* key, const int offset) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record = avl_tree_search(storage->index_table, key);
	if (record) {
		record->offset = offset;
	}
	return 0;
}

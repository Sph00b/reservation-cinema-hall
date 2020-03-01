#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>

#include "index_table.h"

#define MAXLEN 16

struct index_record {
	long offset;
	pthread_rwlock_t lock;
};

struct storage {
	FILE* stream;
	char* buffer_cache;
	index_table_t index_table;
	pthread_mutex_t mutex_seek_stream;
	pthread_rwlock_t lock_buffer_cahce;
};

/*	Prototype declarations of functions included in this code module	*/

static int lexicographical_comparison(const void* key1, const void* key2);
static int update_buffer_cache(const storage_t handle);
static int load_table(const storage_t handle);
static int format(const char* str, char** result);
static index_record_t record_init(index_table_t index_table, void* key);
static int record_destroy(void* key, void* value);

storage_t storage_init(const char* filename) {
	struct storage* storage;
	if ((storage = malloc(sizeof(struct storage))) == NULL) {
		return NULL;
	}
	if ((storage->stream = fopen(filename, "r+")) == NULL) {
		free(storage);
		return NULL;
	}
	if (flock(fileno(storage->stream), LOCK_EX | LOCK_NB) == -1) {	//instead of semget & ftok to avoid mix SysV and POSIX, replace with fcntl
		free(storage);
		return NULL;
	}
	storage->buffer_cache = NULL;
	int ret;
	while ((ret = pthread_mutex_init(&storage->mutex_seek_stream, NULL)) && errno == EINTR);
	if (ret) {
		free(storage);
		return NULL;
	}
	while ((ret = pthread_rwlock_init(&storage->lock_buffer_cahce, NULL)) && errno == EINTR);
	if (ret) {
		free(storage);
		return NULL;
	}
	if ((storage->index_table = index_table_init(&record_init, &record_destroy, &lexicographical_comparison)) == NULL) {
		free(storage);
		return NULL;
	}
	if (load_table(storage)) {
		free(storage);
		return NULL;
	}
	if (update_buffer_cache(storage)) {
		free(storage);
		return NULL;
	}
	return storage;
}

int storage_close(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	int ret;
	while ((ret = pthread_mutex_destroy(&storage->mutex_seek_stream)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while ((ret = pthread_rwlock_destroy(&storage->lock_buffer_cahce)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	index_table_destroy(storage->index_table);
	fclose(storage->stream);
	free(storage->buffer_cache);
	free(storage);
	return 0;
}

int storage_store(const storage_t handle, const char* key, const char* value, char** result) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	char* formatted_key;
	char* formatted_value;
	if (format(key, &formatted_key)) {
		return 1;
	}
	if (format(value, &formatted_value)) {
		return 1;
	}
	if (formatted_key == NULL) {
		*result = strdup(MSG_FAIL);
		return 0;
	}
	if (formatted_value == NULL) {
		*result = strdup(MSG_FAIL);
		return 0;
	}
	if ((record = index_table_search(storage->index_table, key)) == NULL) {
		return 1;
	}
	/*	add record if it doesn't exist	*/
	if (record->offset == -1) {
		while ((ret = pthread_rwlock_wrlock(&storage->lock_buffer_cahce)) && errno == EINTR);
		if (ret) {
			return 1;
		}
		while ((ret = pthread_mutex_lock(&storage->mutex_seek_stream)) && errno == EINTR);
		if (ret) {
			return 1;
		}
		long offset;
		if (fseek(storage->stream, 0, SEEK_END) == -1) {
			return 1;
		}
		if ((offset = ftell(storage->stream)) == -1) {
			return 1;
		}
		for (int i = 0; i < MAXLEN; i++) {
			if (fputc(formatted_key[i], storage->stream) == EOF) {
				return 1;
			}
		}
		for (int i = 0; i < MAXLEN; i++) {
			if (fputc(0, storage->stream) == EOF) {
				return 1;
			}
		}
		fflush(storage->stream);
		while ((ret = pthread_mutex_unlock(&storage->mutex_seek_stream)) && errno == EINTR);
		if (ret) {
			return 1;
		}
		record->offset = offset + MAXLEN;
		update_buffer_cache(storage);
		while ((ret = pthread_rwlock_unlock(&storage->lock_buffer_cahce)) && errno == EINTR);
		if (ret) {
			return 1;
		}
	}
	free(formatted_key);
	while ((ret = pthread_mutex_lock(&storage->mutex_seek_stream)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if (fseek(storage->stream, record->offset, SEEK_SET) == -1) {
		return 1;
	}
	for (int i = 0; i < MAXLEN; i++) {
		if (fputc(formatted_value[i], storage->stream) == EOF) {
			return 1;
		}
	}
	fflush(storage->stream);
	while ((ret = pthread_mutex_unlock(&storage->mutex_seek_stream)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	for (int i = 0; i < MAXLEN; i++) {
		storage->buffer_cache[record->offset + i] = formatted_value[i];
	}
	free(formatted_value);
	*result = strdup(MSG_SUCC);
	return 0;
}

int storage_load(const storage_t handle, const char* key, char** result) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	char* formatted_key;
	if (format(key, &formatted_key)) {
		return 1;
	}
	if (formatted_key == NULL) {
		*result = strdup(MSG_FAIL);
		return 0;
	}
	if ((record = index_table_search(storage->index_table, key)) == NULL) {
		return 1;
	}
	free(formatted_key);
	if (record->offset == -1) {
		index_table_delete(storage->index_table, key);	//avl_delete is bugged and avl_search isn't atomic
		*result = strdup(MSG_FAIL);
		return 0;
	}
	if ((*result = malloc(sizeof(char) * MAXLEN + 1)) == NULL) {
		return 1;
	}
	while ((ret = pthread_rwlock_rdlock(&storage->lock_buffer_cahce)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	for (int i = 0; i < MAXLEN; i++) {
		(*result)[i] = storage->buffer_cache[record->offset + i];
	}
	(*result)[MAXLEN] = 0;
	while ((ret = pthread_rwlock_unlock(&storage->lock_buffer_cahce)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int storage_lock_shared(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	char* formatted_key;
	if (format(key, &formatted_key)) {
		return 1;
	}
	if (formatted_key == NULL) {
		return 0;
	}
	if ((record = index_table_search(storage->index_table, key)) == NULL) {
		return 1;
	}
	free(formatted_key);
	while ((ret = pthread_rwlock_rdlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int storage_lock_exclusive(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	char* formatted_key;
	if (format(key, &formatted_key)) {
		return 1;
	}
	if (formatted_key == NULL) {
		return 0;
	}
	if ((record = index_table_search(storage->index_table, key)) == NULL) {
		return 1;
	}
	free(formatted_key);
	while ((ret = pthread_rwlock_wrlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int storage_unlock(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	int ret;
	char* formatted_key;
	if (format(key, &formatted_key)) {
		return 1;
	}
	if (formatted_key == NULL) {
		return 0;
	}
	if ((record = index_table_search(storage->index_table, key)) == NULL) {
		return 1;
	}
	free(formatted_key);
	while ((ret = pthread_rwlock_unlock(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

static int lexicographical_comparison(const void* key1, const void* key2) {
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
	if (fseek(storage->stream, 0, SEEK_SET) == -1) {
		return 1;
	}
	while ((fgetc(storage->stream)) != EOF) {
		if (fseek(storage->stream, -1, SEEK_CUR) == -1) {
			return 1;
		}
		char* current_key;
		struct index_record* current_record;
		if ((current_key = malloc(sizeof(char) * (MAXLEN + 1))) == NULL) {
			return 1;
		}
		for (int i = 0; i < MAXLEN; i++) {
			if ((int)(current_key[i] = (char)fgetc(storage->stream)) == EOF) {
				free(current_key);
				return 1;
			}
		}
		current_key[MAXLEN] = 0;
		if ((current_record = malloc(sizeof(struct index_record))) == NULL) {
			free(current_key);
			return 1;
		}
		if ((current_record->offset = ftell(storage->stream)) == -1) {
			free(current_key);
			free(current_record);
			return 1;
		}
		int ret;
		while ((ret = pthread_rwlock_init(&current_record->lock, NULL)) && errno == EINTR);
		if (ret) {
			free(current_key);
			free(current_record);
			return 1;
		}
		if (index_table_insert(storage->index_table, current_key, current_record)) {
			free(current_key);
			free(current_record);
			return 1;
		}
		if (fseek(storage->stream, MAXLEN, SEEK_CUR) == -1) {
			free(current_key);
			free(current_record);
			return 1;
		}
	}
	if (!feof(storage->stream)) {
		return 1;
	}
	return 0;
}

static int update_buffer_cache(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	long filesize;
	int ret;

	free(storage->buffer_cache);
	while ((ret = pthread_mutex_lock(&storage->mutex_seek_stream)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	if (fseek(storage->stream, 0, SEEK_END) == -1) {
		return 1;
	}
	if ((filesize = ftell(storage->stream)) == -1) {
		return 1;
	}
	if ((storage->buffer_cache = malloc(sizeof(char) * (size_t)(filesize + 1))) == NULL) {
		return 1;
	}
	if (fseek(storage->stream, 0, SEEK_SET) == -1) {
		return 1;
	}
	if (fgets(storage->buffer_cache, (int)filesize + 1, storage->stream) == NULL) {
		return 1;
	}
	while ((ret = pthread_mutex_unlock(&storage->mutex_seek_stream)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

static int format(const char* str, char** result) {
	if (strlen(str) > MAXLEN) {
		*result = NULL;
		return 0;
	}
	if ((*result = malloc(sizeof(char) * MAXLEN + 1)) == NULL) {
		return 1;
	}
	memset(*result, 0, MAXLEN + 1);
	strncpy(*result, str, MAXLEN);
	return 0;
}

static index_record_t record_init(index_table_t index_table, void* key) {
	struct index_record* record;
	char* record_key;
	if ((record_key = malloc(sizeof(char) * (MAXLEN + 1))) == NULL) {
		return NULL;
	}
	strncpy(record_key, key, MAXLEN);
	record_key[MAXLEN] = 0;
	if ((record = malloc(sizeof(struct index_record))) == NULL) {
		free(record_key);
		return NULL;
	}
	record->offset = -1;
	int ret;
	while ((ret = pthread_rwlock_init(&record->lock, NULL)) && errno == EINTR);
	if (ret) {
		free(record_key);
		free(record);
		return NULL;
	}
	if (index_table_insert(index_table, record_key, record)) {
		free(record_key);
		free(record);
		return NULL;
	}
	return record;
}

static int record_destroy(void* key, void* value) {
	int ret;
	struct index_record* record = (struct index_record*)value;
	while ((ret = pthread_rwlock_destroy(&record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	free(key);
	free(record);
	return 0;
}

#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>

#include "try.h"

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
	pthread_rwlock_t lock_buffer_cache;
};

/*	Prototype declarations of functions included in this code module	*/

static int lexicographical_comparison(const void* key1, const void* key2);
static int update_buffer_cache(const storage_t handle);
static int load_table(const storage_t handle);
static int format(const char* str, char** result);
static index_record_t record_init();
static int record_destroy(void* key, void* value);
static int add_record(const index_record_t _record, FILE* stream, const char* key, const char* value);
static int set_record_value(const storage_t _storage, index_record_t _record, const char* value);
static int get_record_value(const index_record_t _record, const char* buffer, char** result);

extern storage_t storage_init(const char* filename) {
	struct storage* storage;
	storage = calloc(1, sizeof * storage);
	if (storage) {
		storage->buffer_cache = NULL;
		try(storage->stream = fopen(filename, "r+"), NULL, error);
		try_pthread_mutex_init(&storage->mutex_seek_stream, cleanup1);
		try_pthread_rwlock_init(&storage->lock_buffer_cache, cleanup2);
		try(storage->index_table = index_table_init(&record_init, &record_destroy, &lexicographical_comparison), NULL, cleanup3);
		try(load_table(storage), !0, cleanup4);
		try(update_buffer_cache(storage), !0, cleanup4);
	}
	return storage;
	// This is still quite broken
cleanup4:
	index_table_destroy(storage->index_table);
cleanup3:
	pthread_rwlock_destroy(&storage->lock_buffer_cache);
cleanup2:
	pthread_mutex_destroy(&storage->mutex_seek_stream);
cleanup1:
	fclose(storage->stream);
error:
	free(storage);
	return NULL;
}

extern int storage_close(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;

	try_pthread_mutex_destroy(&storage->mutex_seek_stream, error);
	try_pthread_rwlock_destroy(&storage->lock_buffer_cache, error);
	index_table_destroy(storage->index_table);
	fclose(storage->stream);
	free(storage->buffer_cache);
	free(storage);
	return 0;

error:
	return 1;
}

extern int storage_store(const storage_t handle, const char* key, const char* value, char** result) {
	struct storage* storage = (struct storage*)handle;

	struct index_record* record;
	char* formatted_key;
	char* formatted_value;

	try(format(key, &formatted_key), !0, error);
	try(format(value, &formatted_value), !0, cleanup1);
	if (formatted_key == NULL || formatted_value == NULL) {
		goto fail;
	}

	try(record = index_table_search(storage->index_table, strdup(formatted_key)), NULL, cleanup2);

	if (record->offset == -1) {
		try_pthread_rwlock_wrlock(&storage->lock_buffer_cache, error);
		try_pthread_mutex_lock(&storage->mutex_seek_stream, cleanup2);
		try(add_record(record, storage->stream, formatted_key, formatted_value), 1, cleanup2);
		try_pthread_mutex_unlock(&storage->mutex_seek_stream, cleanup2);
		try(update_buffer_cache(storage), 1, cleanup2);
		try_pthread_rwlock_unlock(&storage->lock_buffer_cache, cleanup2);
	}
	else {
		try_pthread_mutex_lock(&storage->mutex_seek_stream, error);
		try(set_record_value(storage, record, formatted_value), 1, cleanup2);
		// update_buffer_cache is still tightly coupled with set_record_value
		try_pthread_mutex_unlock(&storage->mutex_seek_stream, error);
	}
	*result = strdup(MSG_SUCC);

	free(formatted_key);
	free(formatted_value);

	return 0;

fail:
	*result = strdup(MSG_FAIL);
	return 0;
cleanup2:
	free(formatted_value);
cleanup1:
	free(formatted_key);
error:
	return 1;
}

extern int storage_load(const storage_t handle, const char* key, char** result) {
	struct storage* storage = (struct storage*)handle;

	struct index_record* record;
	char* formatted_key;
	try(format(key, &formatted_key), !0, error);
	if (formatted_key == NULL) {
		goto fail;
	}
	try(record = index_table_search(storage->index_table, strdup(formatted_key)), NULL, error);
	free(formatted_key);

	if (record->offset == -1) {
		index_table_delete(storage->index_table, key);
		goto fail;
	}
	else {
		try_pthread_rwlock_rdlock(&storage->lock_buffer_cache, error);
		try(get_record_value(record, storage->buffer_cache, result), 1, error);
		try_pthread_rwlock_unlock(&storage->lock_buffer_cache, error);
	}
	return 0;

fail:
	*result = strdup(MSG_FAIL);
	return 0;
error:
	return 1;
}

extern int storage_lock_shared(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;

	struct index_record* record;
	char* formatted_key;
	try(format(key, &formatted_key), !0, error);
	try(formatted_key, NULL, on_success);
	try(record = index_table_search(storage->index_table, strdup(formatted_key)), NULL, error);
	free(formatted_key);
	try_pthread_rwlock_rdlock(&record->lock, error);

on_success:
	return 0;
error:
	return 1;
}

extern int storage_lock_exclusive(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;

	struct index_record* record;
	char* formatted_key;
	try(format(key, &formatted_key), !0, error);
	try(formatted_key, NULL, on_success);
	try(record = index_table_search(storage->index_table, strdup(formatted_key)), NULL, error);
	free(formatted_key);
	try_pthread_rwlock_wrlock(&record->lock, error);

on_success:
	return 0;
error:
	return 1;
}

extern int storage_unlock(const storage_t handle, const char* key) {
	struct storage* storage = (struct storage*)handle;
	struct index_record* record;
	char* formatted_key;
	try(format(key, &formatted_key), !0, error);
	try(formatted_key, NULL, on_success);
	try(record = index_table_search(storage->index_table, strdup(formatted_key)), NULL, error);
	free(formatted_key);
	if (record->offset == -1) {
		index_table_delete(storage->index_table, key);
		return 0;
	}
	try_pthread_rwlock_unlock(&record->lock, error);

on_success:
	return 0;
error:
	return 1;
}

/*
* Function to compare two strings
*/
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

/*
* Initialize the value in the index table depending on the content of the 
* storage 
*/
static int load_table(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;

	struct index_record* current_record;
	char* current_key;

	try(fseek(storage->stream, 0, SEEK_SET), -1, error);

	while ((fgetc(storage->stream)) != EOF) {

		try(fseek(storage->stream, -1, SEEK_CUR), -1, error);

		try(current_key = calloc(1, sizeof(char) * (MAXLEN + 1)), NULL, error);
		for (int i = 0; i < MAXLEN; i++) {
			try((int)(current_key[i] = (char)fgetc(storage->stream)), EOF, cleanup1);
		}
		
		try(current_record = record_init(), NULL, cleanup1);
		try(current_record->offset = ftell(storage->stream), -1, cleanup2);
		try(index_table_insert(storage->index_table, current_key, current_record), !0, cleanup2);
		try(fseek(storage->stream, MAXLEN, SEEK_CUR), -1, cleanup2);
	}
	if (!feof(storage->stream)) {	// is this really required?
		return 1;
	}
	return 0;

cleanup2:
	free(current_record);
cleanup1:
	free(current_key);
error:
	return 1;
}

/*
* Update the buffer cache
*/
static int update_buffer_cache(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;

	long filesize;
	free(storage->buffer_cache);
	try_pthread_mutex_lock(&storage->mutex_seek_stream, error);
	try(fseek(storage->stream, 0, SEEK_END), -1, error);
	try(filesize = ftell(storage->stream), -1, error);
	try(storage->buffer_cache = calloc(1, sizeof(char) * (size_t)(filesize + 1)), NULL, error);
	try(fseek(storage->stream, 0, SEEK_SET), -1, error);
	try(fgets(storage->buffer_cache, (int)filesize + 1, storage->stream), NULL, error);
	try_pthread_mutex_unlock(&storage->mutex_seek_stream, error);
	return 0;

error:
	return 1;
}

/*
* Format the received string in a valid string saved in the result parameter.
* 
* @return	0 on success or return 1 and set properly errno on error
*/
static int format(const char* str, char** result) {
	if (strlen(str) < MAXLEN) {
		try((*result = calloc(1, sizeof(char) * MAXLEN + 1)), NULL, error);
		strncpy(*result, str, MAXLEN);
	}
	else {
		*result = NULL;
	}
	return 0;
error:
	return 1;
}

/* Function to create a record data type return NULL on failure and set properly
 * errno on error.
 */
static index_record_t record_init() {
	struct index_record* record;
	record = calloc(1, sizeof * record);
	if (record) {
		record->offset = -1;
		try_pthread_rwlock_init(&record->lock, error);
	}
	return record;
error:
	free(record);
	return NULL;
}

/* Function to destroy a record data type return 1 on failure and set properly 
 * errno on error.
*/
static int record_destroy(void* key, void* value) {
	struct index_record* record = (struct index_record*)value;
	try_pthread_rwlock_destroy(&record->lock, error);
	free(key);
	free(record);
	return 0;
error:
	return 1;
}

static int add_record(const index_record_t _record, FILE* stream, const char* key, const char* value) {
	struct index_record* record = _record;

	long offset;
	try(fseek(stream, 0, SEEK_END), -1, error);
	try(offset = ftell(stream), -1, error);
	for (int i = 0; i < MAXLEN; i++) {
		try(fputc(key[i], stream), EOF, error);
	}
	for (int i = 0; i < MAXLEN; i++) {
		try(fputc(value[i], stream), EOF, error);
	}
	fflush(stream);
	record->offset = offset + MAXLEN;
	return 0;

error:
	return 1;
}

static int set_record_value(const storage_t _storage, index_record_t _record, const char* value) {
	struct storage* storage = (struct storage*)_storage;
	struct index_record* record = _record;

	try(fseek(storage->stream, record->offset, SEEK_SET), -1, error);
	for (int i = 0; i < MAXLEN; i++) {
		try(fputc(value[i], storage->stream), EOF, error);
	}
	fflush(storage->stream);
	for (int i = 0; i < MAXLEN; i++) {
		storage->buffer_cache[record->offset + i] = value[i];
	}
	return 0;

error:
	return 1;
}

static int get_record_value(const index_record_t _record, const char* buffer, char** result) {
	struct index_record* record = _record;

	try(*result = calloc(1, sizeof(char) * MAXLEN + 1), NULL, error);
	for (int i = 0; i < MAXLEN; i++) {
		(*result)[i] = buffer[record->offset + i];
	}
	return 0;

error:
	return 1;
}

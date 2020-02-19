#include "index_record.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "resources.h"

struct index_record {
	char key[WORDLEN + 1];
	long offset;
	pthread_rwlock_t lock;
};

index_record_t index_record_init(void** storage) {
	struct index_record* index_record;
	FILE** stream = (FILE**)storage;
	if ((index_record = malloc(sizeof(struct index_record))) == NULL) {
		return NULL;
	}
	for (int i = 0; i < WORDLEN; i++) {
		if ((int)(index_record->key[i] = (char)fgetc(*stream)) == EOF) {
			free(index_record);
			return NULL;
		}
	}
	index_record->key[WORDLEN] = '\0';
	if ((index_record->offset = ftell(*stream)) == -1) {
		free(index_record);
		return NULL;
	}
	for (int i = 0; i < WORDLEN; i++) {
		if (fgetc(*stream) == EOF) {
			free(index_record);
			return NULL;
		}
	}
	int ret;
	while ((ret = pthread_rwlock_init(&index_record->lock, NULL)) && errno == EINTR);
	if (ret) {
		free(index_record);
		return NULL;
	}
	return index_record;
}

int index_record_destroy(const index_record_t handle) {
	struct index_record* index_record = (struct index_record*)handle;
	int ret;
	while ((ret = pthread_rwlock_destroy(&index_record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	free(index_record);
	return 0;
}

int index_record_wrlock(const index_record_t handle) {
	struct index_record* index_record = (struct index_record*)handle;
	int ret;
	while ((ret = pthread_rwlock_wrlock(&index_record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_record_rdlock(const index_record_t handle) {
	struct index_record* index_record = (struct index_record*)handle;
	int ret;
	while ((ret = pthread_rwlock_rdlock(&index_record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_record_unlock(const index_record_t handle) {
	struct index_record* index_record = (struct index_record*)handle;
	int ret;
	while ((ret = pthread_rwlock_unlock(&index_record->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

/*	1 if true 0 if false	*/
int index_record_compare(const index_record_t handle, const void* key) {
	struct index_record* index_record = (struct index_record*)handle;
	char* str_key = (char*)key;
	if (!strncmp(index_record->key, str_key, WORDLEN)) {
			return 1;
		}
	return 0;
}

int index_record_locate(const index_record_t handle, void** storage) {
	struct index_record* index_record = (struct index_record*)handle;
	FILE** stream = (FILE**)storage;
	if (fseek(*stream, index_record->offset, SEEK_SET) == -1) {
		return 1;
	}
	return 0;
}

/*	Hope I'll have enought time to develop a storage structure	*/
int index_record_storage_is_valid(void** storage) {
	FILE** stream = (FILE**)storage;
	int c;
	if ((c = fgetc(*stream)) != EOF){
		if (fseek(*stream, -1, SEEK_CUR) == -1) {
			/*	error isn't handled!	*/
			return 0;
		}
		return 1;
	}
	return 0;
}

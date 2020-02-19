#include "index_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "resources.h"

struct index_record {
	char key_name[WORDLEN + 1];
	long int key_offset;
	pthread_rwlock_t key_lock;
};

struct index_table {
	int n_record;
	struct index_record* record;
	pthread_rwlock_t table_lock;
};

index_table_t index_table_init() {
	struct index_table* index_table;
	if ((index_table = malloc(sizeof(struct index_table))) == NULL) {
		return NULL;
	}
	index_table->n_record = 0;
	index_table->record = NULL;
	int ret;
	while ((ret = pthread_rwlock_init(&index_table->table_lock, NULL)) && errno == EINTR);
	if (ret) {
		free(index_table);
		return NULL;
	}
	return index_table;
}

int index_table_destroy(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_destroy(&index_table->table_lock)) && errno == EINTR);
	if (ret) {
		free(index_table);
		return 1;
	}
	/*	destroy in a for	*/
	free(index_table->record);
	free(index_table);
	return 0;
}

int index_table_wrlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_wrlock(&index_table->table_lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_rdlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_rdlock(&index_table->table_lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_unlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_unlock(&index_table->table_lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_update(const index_table_t handle, void* storage) {
	struct index_table* index_table = (struct index_table*)handle;
	FILE* strm = (FILE*)storage;
	struct index_record* tmp_table;
	int ret;
	if (fseek(strm, 0, SEEK_SET) == -1) {
		return 1;
	}
	//	destroy all regords in a for
	if ((tmp_table = malloc(sizeof(struct index_record))) == NULL) {
		return 1;
	}
	tmp_table->key_name[0] = '\0';
	tmp_table->key_offset = -1;
	int i = 0;
	int c = 0;
	for (i = 0; (c = fgetc(strm)) != EOF; i++) {
		if (fseek(strm, -1, SEEK_CUR) == -1) {
			free(tmp_table);
			return 1;
		}
		if ((tmp_table = realloc(tmp_table, sizeof(struct index_record) * (size_t)(i + 1))) == NULL) {
			free(tmp_table);
			return 1;
		}
		for (int j = 0; j < WORDLEN; j++) {
			if ((c = fgetc(strm)) == EOF) {
				free(tmp_table);
				return 1;
			}
			tmp_table[i].key_name[j] = (char)c;
		}
		tmp_table[i].key_name[WORDLEN] = '\0';
		tmp_table[i].key_offset = ftell(strm);
		if (tmp_table[i].key_offset == -1) {
			free(tmp_table);
			return 1;
		}
		while ((ret = pthread_rwlock_init(&tmp_table[i].key_lock, NULL)) && errno == EINTR);
		if (ret) {
			free(tmp_table);
			return 1;
		}
		if (fseek(strm, WORDLEN, SEEK_CUR) == -1) {
			free(tmp_table);
			return 1;
		}
	}
	index_table->record = tmp_table;
	index_table->n_record = i + 1;
	return 0;
}

long index_table_key_offset(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	char* str_key = (char*)key;
	for (int i = 0; i < index_table->n_record; i++) {
		if (!strncmp(index_table->record[i].key_name, str_key, WORDLEN)) {
			return index_table->record[i].key_offset;
		}
	}
	return -1;
}

void* index_table_key_lock(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	char* str_key = (char*)key;
	for (int i = 0; i < index_table->n_record; i++) {
		if (!strncmp(index_table->record[i].key_name, str_key, WORDLEN)) {
			return &index_table->record[i].key_lock;
		}
	}
	return NULL;
}

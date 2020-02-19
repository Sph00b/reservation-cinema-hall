#include "index_table.h"

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "index_record.h"

struct index_table {
	int n_record;
	index_record_t* record;
	pthread_rwlock_t lock;
};

index_table_t index_table_init() {
	struct index_table* index_table;
	if ((index_table = malloc(sizeof(struct index_table))) == NULL) {
		return NULL;
	}
	index_table->n_record = 0;
	index_table->record = NULL;
	int ret;
	while ((ret = pthread_rwlock_init(&index_table->lock, NULL)) && errno == EINTR);
	if (ret) {
		free(index_table);
		return NULL;
	}
	return index_table;
}

int index_table_destroy(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_destroy(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	while (index_table->n_record) {
		if (index_record_destroy(index_table->record[index_table->n_record - 1])) {
			return 1;
		}
		index_table->n_record--;
	}
	free(index_table->record);
	free(index_table);
	return 0;
}

int index_table_wrlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_wrlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_rdlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_rdlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_unlock(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_unlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int index_table_load(const index_table_t handle, void** storage) {
	struct index_table* index_table = (struct index_table*)handle;
	while (index_record_storage_is_valid(storage)) {
		if ((index_table->record = realloc(index_table->record, sizeof(index_record_t) * (size_t)(index_table->n_record + 1))) == NULL) {
			return 1;
		}
		if ((index_table->record[index_table->n_record] = index_record_init(storage)) == NULL) {
			return 1;
		}
		index_table->n_record++;
	}
	return 0;
}

int index_table_update(const index_table_t handle, void** storage) {
	struct index_table* index_table = (struct index_table*)handle;
	if ((index_table->record = realloc(index_table->record, sizeof(index_record_t) * (size_t)(index_table->n_record + 1))) == NULL) {
		return 1;
	}
	if ((index_table->record[index_table->n_record] = index_record_init(storage)) == NULL) {
		return 1;
	}
	index_table->n_record++;
	return 0;
}

int index_table_record_wrlock(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	for (int i = 0; i < index_table->n_record; i++) {
		if (index_record_compare(index_table->record[i], key)) {
			return index_record_wrlock(index_table->record[i]);
		}
	}
	return 1;
}

int index_table_record_rdlock(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	for (int i = 0; i < index_table->n_record; i++) {
		if (index_record_compare(index_table->record[i], key)) {
			return index_record_rdlock(index_table->record[i]);
		}
	}
	return 1;
}

int index_table_record_unlock(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	for (int i = 0; i < index_table->n_record; i++) {
		if (index_record_compare(index_table->record[i], key)) {
			return index_record_unlock(index_table->record[i]);
		}
	}
	return 1;
}

/*	Non gestisco l'errore qui, assumo che vada sempre tutto bene	*/
int index_table_record_locate(const index_table_t handle, const void* key, void** storage) {
	struct index_table* index_table = (struct index_table*)handle;
	for (int i = 0; i < index_table->n_record; i++) {
		if (index_record_compare(index_table->record[i], key)) {
			return index_record_locate(index_table->record[i], storage);
		}
	}
	return 1;
}

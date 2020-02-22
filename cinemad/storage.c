#include "index_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

#include "storage.h"
#include "index_record.h"
#include "resources.h"

struct storage {
	int fd;
	index_table_t index_table;
};

FILE* storage_get_stream(const storage_t handle);

storage_t storage_init(char* filename) {
	struct storage* storage;
	FILE* stream;
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
	if ((storage->index_table = index_table_init()) == NULL) {
		close(storage->fd);
		free(storage);
		return NULL;
	}
	if ((stream = storage_get_stream(storage)) == NULL){
		index_table_destroy(storage->index_table);
		close(storage->fd);
		free(storage);
		return NULL;
	}
	if (index_table_load(storage->index_table, (void**)&stream)) {
		fclose(stream);
		index_table_destroy(storage->index_table);
		close(storage->fd);
		free(storage);
		return NULL;
	}
	if (fclose(stream) == EOF) {
		index_table_destroy(storage->index_table);
		close(storage->fd);
		free(storage);
		return NULL;
	}
	return storage;
}

int storage_close(const storage_t handle) {
	struct storage* storage = (struct storage*)handle;
	index_table_destroy(storage->index_table);
	close(storage->fd);
	free(storage);
	return 0;
}

int storage_store(const storage_t handle, char* key, char* value) {
	struct storage* storage = (struct storage*)handle;
	FILE* stream;
	if ((stream = storage_get_stream(storage)) == NULL) {
		return 1;
	}
	if (index_table_record_locate(storage->index_table, key, (void**)&stream)) {

		/*	Create the key	*/
		if (index_table_record_locate(storage->index_table, key, (void**)&stream)) {
			if (fseek(stream, 0, SEEK_END) == -1) {
				return 1;
			}
			for (int i = 0; i < WORDLEN; i++) {
				if (fputc((int)key, stream) == EOF) {
					return 1;
				}
			}
			for (int i = 0; i < WORDLEN; i++) {
				if (fputc(0, stream) == EOF) {
					return 1;
				}
			}
			fflush(stream);
			if (fseek(stream, -2 * WORDLEN, SEEK_END) == -1) {
				return 1;
			}
			if (index_table_update(storage->index_table, (void**)&stream)) {
				return 1;
			}
			/*		*/
		}
	}
	for (int i = 0; i < WORDLEN; i++) {
		if (fputc(value[i], stream) == EOF) {
			return 1;
		}
	}
	return 0;
}

int storage_load(const storage_t handle, char* key, char** result) {
	struct storage* storage = (struct storage*)handle;
	FILE* stream;
	if ((stream = storage_get_stream(storage)) == NULL) {
		return 1;
	}
	if (index_table_record_locate(storage->index_table, key, (void**)&stream)) {
		return 1;
	}
	for (int i = 0; i < WORDLEN; i++) {
		if ((int)((*result)[i] = (char)fgetc(stream)) == EOF) {
			return 1;
		}
	}
	(result)[WORDLEN] = 0;
	return 0;
}

int storage_lock_shared(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	return index_table_record_rdlock(storage->index_table, key);
}

int storage_lock_exclusive(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	return index_table_record_wrlock(storage->index_table, key);
}

int storage_unlock(const storage_t handle, char* key) {
	struct storage* storage = (struct storage*)handle;
	return index_table_record_unlock(storage->index_table, key);
}

FILE* storage_get_stream(const storage_t handle) {
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

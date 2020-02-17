#include "concurrent_flag.h"
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "flag.h"

struct concurrent_flag {
	flag_t flag;
	pthread_mutex_t mutex;
};

concurrent_flag_t concurrent_flag_init() {
	int ret;
	struct concurrent_flag* concurrent_flag;
	if ((concurrent_flag = malloc(sizeof(struct concurrent_flag))) == NULL) {
		return NULL;
	}
	if ((concurrent_flag->flag = flag_init()) == NULL) {
		free(concurrent_flag);
		return NULL;
	}
	while ((ret = pthread_mutex_init(&concurrent_flag->mutex, NULL)) && errno == EINTR);
	if (ret) {
		flag_destroy(concurrent_flag->flag);
		free(concurrent_flag);
		return NULL;
	}
	return concurrent_flag;
}

int concurrent_flag_destroy(const concurrent_flag_t handle) {
	int ret;
	struct concurrent_flag* concurrent_flag = (struct concurrent_flag*)handle;
	while ((ret = pthread_mutex_destroy(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	flag_destroy(concurrent_flag->flag);
	free(concurrent_flag);
	return 0;
}

int concurrent_flag_status(const concurrent_flag_t handle, int* result) {
	int ret;
	struct concurrent_flag* concurrent_flag = (struct concurrent_flag*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	*result = flag_status(concurrent_flag->flag);
	while ((ret = pthread_mutex_unlock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int concurrent_flag_set(const concurrent_flag_t handle) {
	int ret;
	struct concurrent_flag* concurrent_flag = (struct concurrent_flag*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	flag_set(concurrent_flag->flag);
	while ((ret = pthread_mutex_unlock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int concurrent_flag_unset(const concurrent_flag_t handle) {
	int ret;
	struct concurrent_flag* concurrent_flag = (struct concurrent_flag*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	flag_unset(concurrent_flag->flag);
	while ((ret = pthread_mutex_unlock(&concurrent_flag->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

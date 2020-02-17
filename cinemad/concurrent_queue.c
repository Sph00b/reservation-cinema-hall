#include "concurrent_queue.h"
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "queue.h"

struct concurrent_queue {
	queue_t queue;
	pthread_mutex_t mutex;
};

concurrent_queue_t concurrent_queue_init() {
	int ret;
	struct concurrent_queue* concurrent_queue;
	if ((concurrent_queue = malloc(sizeof(struct concurrent_queue))) == NULL) {
		return NULL;
	}
	if ((concurrent_queue->queue = queue_init()) == NULL) {
		free(concurrent_queue);
		return NULL;
	}
	while ((ret = pthread_mutex_init(&concurrent_queue->mutex, NULL)) && errno == EINTR);
	if (ret) {
		queue_destroy(concurrent_queue->queue);
		free(concurrent_queue);
		return NULL;
	}
	return concurrent_queue;
}

int concurrent_queue_destroy(const concurrent_queue_t handle) {
	int ret;
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	while ((ret = pthread_mutex_destroy(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	queue_destroy(concurrent_queue->queue);
	free(concurrent_queue);
	return 0;
}

int concurrent_queue_is_empty(const concurrent_queue_t handle, int* result) {
	int ret;
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	*result = queue_is_empty(concurrent_queue->queue);
	while ((ret = pthread_mutex_unlock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

int concurrent_queue_enqueue(const concurrent_queue_t handle, void* item) {
	int ret;
	int result;
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	result = queue_enqueue(concurrent_queue->queue, item);
	while ((ret = pthread_mutex_unlock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return result;
}

int concurrent_queue_dequeue(const concurrent_queue_t handle, void** result) {
	int ret;
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	while ((ret = pthread_mutex_lock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	*result = queue_dequeue(concurrent_queue->queue);
	while ((ret = pthread_mutex_unlock(&concurrent_queue->mutex)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return 0;
}

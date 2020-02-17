#include "concurrent_queue.h"
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "queue.h"

struct concurrent_queue {
	concurrent_queue_t queue;
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

void concurrent_queue_destroy(const concurrent_queue_t handle) {
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	queue_destroy(concurrent_queue->queue);
	while (pthread_mutex_destroy(&concurrent_queue->mutex) && errno == EINTR);
	free(concurrent_queue);
}

int concurrent_queue_is_empty(const concurrent_queue_t handle) {
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	int is_empty;
	while (pthread_mutex_lock(&concurrent_queue->mutex) && errno == EINTR);
	is_empty = queue_is_empty(concurrent_queue->queue);
	while (pthread_mutex_unlock(&concurrent_queue->mutex) && errno == EINTR);
	return is_empty;
}

int concurrent_queue_push(const concurrent_queue_t handle, void* data) {
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	int ret;
	while (pthread_mutex_lock(&concurrent_queue->mutex) && errno == EINTR);
	ret = queue_push(concurrent_queue->queue, data);
	while (pthread_mutex_unlock(&concurrent_queue->mutex) && errno == EINTR);
	return ret;
}

void* concurrent_queue_pop(const concurrent_queue_t handle) {
	struct concurrent_queue* concurrent_queue = (struct concurrent_queue*)handle;
	void* popped;
	while (pthread_mutex_lock(&concurrent_queue->mutex) && errno == EINTR);
	popped = queue_pop(concurrent_queue->queue);
	while (pthread_mutex_unlock(&concurrent_queue->mutex) && errno == EINTR);
	return popped;
}

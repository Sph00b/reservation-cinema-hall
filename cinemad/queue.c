#include "queue.h"
#include <stdlib.h>

#include "stack.h"

struct queue {
	_stack_t stack_in;
	_stack_t stack_out;
};

queue_t queue_init() {
	struct queue* queue;
	if ((queue = malloc(sizeof(struct queue))) == NULL) {
		return NULL;
	}
	if ((queue->stack_in = stack_init()) == NULL) {
		free(queue);
		return NULL;
	}
	if ((queue->stack_out = stack_init()) == NULL) {
		free(queue);
		return NULL;
	}
	return queue;
}

void queue_destroy(const queue_t handle) {
	struct queue* queue = (struct queue*)handle;
	stack_destroy(queue->stack_in);
	stack_destroy(queue->stack_out);
	free(queue);
}

int queue_is_empty(const queue_t handle) {
	struct queue* queue = (struct queue*)handle;
	return (stack_is_empty(queue->stack_in) && stack_is_empty(queue->stack_out));
}

int queue_enqueue(const queue_t handle, void* item) {
	struct queue* queue = (struct queue*)handle;
	return stack_push(queue->stack_in, item);
}

void* queue_dequeue(const queue_t handle) {
	struct queue* queue = (struct queue*)handle;
	if (stack_is_empty(queue->stack_out)) {
		while (!stack_is_empty(queue->stack_in)) {
			stack_push(queue->stack_out, stack_pop(queue->stack_in));
		}
	}
	return stack_pop(queue->stack_out);
}

#include "queue.h"
#include <stdlib.h>

int queue_init(queue_t* queue) {
	stack_init(&(queue->stack_in));
	stack_init(&(queue->stack_out));
	return 0;
}

int queue_is_empty(queue_t* queue) {
	return (stack_is_empty(&(queue->stack_in)) && stack_is_empty(&(queue->stack_out)));
}

int queue_push(queue_t* queue, void* data) {
	return stack_push(&(queue->stack_in), data);
}

void* queue_pop(queue_t* queue) {
	if (queue_is_empty(queue)) {
		return NULL;
	}
	if (stack_is_empty(&(queue->stack_out))) {
		while (!stack_is_empty(&(queue->stack_in))) {
			stack_push(&(queue->stack_out), stack_pop(&(queue->stack_in)));
		}
	}
	return stack_pop(&(queue->stack_out));
}

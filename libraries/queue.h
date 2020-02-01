#pragma once
#include "stack.h"

typedef struct {
	_stack_t stack_in;
	_stack_t stack_out;
} _queue_t;

int queue_init(queue_t* queue);

int queue_is_empty(queue_t* queue);

int queue_push(queue_t* queue, void* data);

void* queue_pop(queue_t* queue);

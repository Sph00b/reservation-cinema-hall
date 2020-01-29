#pragma once

typedef struct _stack_node {
	void* data;
	struct _stack_node* next;
} *_stack_t;

int stack_is_empty(_stack_t* pstack);

int stack_push(_stack_t* pstack, void* data);

void* stack_pop(_stack_t* pstack);

void* stack_peek(_stack_t* pstack);

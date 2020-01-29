#include "stack.h"
#include <stdlib.h>

int stack_is_empty(_stack_t* stack) {
	return !*stack;
}

int stack_push(_stack_t* stack, void* data) {
	struct _stack_node* node;
	if ((node = (struct _stack_node*)malloc(sizeof(struct _stack_node))) == NULL) {
		return 1;
	}
	node->data = data;
	node->next = *stack;
	*stack = node;
	return 0;
}

void* stack_pop(_stack_t* stack) {
	void* popped;
	if (stack_is_empty(stack)) {
		return NULL;
	}
	popped = (*stack)->data;
	*stack = (*stack)->next;
	return popped;
}

void* stack_peek(_stack_t* stack) {
	if (stack_is_empty(stack)) {
		return NULL;
	}
	return (*stack)->data;
}

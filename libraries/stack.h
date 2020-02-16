#pragma once

typedef void* _stack_t;

_stack_t stack_init();

void stack_destroy(_stack_t handle);

int stack_is_empty(_stack_t handle);

int stack_push(_stack_t handle, void* data);

void* stack_pop(_stack_t handle);

void* stack_peek(_stack_t handle);

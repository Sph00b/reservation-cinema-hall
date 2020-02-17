#pragma once

typedef void* _stack_t;

_stack_t stack_init();

void stack_destroy(const _stack_t handle);

int stack_is_empty(const _stack_t handle);

int stack_push(const _stack_t handle, void* item);

void* stack_pop(const _stack_t handle);

void* stack_peek(const _stack_t handle);

#pragma once

typedef void* queue_t;

queue_t queue_init();

void queue_destroy(const queue_t handle);

int queue_is_empty(const queue_t handle);

int queue_push(const queue_t handle, void* data);

void* queue_pop(const queue_t handle);

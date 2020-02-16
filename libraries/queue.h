#pragma once

typedef void* queue_t;

queue_t queue_init();

void queue_destroy(queue_t handle);

int queue_is_empty(queue_t handle);

int queue_push(queue_t handle, void* data);

void* queue_pop(queue_t handle);

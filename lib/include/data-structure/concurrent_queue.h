#pragma once

typedef void* concurrent_queue_t;

concurrent_queue_t concurrent_queue_init();

int concurrent_queue_destroy(const concurrent_queue_t handle);

int concurrent_queue_is_empty(const concurrent_queue_t handle, int* result);

int concurrent_queue_enqueue(const concurrent_queue_t handle, void* item);

int concurrent_queue_dequeue(const concurrent_queue_t handle, void** result);

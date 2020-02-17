#pragma once

/*	Al contrario delle strutture dati precedenti queste funioni possono fallie, pertanto molte funzioni vanno ristrutturate*/

typedef void* concurrent_queue_t;

concurrent_queue_t concurrent_queue_init();

void concurrent_queue_destroy(const concurrent_queue_t handle);

int concurrent_queue_is_empty(const concurrent_queue_t handle);

int concurrent_queue_push(const concurrent_queue_t handle, void* data);

void* concurrent_queue_pop(const concurrent_queue_t handle);

#pragma once

typedef void* list_t;

list_t list_init();

int list_destroy(list_t handle);

int list_lenght(list_t handle);

int list_append(list_t handle, void* item);
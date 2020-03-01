#pragma once

typedef void* index_table_t;

index_table_t index_table_init(int (*comparison_function)(const void* key1, const void* key2));

int index_table_destroy(index_table_t handle, int (*record_destroy)(void* key, void* value));

int index_table_insert(index_table_t handle, const void* key, const void* record);

void* index_table_search(index_table_t handle, const void* key);

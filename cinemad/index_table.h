#pragma once

typedef void* index_record_t;
typedef void* index_table_t;

index_table_t index_table_init(
    index_record_t (*record_init)(index_table_t index_table, void* key),
    int (*record_destroy)(void* key, void* value),
    int (*comparison_function)(const void* key1, const void* key2));

int index_table_destroy(index_table_t handle);

int index_table_insert(index_table_t handle, const void* key, const void* record);

int index_table_delete(index_table_t handle, const void* key);

index_record_t index_table_search(index_table_t handle, const void* key);

#pragma once

typedef void* index_table_t;

index_table_t index_table_init();

int index_table_destroy(index_table_t handle);

int index_table_insert(index_table_t handle, const void* key, const void* record);

void* index_table_search(index_table_t handle, const void* key);
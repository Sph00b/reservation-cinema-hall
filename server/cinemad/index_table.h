#pragma once

typedef void* index_record_t;
typedef void* index_table_t;

index_table_t index_table_init(
    const index_record_t (*record_init)(),
    const int (*record_destroy)(void* key, void* value),
    const int (*comparison_function)(const void* key1, const void* key2)
);

extern int index_table_destroy(
    const index_table_t handle
);

extern int index_table_insert(
    const index_table_t handle,
    const void* key, 
    const void* record
);

extern int index_table_delete(
    const index_table_t handle,
    const void* key
);

extern index_record_t index_table_search(
    const index_table_t handle,
    void* key
);

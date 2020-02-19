#pragma once

typedef void* index_table_t;

extern index_table_t index_table_init();

extern int index_table_destroy(const index_table_t handle);

extern int index_table_wrlock(const index_table_t handle);

extern int index_table_rdlock(const index_table_t handle);

extern int index_table_unlock(const index_table_t handle);

extern int index_table_update(const index_table_t handle, void* storage);

extern long index_table_key_offset(const index_table_t handle, const void* key);

extern void* index_table_key_lock(const index_table_t handle, const void* key);

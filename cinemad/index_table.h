#pragma once

typedef void* index_table_t;

extern index_table_t index_table_init();

extern int index_table_destroy(const index_table_t handle);

extern int index_table_wrlock(const index_table_t handle);

extern int index_table_rdlock(const index_table_t handle);

extern int index_table_unlock(const index_table_t handle);

extern int index_table_load(const index_table_t handle, void** storage);

extern int index_table_update(const index_table_t handle, void** storage);

extern int index_table_record_wrlock(const index_table_t handle, const void* key);

extern int index_table_record_rdlock(const index_table_t handle, const void* key);

extern int index_table_record_unlock(const index_table_t handle, const void* key);

extern int index_table_record_locate(const index_table_t handle, const void* key, void** storage);

#pragma once

typedef void* index_record_t;

extern index_record_t index_record_init(void** storage);

extern int index_record_destroy(const index_record_t handle);

extern int index_record_wrlock(const index_record_t handle);

extern int index_record_rdlock(const index_record_t handle);

extern int index_record_unlock(const index_record_t handle);

extern int index_record_compare(const index_record_t handle, const void* key);

extern int index_record_locate(const index_record_t handle, void** location);

/*	I definetly need a storage structure	*/
extern int index_record_storage_is_valid(void** storage);

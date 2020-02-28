#pragma once

#define MSG_SUCC "OPERATION SUCCEDED"
#define MSG_FAIL "OPERATION FAILED"

typedef void* storage_t;

storage_t storage_init(const char* filename);

int storage_close(const storage_t handle);

int storage_store(const storage_t handle, const char* key, const char* value, char** result);

int storage_load(const storage_t handle, const char* key, char** result);

int storage_lock_shared(const storage_t handle, const char* key);

int storage_lock_exclusive(const storage_t handle, const char* key);

int storage_unlock(const storage_t handle, const char* key);

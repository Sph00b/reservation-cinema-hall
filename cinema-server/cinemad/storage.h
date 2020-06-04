#pragma once

#define MSG_SUCC "OPERATION SUCCEDED"
#define MSG_FAIL "OPERATION FAILED"

typedef void* storage_t;

/*	Create storage, return database handle on success or return NULL and set properly errno on error */

storage_t storage_init(const char* filename);

/*	Close the storage, return 0 on succes or return 1 and set properly errno on error */

int storage_close(const storage_t handle);

/*	Store the value linked to the key in the storage */

int storage_store(const storage_t handle, const char* key, const char* value, char** result);

/*	Load the value linked to the key from the storage */

int storage_load(const storage_t handle, const char* key, char** result);

/*	Llock as shared the lock linked to the key */

int storage_lock_shared(const storage_t handle, const char* key);

/*	Llock as exclusive the lock linked to the key */

int storage_lock_exclusive(const storage_t handle, const char* key);

/*	Unlock the lock linked to the key */

int storage_unlock(const storage_t handle, const char* key);

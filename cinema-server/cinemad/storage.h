#pragma once

#define MSG_SUCC "OPERATION SUCCEDED"
#define MSG_FAIL "OPERATION FAILED"

typedef void* storage_t;

/*
* Create storage.
* 
* @return	database handle on success or return NULL and properly errno 
*			on error.
*/
extern storage_t storage_init(
	const char* filename
);

/*
* Close the storage.
* 
* @return	0 on success or return 1 and set properly errno on error.
*/
extern int storage_close(
	const storage_t handle
);

/*
* Store the value linked to the key in the storage.
*/
extern int storage_store(
	const storage_t handle, 
	const char* key, 
	const char* value, 
	char** result
);

/*
* Load the value linked to the key from the storage.
*/
extern int storage_load(
	const storage_t handle, 
	const char* key, 
	char** result
);

/*
* Lock as shared the lock linked to the key.
*/
extern int storage_lock_shared(
	const storage_t handle, 
	const char* key
);

/*
* Lock as exclusive the lock linked to the key.
*/
extern int storage_lock_exclusive(
	const storage_t handle, 
	const char* key
);

/*
* Unlock the lock linked to the key.
*/
extern int storage_unlock(
	const storage_t handle, 
	const char* key
);

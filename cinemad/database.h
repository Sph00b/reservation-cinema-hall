#pragma once

typedef void* database_t;

extern database_t database_init(const char *filename);

extern int database_close(const database_t handle);

extern int database_execute(const database_t handle, const char *query, char **result);

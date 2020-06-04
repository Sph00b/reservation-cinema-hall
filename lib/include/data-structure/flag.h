#pragma once

typedef void* flag_t;

flag_t flag_init();

void flag_destroy(const flag_t handle);

int flag_status(const flag_t handle);

void flag_set(const flag_t handle);

void flag_unset(const flag_t handle);

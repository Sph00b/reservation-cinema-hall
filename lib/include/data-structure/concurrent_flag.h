#pragma once

typedef void* concurrent_flag_t;

concurrent_flag_t concurrent_flag_init();

int concurrent_flag_destroy(const concurrent_flag_t handle);

int concurrent_flag_status(const concurrent_flag_t handle, int* result);

int concurrent_flag_set(const concurrent_flag_t handle);

int concurrent_flag_unset(const concurrent_flag_t handle);

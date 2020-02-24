#pragma once

#include "node.h"

typedef void* bs_node_t;

bs_node_t bs_node_init(void* key, void* value);

int bs_node_destroy(bs_node_t handle);

void* bs_node_get_key(bs_node_t handle);

void* bs_node_get_value(bs_node_t handle);

int bs_node_is_left_son(bs_node_t handle);

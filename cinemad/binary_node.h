#pragma once

#include "node.h"

typedef void* binary_node_t;

binary_node_t binary_node_init(void* info);

int binary_node_destroy(binary_node_t handle);

binary_node_t binary_node_get_left_son(binary_node_t handle);

binary_node_t binary_node_get_right_son(binary_node_t handle);

void binary_node_set_left_son(binary_node_t handle, binary_node_t left_son);

void binary_node_set_right_son(binary_node_t handle, binary_node_t right_son);

int binary_node_is_left_son(binary_node_t handle);

int binary_node_is_right_son(binary_node_t handle);

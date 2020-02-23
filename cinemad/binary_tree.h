#pragma once

#include "tree.h"
#include "bool.h"

typedef void* binary_node_t;
typedef void* binary_tree_t;

binary_tree_t binary_tree_init(binary_node_t root);

int binary_tree_destroy(binary_tree_t handle);

bool_t binary_node_is_leaf(binary_node_t handle);

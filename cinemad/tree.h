#pragma once

#include "list.h"

typedef void* node_t;
typedef void* tree_t;

long tree_nodes_number(tree_t handle);

long tree_degree(tree_t handle, node_t node);

node_t tree_get_root(tree_t handle);

node_t tree_get_father(tree_t handle, node_t node);

list_t tree_get_sons(tree_t handle, node_t node);

void* tree_get_info(tree_t handle, node_t node);

tree_t tree_cut(tree_t handle, node_t node);

#pragma once

typedef void* binary_tree_node_t;

binary_tree_node_t binary_tree_node_init(void* info);

int binary_tree_node_destroy(binary_tree_node_t handle);

long binary_tree_node_degree(binary_tree_node_t handle);

int binary_tree_node_swap(binary_tree_node_t handle1, binary_tree_node_t handle2);

void* binary_tree_node_get_info(binary_tree_node_t handle);

binary_tree_node_t binary_tree_node_get_father(binary_tree_node_t handle);

int binary_tree_node_set_father(binary_tree_node_t handle, binary_tree_node_t father);

binary_tree_node_t binary_tree_node_get_left_son(binary_tree_node_t handle);

int binary_tree_node_set_left_son(binary_tree_node_t handle, binary_tree_node_t left_son);

binary_tree_node_t binary_tree_node_get_right_son(binary_tree_node_t handle);

int binary_tree_node_set_right_son(binary_tree_node_t handle, binary_tree_node_t right_son);

int binary_tree_node_is_left_son(binary_tree_node_t handle);

int binary_tree_node_is_right_son(binary_tree_node_t handle);

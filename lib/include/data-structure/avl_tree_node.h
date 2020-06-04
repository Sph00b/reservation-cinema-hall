#pragma once

typedef void* avl_tree_node_t;

avl_tree_node_t avl_tree_node_init(const void* key, const void* value);

int avl_tree_node_destroy(const avl_tree_node_t handle);

long avl_tree_node_degree(const avl_tree_node_t handle);

int avl_tree_node_swap(const avl_tree_node_t handle1, const avl_tree_node_t handle2);

void* avl_tree_node_get_key(const avl_tree_node_t handle);

void* avl_tree_node_get_value(const avl_tree_node_t handle);

int avl_tree_node_get_height(const avl_tree_node_t handle);

int avl_tree_node_set_height(const avl_tree_node_t handle, const int height);

void avl_tree_node_update_height(const avl_tree_node_t handle);

int avl_tree_node_get_balance_factor(const avl_tree_node_t handle);

avl_tree_node_t avl_tree_node_get_father(const avl_tree_node_t handle);

int avl_tree_node_set_father(const avl_tree_node_t handle, const avl_tree_node_t father);

avl_tree_node_t avl_tree_node_get_left_son(const avl_tree_node_t handle);

int avl_tree_node_set_left_son(const avl_tree_node_t handle, const avl_tree_node_t left_son);

avl_tree_node_t avl_tree_node_get_right_son(const avl_tree_node_t handle);

int avl_tree_node_set_right_son(const avl_tree_node_t handle, const avl_tree_node_t right_son);

int avl_tree_node_is_left_son(const avl_tree_node_t handle);

int avl_tree_node_is_right_son(const avl_tree_node_t handle);

avl_tree_node_t avl_tree_node_get_max(const avl_tree_node_t node);

avl_tree_node_t avl_tree_node_get_pred(const avl_tree_node_t node);

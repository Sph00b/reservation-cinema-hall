#pragma once

typedef void* bst_node_t;

bst_node_t bst_node_init(void* key, void* value);

int bst_node_destroy(bst_node_t handle);

long bst_node_degree(bst_node_t handle);

int bst_node_swap(bst_node_t handle1, bst_node_t handle2);

void* bst_node_get_info(bst_node_t handle);

void* bst_node_get_key(bst_node_t handle);

void* bst_node_get_value(bst_node_t handle);

bst_node_t bst_node_get_father(bst_node_t handle);

int bst_node_set_father(bst_node_t handle, bst_node_t father);

bst_node_t bst_node_get_left_son(bst_node_t handle);

int bst_node_set_left_son(bst_node_t handle, bst_node_t left_son);

bst_node_t bst_node_get_right_son(bst_node_t handle);

int bst_node_set_right_son(bst_node_t handle, bst_node_t right_son);

int bst_node_is_left_son(bst_node_t handle);

int bst_node_is_right_son(bst_node_t handle);

bst_node_t bst_node_get_max(bst_node_t handle);

bst_node_t bst_node_get_pred(bst_node_t handle);
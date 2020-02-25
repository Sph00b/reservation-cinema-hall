#pragma once

#include "avl_tree_node.h"

typedef void* avl_tree_t;

typedef int* avl_tree_comparison_function(const void* key1, const void* key2);

avl_tree_t avl_tree_init(avl_tree_node_t root, avl_tree_comparison_function* comparison_function);

int avl_tree_destroy(avl_tree_t handle);

long avl_tree_nodes_number(avl_tree_t handle);

avl_tree_node_t avl_tree_get_root(avl_tree_t handle);

int avl_tree_set_root(avl_tree_t handle, avl_tree_node_t node);

int avl_tree_insert_as_left_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree);

int avl_tree_insert_as_right_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree);

avl_tree_t avl_tree_cut(avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_left(avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_right(avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_one_son_node(avl_tree_t handle, avl_tree_node_t node);

void avl_tree_insert_single_node_tree(avl_tree_t handle, void* key, avl_tree_t new_tree);

avl_tree_node_t avl_tree_search_node(avl_tree_t handle, void* key);

void* avl_tree_search(avl_tree_t handle, void* key);

int avl_tree_insert(avl_tree_t handle, void* key, void* value);

void avl_tree_delete(avl_tree_t handle, void* key);
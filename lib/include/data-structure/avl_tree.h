#pragma once

#include "avl_tree_node.h"

typedef void* avl_tree_t;

typedef int avl_tree_comparison_function(const void* key1, const void* key2);

avl_tree_t avl_tree_init(const avl_tree_comparison_function* comparison_function);

int avl_tree_destroy(const avl_tree_t handle);

long avl_tree_nodes_number(const avl_tree_t handle);

avl_tree_node_t avl_tree_get_root(const avl_tree_t handle);

void avl_tree_set_root(const avl_tree_t handle, avl_tree_node_t node);

void avl_tree_insert_as_left_subtree(const avl_tree_t handle, avl_tree_node_t node, const avl_tree_t subtree);

void avl_tree_insert_as_right_subtree(const avl_tree_t handle, avl_tree_node_t node, const avl_tree_t subtree);

avl_tree_t avl_tree_cut(const avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_left(const avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_right(const avl_tree_t handle, avl_tree_node_t node);

avl_tree_t avl_tree_cut_one_son_node(const avl_tree_t handle, avl_tree_node_t node);

void avl_tree_insert_single_node_tree(const avl_tree_t handle, const void* key, const avl_tree_t new_tree);

avl_tree_node_t avl_tree_search_node(const avl_tree_t handle, const void* key);

void* avl_tree_search(const avl_tree_t handle, const void* key);

int avl_tree_insert(const avl_tree_t handle, const void* key, const void* value);

int avl_tree_delete(const avl_tree_t handle, const void* key);

int avl_tree_delete_node(const avl_tree_t handle, avl_tree_node_t node);

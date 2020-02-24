#pragma once

typedef void* avl_tree_t;

extern avl_tree_t avl_tree_init(void* (*order_function)(void*, void*));

extern int avl_tree_insert(avl_tree_t handle, void* key, void* value);

extern int avl_tree_insert(avl_tree_t handle, void* key, void* value);

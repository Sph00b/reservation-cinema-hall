#pragma once

typedef void* avl_tree;

extern avl_tree avl_tree_init(void* (*order_function)(void*, void*));

extern int avl_tree_insert(avl_tree handle, void* key, void* value);

extern int avl_tree_insert(avl_tree handle, void* key, void* value);

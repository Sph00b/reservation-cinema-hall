#include "bst.h"

#include <stdlib.h>

#include "binary_tree.h"

#define KEY_INDEX = 0	//indici usati per trovare le relative informazioni da un nodo
#define	VALUE_INDEX = 1	//nodo.info[KEY_INDEX] è la chiave del nodo

struct bst {
	binary_tree_t tree;
	void* (*order_function)(void*, void*);
};

int default_order(void* key1, void* key2) {
	if (key1 < key2) {
		return -1;
	}
	else if (key1 > key2) {
		return 1;
	}
	return 0;
}

bst_t bst_init(void* (*order_function)(void*, void*)) {
	struct bst* bst;
	if ((bst = malloc(sizeof(struct bst))) == NULL) {
		return NULL;
	}
	if ((bst->tree = binary_tree_init(NULL)) == NULL)  {
		free(bst);
		return NULL;
	}
	if (order_function) {
		bst->order_function = order_function;
	}
	else {
		bst->order_function = &default_order;
	}
	return bst;
}

int bst_destroy(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	if (binary_tree_destroy(bst->tree)) {
		return 1;
	}
	free(bst);
	return 0;
}

node_t bst_search_node(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t current = tree_get_root(handle);
	while (current != NULL) {
		void* current_key = get_key(handle, current);
		if (key == current_key) {
			return current;
		}
		if (key < current_key) {
			current = binary_tree_get_left_son(handle, current);
		}
		else {
			current = binary_tree_get_right_son(handle, current);
		}
	}
	return NULL;
}

void cut_one_son_node(bst_t handle, node_t node) {
	struct bst* bst = (struct bst*)handle;
	node_t son = NULL;
	if ((son = binary_tree_get_left_son(handle, node)) == NULL) {
		son = binary_tree_get_right_son(handle, node);
	}
	if (son == NULL) {
		tree_cut(handle, son);
	}
	else {
		tree_swap_info(node, son);
		tree_t tmp_tree = tree_cut(handle, son);
		binary_tree_insert_as_left_subtree(handle, node, tree_cut(tmp_tree, binary_tree_get_left_son(tree_get_root(tmp_tree))));
		binary_tree_insert_as_right_subtree(handle, node, tree_cut(tmp_tree, binary_tree_get_right_son(tree_get_root(tmp_tree))));
	}
}

node_t max(bst_t handle, node_t node) {
	struct bst* bst = (struct bst*)handle;
	node_t current = node;
	while (binary_tree_get_right_son(handle, current)) {
		current = binary_tree_get_right_son(handle, current);
	}
	return current;
}

node_t pred(bst_t handle, node_t node) {
	struct bst* bst = (struct bst*)handle;
	if (node == NULL) {
		return NULL;
	}
	if (binary_tree_get_left_son(handle, node)) {
		return max(binary_tree_get_left_son(handle, node));
	}
	node_t current = node;
	while (tree_get_father(handle, current) && binary_tree_is_left_son(handle, current)) {
		current = tree_get_father(handle, current);
	}
	return current;
}

long size(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	return tree_nodes_number(handle);
}

void* search(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t node;
	node = bst_search_node(handle, key);
	return get_value(handle, node);
}

void insert_single_node_tree(bst_t handle, void* key, bst_t new_tree) {
	struct bst* bst = (struct bst*)handle;
	if (tree_get_root(handle) == NULL) {
		tree_set_root(handle, tree_get_root(new_tree));
		tree_set_nodes_number(handle, 1);
	}
	else {
		node_t current = tree_get_root(handle);
		node_t pred = NULL;
		while (current) {
			pred = current;
			if (key <= get_key(handle, current)) {
				current = binary_tree_get_left_son(handle, current);
			}
			else {
				current = binary_tree_get_right_son(handle, current);
			}
		}
		if (key <= get_key(handle, pred)) {
			binary_tree_insert_as_left_subtree(handle, pred, new_tree);
		}
		else {
			binary_tree_insert_as_right_subtree(handle, pred, new_tree);
		}
	}
}

void insert(bst_t handle, void* key, void* value) {
	struct bst* bst = (struct bst*)handle;
	binary_tree_t tree = binary_tree_init(binary_node_init[key, value]);
	insert_single_node_tree(handle, key, tree);
}

void delete(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t node;
	if (node = bst_search_node(handle, key)) {
		if (list_lenght(tree_get_sons(handle, node)) < 2) {
			cut_one_son_node(handle, node);
		}
		else {
			node_t p = pred(handle, node);
			tree_swap_info(node, p);
			cut_one_son_node(handle, p);
		}
	}
}

#undef KEY_INDEX
#undef VALUE_INDEX

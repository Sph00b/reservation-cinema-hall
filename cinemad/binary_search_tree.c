#include "binary_search_tree.h"

#include <stdlib.h>

#include "binary_tree.h"

#define KEY_INDEX = 0	//indici usati per trovare le relative informazioni da un nodo
#define	VALUE_INDEX = 1	//nodo.info[KEY_INDEX] è la chiave del nodo

struct binary_search_tree {
	binary_tree_t binary_tree;
	void* (*order_function)(void*, void*);
};

binary_search_tree_t binary_search_tree_init(void* (*order_function)(void*, void*)) {
	struct binary_search_tree* binary_search_tree;
	if ((binary_search_tree = malloc(sizeof(struct binary_search_tree))) == NULL) {
		return NULL;
	}
	if ((binary_search_tree->binary_tree = binary_tree_init(NULL)) == NULL)  {
		free(binary_search_tree);
		return NULL;
	}
	binary_search_tree->order_function = order_function;
	return binary_search_tree;
}

int binary_search_tree_destroy(binary_search_tree_t handle) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	if (binary_tree_destroy(binary_search_tree->binary_tree)) {
		return 1;
	}
	free(binary_search_tree);
	return 0;
}

void* get_key(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	if (node == NULL) {
		return NULL;
	}
	void* info = tree_get_info(binary_search_tree->binary_tree, node);
	return info[KEY_INDEX];
}

void* get_value(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	if (node == NULL) {
		return NULL;
	}
	void* info = binary_tree_get_info(handle, node);
	return info[VALUE_INDEX];
}

bool_t is_left_son(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	if (node == binary_tree_get_left_son(handle, tree_get_father(handle, node))) {
		return true;
	}
	return false;
}

node_t binary_search_tree_search_node(binary_search_tree_t handle, void* key) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
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

void cut_one_son_node(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
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

node_t max(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	node_t current = node;
	while (binary_tree_get_right_son(handle, current)) {
		current = binary_tree_get_right_son(handle, current);
	}
	return current;
}

node_t pred(binary_search_tree_t handle, node_t node) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
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

long size(binary_search_tree_t handle) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	return tree_nodes_number(handle);
}

void* search(binary_search_tree_t handle, void* key) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	node_t node;
	node = binary_search_tree_search_node(handle, key);
	return get_value(handle, node);
}

void insert_single_node_tree(binary_search_tree_t handle, void* key, binary_search_tree_t new_tree) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
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

void insert(binary_search_tree_t handle, void* key, void* value) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	binary_tree_t tree = binary_tree_init(binary_node_init[key, value]);
	insert_single_node_tree(handle, key, tree);
}

void delete(binary_search_tree_t handle, void* key) {
	struct binary_search_tree* binary_search_tree = (struct binary_search_tree*)handle;
	node_t node;
	if (node = binary_search_tree_search_node(handle, key)) {
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

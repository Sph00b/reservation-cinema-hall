#include "bst.h"

#include <stdlib.h>

#include "binary_node.h"
#include "binary_tree.h"

struct bst {
	binary_tree_t tree;
	void* (*get_order)(void* key1, void* key2);
};

/*	Private implementation	*/

struct dictionary {
	void* key;
	void* value;
};

void* get_key(node_t node) {
	if (node) {
		return ((struct dictionary*)node_get_info(node))->key;
	}
	return NULL;
}

void* get_value(node_t node) {
	if (node) {
		return ((struct dictionary*)node_get_info(node))->value;
	}
	return NULL;
}

int default_order(void* key1, void* key2) {
	if (key1 < key2) {
		return -1;
	}
	else if (key1 > key2) {
		return 1;
	}
	return 0;
}

/*	*/

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
		bst->get_order = order_function;
	}
	else {
		bst->get_order = &default_order;
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

node_t search_node(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t current = tree_get_root(bst->tree);
	while (current) {
		void* current_key = get_key(current);
		int order = bst->get_order(key, current_key);
		if (order == 0) {
			return current;
		}
		if (order == -1) {
			current = binary_node_get_left_son(current);
		}
		else {
			current = binary_node_get_right_son(current);
		}
	}
	return NULL;
}

void cut_one_son_node(bst_t handle, node_t node) {
	struct bst* bst = (struct bst*)handle;
	node_t son;
	if (!(son = binary_node_get_left_son(node))) {
		son = binary_node_get_right_son(node);
	}
	if (!son) {		//foglia (null check necessario)
		tree_cut(bst->tree, node);
	}
	else {
		node_swap(node, son);
		tree_t tmp_tree = tree_cut(bst->tree, son);
		binary_tree_insert_as_left_subtree(bst->tree, node, tree_cut(tmp_tree, binary_node_get_left_son(son)));
		binary_tree_insert_as_right_subtree(bst->tree, node, tree_cut(tmp_tree, binary_node_get_right_son(son)));
		//memory leack?
	}
}

node_t max(node_t node) {
	node_t current = node;
	while (binary_node_get_right_son(current)) {
		current = binary_node_get_right_son(current);
	}
	return current;
}

node_t pred(node_t node) {
	if (node == NULL) {
		return NULL;
	}
	if (binary_node_get_left_son(node)) {
		return max(binary_node_get_left_son(node));
	}
	node_t current = node;
	while (binary_node_is_left_son(current)) {
		current = node_get_father(current);
	}
	return current;
}

long size(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	return tree_nodes_number(bst->tree);
}

void* search(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t node;
	node = search_node(bst, key);
	return get_value(node);
}

void insert_single_node_tree(bst_t handle, void* key, bst_t new_tree) {
	struct bst* bst = (struct bst*)handle;
	if (tree_get_root(bst->tree) == NULL) {
		tree_set_root(bst->tree, tree_get_root(new_tree));
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

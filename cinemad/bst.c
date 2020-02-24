#include "bst.h"

#include <stdlib.h>

#include "binary_node.h"
#include "binary_tree.h"

struct bst {
	binary_tree_t tree;
	int (*get_order)(void* key1, void* key2);
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

bst_t bst_init(int (*order_function)(void*, void*)) {
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
	return get_value(search_node(bst, key));
}

void insert_single_node_tree(bst_t handle, void* key, tree_t new_tree) {
	struct bst* bst = (struct bst*)handle;
	int order;

	if (tree_get_root(bst->tree) == NULL) {
		tree_set_root(bst->tree, tree_get_root(new_tree));
	}
	else {
		node_t current = tree_get_root(bst->tree);
		node_t prev = NULL;
		while (current) {
			prev = current;
			order = bst->get_order(key, get_key(current));
			if (order < 1) {
				current = binary_node_get_left_son(current);
			}
			else {
				current = binary_node_get_right_son(current);
			}
		}
		order = bst->get_order(key, get_key(current));
		if (order < 1) {
			binary_tree_insert_as_left_subtree(bst->tree, prev, new_tree);
		}
		else {
			binary_tree_insert_as_right_subtree(bst->tree, prev, new_tree);
		}
	}
}

int insert(bst_t handle, void* key, void* value) {
	struct bst* bst = (struct bst*)handle;
	struct dictionary* info;
	if ((info = malloc(sizeof(struct dictionary))) == NULL) {
		return 1;
	}
	info->key = key;
	info->value = value;
	binary_tree_t tree = binary_tree_init(binary_node_init(info));
	insert_single_node_tree(bst, key, tree);
	return 0;
}

void delete(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	node_t node;
	if (node = search_node(bst, key)) {
		if (node_degree(node) < 2) {
			cut_one_son_node(handle, node);
		}
		else {
			node_swap(node, pred(node));
			cut_one_son_node(handle, pred(node));
		}
	}
}

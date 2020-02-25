#include "bst.h"

#include <stdlib.h>

#include "binary_tree.h"

struct bst {
	binary_tree_t tree;
	bst_comparison_function* compare;
};

int defualt_comparison_function(void* key1, void* key2) {
	if (key1 < key2) {
		return -1;
	}
	else if (key1 > key2) {
		return 1;
	}
	return 0;
}

/*
---------------------------------------------------------------------------
*/


bst_t bst_init(bst_node_t root, bst_comparison_function* comparison_function) {
	struct bst* bst;
	if ((bst = malloc(sizeof(struct bst))) == NULL) {
		return NULL;
	}
	if ((bst->tree = binary_tree_init(root)) == NULL) {
		free(bst);
		return NULL;
	}
	bst->compare = comparison_function ? comparison_function : &defualt_comparison_function;
	return bst;
}

int bst_destroy(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	bst_t binary_left_subtree = bst_cut_left(bst, bst_get_root(bst));
	bst_t binary_right_subtree = bst_cut_right(bst, bst_get_root(bst));
	if (bst_destroy(binary_left_subtree)) {
		return 1;
	}
	if (bst_destroy(binary_right_subtree)) {
		return 1;
	}
	if (bst_node_destroy(bst_get_root(bst))) {
		return 1;
	}
	free(bst->tree);
	free(bst);
	return 0;
}

static inline long bst_nodes_number(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_nodes_number(bst->tree);
}

static inline bst_node_t bst_get_root(bst_t handle) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_get_root(bst->tree);
}

static inline int bst_set_root(bst_t handle, bst_node_t node) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_set_root(bst->tree, node);
}

static inline int bst_insert_as_left_subtree(bst_t handle, bst_node_t node, bst_t subtree) {
	struct bst* bst = (struct bst*)handle;
	struct bst* bst_subtree = (struct bst*)subtree;
	return binary_tree_insert_as_left_subtree(bst->tree, node, bst_subtree->tree);
}

static inline int bst_insert_as_right_subtree(bst_t handle, bst_node_t node, bst_t subtree) {
	struct bst* bst = (struct bst*)handle;
	struct bst* bst_subtree = (struct bst*)subtree;
	return binary_tree_insert_as_right_subtree(bst->tree, node, bst_subtree->tree);
}

static inline bst_t bst_cut(bst_t handle, bst_node_t node) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_cut(bst->tree, node);
}

static inline bst_t bst_cut_left(bst_t handle, bst_node_t node) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_cut_left(bst->tree, node);
}

static inline bst_t bst_cut_right(bst_t handle, bst_node_t node) {
	struct bst* bst = (struct bst*)handle;
	return binary_tree_cut_right(bst->tree, node);
}

bst_t bst_cut_one_son_node(bst_t handle, bst_node_t node) {
	struct bst* bst = (struct bst*)handle;

	bst_t cutted_tree;
	bst_node_t son;
	if (!(son = bst_node_get_left_son(node))) {
		son = bst_node_get_right_son(node);
	}
	if (!son) {
		cutted_tree = bst_cut(bst, node);
	}
	else {
		bst_node_swap(node, son);
		cutted_tree = bst_tree_cut(bst, son);
		bst_insert_as_left_subtree(bst, node, bst_tree_cut(cutted_tree, bst_node_get_left_son(son)));
		bst_insert_as_right_subtree(bst, node, bst_tree_cut(cutted_tree, bst_node_get_right_son(son)));
		//memory leack
	}
	return cutted_tree;
}

void bst_insert_single_node_tree(bst_t handle, void* key, bst_t new_tree) {
	struct bst* bst = (struct bst*)handle;
	int cmp;

	if (bst_get_root(bst) == NULL) {
		bst_set_root(bst, bst_get_root(new_tree));
	}
	else {
		bst_node_t current = bst_get_root(bst);
		bst_node_t prev = NULL;
		while (current) {
			prev = current;
			cmp = bst->compare(key, bst_node_get_key(current));
			if (cmp < 1) {
				current = bst_node_get_left_son(current);
			}
			else {
				current = bst_node_get_right_son(current);
			}
		}
		cmp = bst->compare(key, bst_node_get_key(current));
		if (cmp < 1) {
			binary_tree_insert_as_left_subtree(bst->tree, prev, new_tree);
		}
		else {
			binary_tree_insert_as_right_subtree(bst->tree, prev, new_tree);
		}
	}
}

bst_node_t bst_search_node(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	bst_node_t current = bst_get_root(bst);
	int cmp;
	while (current) {
		cmp = bst->compare(key, bst_node_get_key(current));
		if (!cmp) {
			return current;
		}
		else if (cmp == -1) {
			current = bst_node_get_left_son(current);
		}
		else {
			current = bst_node_get_right_son(current);
		}
	}
	return NULL;
}

void* bst_search(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	return bst_node_get_value(bst_search_node(bst, key));
}

int bst_insert(bst_t handle, void* key, void* value) {
	struct bst* bst = (struct bst*)handle;
	bst_t tree = bst_init(bst_node_init(key, value), bst->compare);
	bst_insert_single_node_tree(bst, key, tree);
	return 0;
}

void bst_delete(bst_t handle, void* key) {
	struct bst* bst = (struct bst*)handle;
	bst_node_t node;
	if (node = bst_search_node(bst, key)) {
		if (bst_node_degree(node) < 2) {
			bst_cut_one_son_node(bst, node);
		}
		else {
			bst_node_t pred = bst_node_get_pred(node);
			bst_node_swap(node, pred);
			bst_cut_one_son_node(bst, pred);
		}
	}
}

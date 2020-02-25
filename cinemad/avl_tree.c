#include "avl_tree.h"

#include <stdlib.h>

#include "bst.h"

// search

int insert(avl_tree_t handle, void* key, void* value) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	struct avl_dictionary* info;
	if ((info = malloc(sizeof(struct avl_dictionary))) == NULL) {
		return 1;
	}
	info->key = key;
	info->value = value;
	info->height = 0;
	binary_tree_t tree = binary_tree_init(binary_node_init(info));
	bst_insert_single_node_tree(avl_tree->search_tree, key, tree);
	balance_insert(avl_tree, tree);
	return 0;
}

void delete(avl_tree_t handle, void* key) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	node_t node = avl_search(avl_tree, key);
	if (node) {
		if (node_degree(node) < 2) {
			cut_single_son(avl_tree, node);
		}
		else {
			node_t p = bst_pred(node);
			node_swap(node, p);
			double tmp_h = get_height(node);
			set_height(node, get_height(p));
			set_height(p, tmp_h);
			cut_single_son(avl_tree, p);
		}
	}
}

/*
------------------------------------------------------------------------
*/

avl_tree_t avl_tree_init(avl_tree_node_t root, avl_tree_comparison_function* comparison_function) {
	struct avl_tree* avl_tree;
	if ((avl_tree = bst_init(NULL, comparison_function)) == NULL) {
		return NULL;
	}
	return avl_tree;
}

int avl_tree_destroy(avl_tree_t handle) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;
	avl_tree_t avl_tree_left_subtree = avl_tree_cut_left(avl_tree, avl_tree_get_root(avl_tree));
	avl_tree_t avl_tree_right_subtree = avl_tree_cut_right(avl_tree, avl_tree_get_root(avl_tree));
	if (avl_tree_destroy(avl_tree_left_subtree)) {
		return 1;
	}
	if (avl_tree_destroy(avl_tree_right_subtree)) {
		return 1;
	}
	if (avl_tree_node_destroy(avl_tree_get_root(avl_tree))) {
		return 1;
	}
	free(avl_tree);
	return 0;
}

static inline long avl_tree_nodes_number(avl_tree_t handle) { return bst_nodes_number(handle); }

static inline avl_tree_node_t avl_tree_get_root(avl_tree_t handle) { return bst_get_root(handle); }

static inline int avl_tree_set_root(avl_tree_t handle, avl_tree_node_t node) { return bst_set_root(handle, node); }

static inline int avl_tree_insert_as_left_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree) { return bst_insert_as_left_subtree(handle, node, subtree); }

static inline int avl_tree_insert_as_right_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree) { return bst_insert_as_right_subtree(handle, node, subtree); }

static inline avl_tree_t avl_tree_cut(avl_tree_t handle, avl_tree_node_t node) { return bst_cut(handle, node); }

static inline avl_tree_t avl_tree_cut_left(avl_tree_t handle, avl_tree_node_t node) {	return bst_cut_left(handle, node); }

static inline avl_tree_t avl_tree_cut_right(avl_tree_t handle, avl_tree_node_t node) { return bst_cut_right(handle, node); }

static inline avl_tree_t avl_tree_cut_one_son_node(avl_tree_t handle, avl_tree_node_t node) {	return bst_cut_one_son_node(handle, node); }

static inline void avl_tree_insert_single_node_tree(avl_tree_t handle, void* key, avl_tree_t new_tree) { return bst_insert_single_node_tree(handle, key, new_tree); }

static inline avl_tree_node_t avl_tree_search_node(avl_tree_t handle, void* key) { return bst_search_node(handle, key); }

static inline void* avl_tree_search(avl_tree_t handle, void* key) {	return bst_search(handle, key); }

static inline int avl_tree_insert(avl_tree_t handle, void* key, void* value) { return bst_insert(handle, key, value); }

static inline void avl_tree_delete(avl_tree_t handle, void* key) { return bst_delete(handle, key); }


/*  METODI DI ROTAZIONE E BILANCIAMENTO */

void right_rotation(avl_tree_t handle, node_t u) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;
	node_t v = binary_node_get_left_son(u);
	node_swap(u, v);

	tree_t v_tree = bst_cut_left(avl_tree->search_tree, u);
	tree_t t1 = bst_cut_left(v_tree, v);
	tree_t t2 = bst_cut_right(v_tree, v);
	tree_t t3 = bst_cut_right(avl_tree->search_tree, u);

	bst_insert_as_right_subtree(v_tree, tree_get_root(v_tree), t3);
	bst_insert_as_left_subtree(v_tree, tree_get_root(v_tree), t2);
	bst_insert_as_right_subtree(avl_tree->search_tree, u, v_tree);
	bst_insert_as_right_subtree(avl_tree->search_tree, u, t1);

	update_height(binary_node_get_right_son(u));
	update_height(u);
}

void left_rotation(avl_tree_t handle, node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	node_t right_son = binary_node_get_right_son(node);
	node_swap(node, right_son);

	tree_t r_tree = bst_cut_right(avl_tree->search_tree, node);
	tree_t l_tree = bst_cut_left(avl_tree->search_tree, node);
	tree_t r_tree_l = bst_cut_left(r_tree, right_son);
	tree_t r_tree_r = bst_cut_right(r_tree, right_son);

	bst_insert_as_left_subtree(r_tree, tree_get_root(r_tree), l_tree);
	bst_insert_as_right_subtree(r_tree, tree_get_root(r_tree), r_tree_l);
	bst_insert_as_right_subtree(avl_tree->search_tree, node, r_tree);
	bst_insert_as_right_subtree(avl_tree->search_tree, node, r_tree_r);

	update_height(binary_node_get_left_son(node));
	update_height(node);
}

void rotate(avl_tree_t handle, node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	long balanced_factor = get_balance_factor(node);
	if (balanced_factor == 2) {		//height of the node's left son is 2 times greater than the right one
		if (get_balance_factor(binary_node_get_left_son(node)) >= 0) {	//LL balancing
			right_rotation(avl_tree, node);
		}
		else {	//LR balancing
			left_rotation(avl_tree, binary_node_get_left_son(node));
			right_rotation(avl_tree, node);
		}
	}
	else if (balanced_factor == -2) {	//height of the node's right son is 2 times greater than the left one
		if (get_balance_factor(binary_node_get_left_son(node)) >= 0) {	//RR balancing
			left_rotation(avl_tree, node);
		}
		else {	//RL balancing
			right_rotation(avl_tree, binary_node_get_right_son(node));
			left_rotation(avl_tree, node);
		}
	}
}

void balance_insert(avl_tree_t handle, node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	node_t current = node_get_father(node);
	while (current) {
		if (abs(get_balance_factor(current)) >= 2) {
			break;
		}
		else {
			update_height(current);
			current = node_get_father(current);
		}
	}
	if (current) {
		rotate(avl_tree, current);
	}
}

void balance_delete(avl_tree_t handle, node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	node_t current = node_get_father(node);
	while (current) {
		if (abs(get_balance_factor(current)) == 2) {
			rotate(avl_tree, current);
		}
		else {
			update_height(current);
		}
		current = node_get_father(current);
	}
}

void cut_single_son(avl_tree_t handle, node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	bst_cut_one_son_node(avl_tree->search_tree, node);
	balance_delete(avl_tree->search_tree, node);
}

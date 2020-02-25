#include "avl_tree.h"

#include <stdlib.h>

#include "bst.h"

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

static inline int avl_tree_insert(avl_tree_t handle, void* key, void* value) {
	avl_tree_node_t node = avl_tree_node_init(key, value);
	avl_tree_t tree = avl_tree_init(node, NULL);
	avl_tree_insert_single_node_tree(handle, key, tree);
	balance_insert(handle, node);
}

static inline void avl_tree_delete(avl_tree_t handle, void* key) {
	avl_tree_node_t node = avl_tree_search(handle, key);
	if (node) {
		if (avl_tree_node_degree(node) < 2) {
			cut_single_son(handle, node);
		}
		else {
			avl_tree_node_t p = avl_tree_node_get_pred(node);
			avl_tree_node_swap(node, p);
			double tmp_h = avl_tree_node_get_height(node);
			avl_tree_node_set_height(node, avl_tree_node_get_height(p));
			avl_tree_node_set_height(p, tmp_h);
			cut_single_son(handle, p);
		}
	}
}

/*  METODI DI ROTAZIONE E BILANCIAMENTO */

//	un sacco di memory leack

void right_rotation(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;
	avl_tree_node_t left_son = avl_tree_node_get_left_son(node);
	avl_tree_node_swap(node, left_son);

	avl_tree_t l_tree = avl_tree_cut_left(avl_tree, node);
	avl_tree_t r_tree = avl_tree_cut_right(avl_tree, node);
	avl_tree_t l_tree_l = avl_tree_cut_left(l_tree, left_son);
	avl_tree_t l_tree_r = avl_tree_cut_right(l_tree, left_son);

	avl_tree_insert_as_right_subtree(l_tree, avl_tree_get_root(l_tree), r_tree);
	avl_tree_insert_as_left_subtree(l_tree, avl_tree_get_root(l_tree), l_tree_r);
	avl_tree_insert_as_right_subtree(avl_tree, node, l_tree);
	avl_tree_insert_as_left_subtree(avl_tree, node, l_tree_l);

	avl_tree_node_update_height(avl_tree_node_get_right_son(node));
	avl_tree_node_update_height(node);
}

void left_rotation(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	avl_tree_node_t right_son = avl_tree_node_get_right_son(node);
	avl_tree_node_swap(node, right_son);

	avl_tree_t r_tree = avl_tree_cut_right(avl_tree, node);
	avl_tree_t l_tree = avl_tree_cut_left(avl_tree, node);
	avl_tree_t r_tree_l = avl_tree_cut_left(r_tree, right_son);
	avl_tree_t r_tree_r = avl_tree_cut_right(r_tree, right_son);

	avl_tree_insert_as_left_subtree(r_tree, avl_tree_get_root(r_tree), l_tree);
	avl_tree_insert_as_right_subtree(r_tree, avl_tree_get_root(r_tree), r_tree_l);
	avl_tree_insert_as_left_subtree(avl_tree, node, r_tree);
	avl_tree_insert_as_right_subtree(avl_tree, node, r_tree_r);

	avl_tree_node_update_height(avl_tree_node_get_left_son(node));
	avl_tree_node_update_height(node);
}

void rotate(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	long balanced_factor = avl_tree_node_get_balance_factor(node);
	if (balanced_factor == 2) {		//height of the node's left son is 2 times greater than the right one
		if (avl_tree_node_get_balance_factor(avl_tree_node_get_left_son(node)) >= 0) {	//LL balancing
			right_rotation(avl_tree, node);
		}
		else {	//LR balancing
			left_rotation(avl_tree, avl_tree_node_get_left_son(node));
			right_rotation(avl_tree, node);
		}
	}
	else if (balanced_factor == -2) {	//height of the node's right son is 2 times greater than the left one
		if (avl_tree_node_get_balance_factor(avl_tree_node_get_left_son(node)) >= 0) {	//RR balancing
			left_rotation(avl_tree, node);
		}
		else {	//RL balancing
			right_rotation(avl_tree, avl_tree_node_get_right_son(node));
			left_rotation(avl_tree, node);
		}
	}
}

void balance_insert(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	avl_tree_node_t current = avl_tree_node_get_father(node);
	while (current) {
		if (abs(avl_tree_node_get_balance_factor(current)) >= 2) {
			break;
		}
		else {
			avl_tree_node_update_height(current);
			current = avl_tree_node_get_father(current);
		}
	}
	if (current) {
		rotate(avl_tree, current);
	}
}

void balance_delete(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	avl_tree_node_t current = avl_tree_node_get_father(node);
	while (current) {
		if (abs(avl_tree_node_get_balance_factor(current)) == 2) {
			rotate(avl_tree, current);
		}
		else {
			avl_tree_node_update_height(current);
		}
		current = avl_tree_node_get_father(current);
	}
}

void cut_single_son(avl_tree_t handle, avl_tree_node_t node) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;

	avl_tree_cut_one_son_node(avl_tree, node);
	balance_delete(avl_tree, node);
}

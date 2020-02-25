#include "avl_tree.h"

#include <stdlib.h>

#include "avl_tree_node.h"
#include "bst.h"

//Node's info now is a triple [ key, value, height]

avl_tree_t avl_tree_init(int (*order_function)(void*, void*)) {
	struct avl_tree* avl_tree;
	if ((avl_tree = malloc(sizeof(struct avl_tree))) == NULL) {
		return NULL;
	}
	if ((avl_tree->search_tree = bst_init(order_function)) == NULL) {
		free(avl_tree);
	}
	return avl_tree;
}

int avl_tree_destroy(avl_tree_t handle) {
	struct avl_tree* avl_tree = (struct avl_tree*)handle;
	if (bst_destroy(avl_tree->search_tree)) {
		return 1;
	}
	free(avl_tree);
	return 0;
}

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

}

int avl_tree_destroy(avl_tree_t handle) {

}

long avl_tree_nodes_number(avl_tree_t handle) {

}

avl_tree_node_t avl_tree_get_root(avl_tree_t handle) {

}

int avl_tree_set_root(avl_tree_t handle, avl_tree_node_t node) {

}

int avl_tree_insert_as_left_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree) {

}

int avl_tree_insert_as_right_subtree(avl_tree_t handle, avl_tree_node_t node, avl_tree_t subtree) {

}

avl_tree_t avl_tree_cut(avl_tree_t handle, avl_tree_node_t node) {

}

avl_tree_t avl_tree_cut_left(avl_tree_t handle, avl_tree_node_t node) {

}

avl_tree_t avl_tree_cut_right(avl_tree_t handle, avl_tree_node_t node) {

}

avl_tree_t avl_tree_cut_one_son_node(avl_tree_t handle, avl_tree_node_t node) {

}

void avl_tree_insert_single_node_tree(avl_tree_t handle, void* key, avl_tree_t new_tree) {

}

avl_tree_node_t avl_tree_search_node(avl_tree_t handle, void* key) {

}

void* avl_tree_search(avl_tree_t handle, void* key) {

}

int avl_tree_insert(avl_tree_t handle, void* key, void* value) {

}

void avl_tree_delete(avl_tree_t handle, void* key) {

}

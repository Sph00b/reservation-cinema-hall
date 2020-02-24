#include "binary_tree.h"

#include <stdlib.h>

#include "binary_node.h"
#include "stack.h"

struct binary_tree {
	binary_node_t root;
	long n;
};

binary_tree_t binary_tree_init(node_t root) {
	struct binary_tree* binary_tree;
	if ((binary_tree = malloc(sizeof(struct binary_tree))) == NULL) {
		return NULL;
	}
	binary_tree->root = root;
	binary_tree->n = subtree_nodes_number(binary_tree, root);
	return binary_tree;
}

int binary_tree_destroy(binary_tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	if (binary_node_destroy(binary_tree->root)) {
		return 1;
	}
	free(binary_tree);
	return 0;
}

void binary_tree_insert_as_left_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree) {
	struct binary_tree* tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root) {
		node_set_father(binary_subtree->root, binary_node);
	}
	binary_node_set_left_son(binary_node, binary_subtree->root);
	increase_nodes_number_by_subtree(tree, binary_subtree->root);
}

void binary_tree_insert_as_right_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree) {
	struct binary_tree* tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root) {
		node_set_father(binary_subtree->root, binary_node);
	}
	binary_node_set_right_son(binary_node, binary_subtree->root);
	increase_nodes_number_by_subtree(tree, binary_subtree->root);
}

binary_tree_t binary_tree_cut_left(binary_tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* new_tree;
	binary_node_t left_son = binary_node_get_left_son(binary_node);
	if ((new_tree = binary_tree_init(left_son)) == NULL) {
		return NULL;
	}
	decrease_nodes_number_by_subtree(binary_tree, left_son);
	binary_node_set_left_son(binary_node, NULL);
	return new_tree;
}

binary_tree_t binary_tree_cut_right(binary_tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* new_tree;
	binary_node_t right_son = binary_node_get_right_son(binary_node);
	if ((new_tree = binary_tree_init(right_son)) == NULL) {
		return NULL;
	}
	decrease_nodes_number_by_subtree(binary_tree, right_son);
	binary_node_set_right_son(binary_node, NULL);
	return new_tree;
}

/*
	METODI EREDITATI DALLA CLASSE TREE.
*/

node_t tree_get_root(tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	return binary_tree->root;
}

long tree_nodes_number(tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	return binary_tree->n;
}

tree_t tree_cut(tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	node_t father = node_get_father(node);

	if (node) {
		if (node == tree_get_root(binary_tree)) {
			binary_tree->root = NULL;
			binary_tree->n = 0;
		}
		else if (node == binary_node_get_left_son(father)) {
			if (!node_degree(node)) {	//if node is a leaf
				binary_node_set_left_son(father, NULL);
				binary_tree->n--;
			}
			else {
				return binary_tree_cut_left(binary_tree, node);
			}
		}
		else if (node == binary_node_get_right_son(father)) {
			if (!node_degree(node)) {	//if node is a leaf
				binary_node_set_right_son(father, NULL);
				binary_tree->n--;
			}
			else {
				return binary_tree_cut_right(binary_tree, node);
			}
		}
	}
	return binary_tree_init(node);
}

int subtree_nodes_number(tree_t handle, node_t node) {
	struct binary_node* binary_node = (struct binary_node*)node;
	int nodes_number = 0;
	_stack_t stack = stack_init();
	if (binary_node) {
		stack_push(stack, binary_node);
		while (!stack_is_empty(stack)) {
			struct binary_node* current_node;
			current_node = stack_pop(stack);
			nodes_number++;
			if (binary_node_get_left_son(current_node)) {
				stack_push(stack, binary_node_get_left_son(current_node));
			}
			if (binary_node_get_right_son(current_node)) {
				stack_push(stack, binary_node_get_right_son(current_node));
			}
		}
	}
	return nodes_number;
}

void increase_nodes_number_by_subtree(tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	binary_tree->n += subtree_nodes_number(binary_tree, node);
}

void decrease_nodes_number_by_subtree(tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	binary_tree->n -= subtree_nodes_number(binary_tree, node);
}

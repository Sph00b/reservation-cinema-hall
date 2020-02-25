#include "binary_tree.h"

#include <stdlib.h>

#include "stack.h"

struct binary_tree {
	binary_tree_node_t root;
	long n;
};

binary_tree_t binary_tree_init(binary_tree_node_t root) {
	struct binary_tree* binary_tree;
	if ((binary_tree = malloc(sizeof(struct binary_tree))) == NULL) {
		return NULL;
	}
	binary_tree->root = root;
	binary_tree->n = subtree_nodes_number(root);
	return binary_tree;
}

int binary_tree_destroy(binary_tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_tree* binary_left_subtree = binary_tree_cut_left(binary_tree, binary_tree->root);
	struct binary_tree* binary_right_subtree = binary_tree_cut_right(binary_tree, binary_tree->root);
	if (binary_left_subtree->root) {
		if (binary_tree_destroy(binary_left_subtree)) {
			return 1;
		}
	}
	if (binary_right_subtree->root) {
		if (binary_tree_destroy(binary_right_subtree)) {
			return 1;
		}
	}
	if (binary_tree_node_destroy(binary_tree->root)) {
		return 1;
	}
	free(binary_tree);
	return 0;
}

long binary_tree_nodes_number(binary_tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;

	return binary_tree->n;
}

binary_tree_node_t binary_tree_get_root(binary_tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;

	return binary_tree->root;
}

int binary_tree_set_root(binary_tree_t handle, binary_tree_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;

	binary_tree->root = node;
	binary_tree->n = subtree_nodes_number(node);
}

int binary_tree_insert_as_left_subtree(binary_tree_t handle, binary_tree_node_t node, binary_tree_t subtree) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root) {
		binary_tree_node_set_father(binary_subtree->root, node);
	}
	binary_tree_node_set_left_son(node, binary_subtree->root);
	binary_tree->n += subtree_nodes_number(node);
}

int binary_tree_insert_as_right_subtree(binary_tree_t handle, binary_tree_node_t node, binary_tree_t subtree) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root) {
		binary_tree_node_set_father(binary_subtree->root, node);
	}
	binary_tree_node_set_right_son(node, binary_subtree->root);
	binary_tree->n += subtree_nodes_number(node);
}

binary_tree_t binary_tree_cut(binary_tree_t handle, binary_tree_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;

	if (node) {
		binary_tree_node_t father = binary_tree_node_get_father(node);

		if (node == binary_tree_get_root(binary_tree)) {
			binary_tree->root = NULL;
			binary_tree->n = 0;
		}
		else if (node == binary_tree_node_get_left_son(father)) {
			if (binary_tree_node_degree(node) == 0) {
				binary_tree_node_set_left_son(father, NULL);
				binary_tree->n--;
			}
			else {
				return binary_tree_cut_left(binary_tree, node);
			}
		}
		else if (node == binary_tree_node_get_right_son(father)) {
			if (binary_tree_node_degree(node) == 0) {
				binary_tree_node_set_right_son(father, NULL);
				binary_tree->n--;
			}
			else {
				return binary_tree_cut_right(binary_tree, node);
			}
		}
	}
	return binary_tree_init(node);
}

binary_tree_t binary_tree_cut_left(binary_tree_t handle, binary_tree_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_tree* new_tree;
	binary_tree_t left_son = binary_tree_node_get_left_son(node);
	if ((new_tree = binary_tree_init(left_son)) == NULL) {
		return NULL;
	}
	binary_tree->n -= subtree_nodes_number(left_son);
	binary_tree_node_set_left_son(node, NULL);
	return new_tree;
}

binary_tree_t binary_tree_cut_right(binary_tree_t handle, binary_tree_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_tree* new_tree;
	binary_tree_node_t right_son = binary_tree_node_get_right_son(node);
	if ((new_tree = binary_tree_init(right_son)) == NULL) {
		return NULL;
	}
	binary_tree->n -= subtree_nodes_number(right_son);
	binary_tree_node_set_right_son(node, NULL);
	return new_tree;
}

int subtree_nodes_number(binary_tree_node_t node) {
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

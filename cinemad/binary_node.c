#include "binary_tree_node.h"

#include <stdlib.h>

struct binary_tree_node {
	void* data;
	struct binary_tree_node* father;
	struct binary_tree_node* left_son;
	struct binary_tree_node* right_son;
};

binary_tree_node_t binary_tree_node_init(void* info) {
	struct binary_tree_node* binary_tree_node;
	if ((binary_tree_node = malloc(sizeof(struct binary_tree_node))) == NULL) {
		return NULL;
	}
	binary_tree_node->data = info;
	binary_tree_node->father = NULL;
	binary_tree_node->left_son = NULL;
	binary_tree_node->right_son = NULL;
	return binary_tree_node;
}

int binary_tree_node_destroy(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;
	free(binary_tree_node);
	return 0;
}

long binary_tree_node_degree(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;
	long degree = 0;

	if (binary_tree_node) {
		if (binary_tree_node->left_son) {
			degree++;
		}
		if (binary_tree_node->right_son) {
			degree++;
		}
	}
	return degree;
}

int binary_tree_node_swap(binary_tree_node_t handle1, binary_tree_node_t handle2) {
	struct binary_tree_node* binary_tree_node1 = (struct binary_tree_node*)handle1;
	struct binary_tree_node* binary_tree_node2 = (struct binary_tree_node*)handle2;
	if (binary_tree_node1 && binary_tree_node2) {
		void* tmp = binary_tree_node1->data;
		binary_tree_node1->data = binary_tree_node2->data;
		binary_tree_node2->data = tmp;
		return 0;
	}
	return 1;
}

void* binary_tree_node_get_info(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;
	if (binary_tree_node) {
		return binary_tree_node->data;
	}
}

binary_tree_node_t binary_tree_node_get_father(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;
	if (binary_tree_node) {
		return binary_tree_node->father;
	}
	return NULL;
}

int binary_tree_node_set_father(binary_tree_node_t handle, binary_tree_node_t father) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;
	if (binary_tree_node) {
		binary_tree_node->father = father;
		return 0;
	}
	return 1;
}

binary_tree_node_t binary_tree_node_get_left_son(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		return binary_tree_node->left_son;
	}
	return NULL;
}

int binary_tree_node_set_left_son(binary_tree_node_t handle, binary_tree_node_t left_son) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		binary_tree_node->left_son = left_son;
		return 0;
	}
	return 1;
}

binary_tree_node_t binary_tree_node_get_right_son(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		return binary_tree_node->right_son;
	}
	return NULL;
}

int binary_tree_node_set_right_son(binary_tree_node_t handle, binary_tree_node_t right_son) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		binary_tree_node->right_son = right_son;
		return 0;
	}
	return 1;
}

int binary_tree_node_is_left_son(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		struct binary_tree_node* father = binary_tree_node->father;
		if (father) {
			if (father->left_son == binary_tree_node) {
				return 1;
			}
		}
	}
	return 0;
}

int binary_tree_node_is_right_son(binary_tree_node_t handle) {
	struct binary_tree_node* binary_tree_node = (struct binary_tree_node*)handle;

	if (binary_tree_node) {
		struct binary_tree_node* father = binary_tree_node->father;
		if (father) {
			if (father->right_son == binary_tree_node) {
				return 1;
			}
		}
	}
	return 0;
}

#include "binary_node.h"

#include <stdlib.h>

struct binary_node {
	void* data;
	binary_node_t father;
	binary_node_t left_son;
	binary_node_t right_son;
};

binary_node_t binary_node_init(void* item) {
	struct binary_node* binary_node;
	if ((binary_node = malloc(sizeof(struct binary_node))) == NULL) {
		return NULL;
	}
	binary_node->data = item;
	binary_node->father = NULL;
	binary_node->left_son = NULL;
	binary_node->right_son = NULL;
	return binary_node;
}

int binary_node_destroy(binary_node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	if (binary_node) {
		binary_node_destroy(binary_node->left_son);
		binary_node_destroy(binary_node->right_son);
		//free(binary_node->data);
		free(binary_node);
	}
	return 0;
}

binary_node_t binary_node_get_left_son(binary_node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	return binary_node->left_son;
}

binary_node_t binary_node_get_right_son(binary_node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	return binary_node->right_son;
}

void binary_node_set_left_son(binary_node_t handle, binary_node_t left_son) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	binary_node->left_son = left_son;
}

void binary_node_set_right_son(binary_node_t handle, binary_node_t right_son) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	binary_node->right_son = right_son;
}

/*	funzioni ereditate da node	*/

long node_degree(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	return list_lenght(node_get_sons(binary_node));
}

void* node_get_info(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	return binary_node->data;
}

node_t node_get_father(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	if (binary_node == NULL) {
		return NULL;
	}
	return binary_node->father;
}

void node_set_father(node_t handle, node_t father) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	binary_node->father = father;
}

list_t node_get_sons(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	list_t sons;
	if ((sons = list_init()) == NULL) {
		return NULL;
	}
	if (binary_node != NULL) {
		if (binary_node->left_son != NULL) {
			list_append(sons, binary_node->left_son);
		}
		if (binary_node->right_son != NULL) {
			list_append(sons, binary_node->right_son);
		}
	}
	return sons;
}
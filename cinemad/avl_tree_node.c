#include "avl_tree_node.h"

#include <stdlib.h>

#include "bst_node.h"

struct avl_tree_node {
	bst_node_t node;
	int height;
};

static inline void* max(void* a, void* b) {
	return a > b ? a : b;
}

avl_tree_node_t avl_tree_node_init(void* key, void* value) {
	struct avl_tree_node* avl_tree_node;
	if ((avl_tree_node = malloc(sizeof(struct avl_tree_node))) == NULL) {
		return NULL;
	}
	avl_tree_node->height = 0;
	if ((avl_tree_node->node = bst_node_init(key, value)) == NULL) {
		free(avl_tree_node);
		return NULL;
	}
	return avl_tree_node;
}

int avl_tree_node_destroy(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	if (bst_node_destroy(avl_tree_node->node)) {
		return 1;
	}
	free(avl_tree_node);
	return 0;
}

static inline long avl_tree_node_degree(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_degree(avl_tree_node->node);
}

static inline int avl_tree_node_swap(avl_tree_node_t handle1, avl_tree_node_t handle2) {
	struct avl_tree_node* avl_tree_node1 = (struct avl_tree_node*)handle1;
	struct avl_tree_node* avl_tree_node2 = (struct avl_tree_node*)handle2;
	return bst_node_swap(avl_tree_node1->node, avl_tree_node2->node);
}

static inline void* avl_tree_node_get_info(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_info(avl_tree_node->node);
}

static inline void* avl_tree_node_get_key(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_key(avl_tree_node->node);
}

static inline void* avl_tree_node_get_value(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_value(avl_tree_node->node);
}

int avl_tree_node_get_height(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return avl_tree_node->height;
}

int avl_tree_node_get_height(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	if (avl_tree_node) {
		return avl_tree_node->height;
	}
	return -1;
}

int avl_tree_node_set_height(avl_tree_node_t handle, int height) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	if (avl_tree_node) {
		avl_tree_node->height = height;
		return 0;
	}
	return 1;
}

void avl_tree_node_update_height(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	if (avl_tree_node->node) {
		long left_son_height = avl_tree_node_get_height(avl_tree_node_get_left_son(avl_tree_node));
		long right_son_height = avl_tree_node_get_height(avl_tree_node_get_right_son(avl_tree_node));
		avl_tree_node_set_height(avl_tree_node, max(left_son_height, right_son_height) + 1);
	}
}

long avl_tree_node_get_balance_factor(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	if (avl_tree_node->node) {
		long left_son_height = avl_tree_node_get_height(avl_tree_node_get_left_son(avl_tree_node));
		long right_son_height = avl_tree_node_get_height(avl_tree_node_get_right_son(avl_tree_node));
		return  left_son_height - right_son_height;
	}
	return 0;
}

/*	a lot of 'em doesn't work 	*/

static inline avl_tree_node_t avl_tree_node_get_father(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_father(avl_tree_node->node);
}

static inline int avl_tree_node_set_father(avl_tree_node_t handle, avl_tree_node_t father) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	struct avl_tree_node* avl_tree_node_father = (struct avl_tree_node*)father;
	return bst_node_set_father(avl_tree_node->node, avl_tree_node_father->node);
}

static inline avl_tree_node_t avl_tree_node_get_left_son(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_left_son(avl_tree_node->node);
}

static inline int avl_tree_node_set_left_son(avl_tree_node_t handle, avl_tree_node_t left_son) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	struct avl_tree_node* avl_tree_node_son = (struct avl_tree_node*)left_son;
	return bst_node_set_left_son(avl_tree_node->node, avl_tree_node_son->node);
}

static inline avl_tree_node_t avl_tree_node_get_right_son(avl_tree_node_t handle){
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_right_son(avl_tree_node->node);
}

static inline int avl_tree_node_set_right_son(avl_tree_node_t handle, avl_tree_node_t right_son) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	struct avl_tree_node* avl_tree_node_son = (struct avl_tree_node*)right_son;
	return bst_node_set_right_son(avl_tree_node->node, avl_tree_node_son->node);
}

static inline int avl_tree_node_is_left_son(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_is_left_son(avl_tree_node->node);
}

static inline int avl_tree_node_is_right_son(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_is_right_son(avl_tree_node->node);
}

static inline avl_tree_node_t avl_tree_node_get_max(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_max(avl_tree_node->node);
}

static inline avl_tree_node_t avl_tree_node_get_pred(avl_tree_node_t handle) {
	struct avl_tree_node* avl_tree_node = (struct avl_tree_node*)handle;
	return bst_node_get_pred(avl_tree_node->node);
}

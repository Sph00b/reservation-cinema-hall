#include "bst_node.h"

#include <stdlib.h>

#include "binary_tree_node.h"

struct dictionary {
	void* key;
	void* value;
};

bst_node_t bst_node_init(void* key, void* value) {
	bst_node_t bst_node;
	struct dictionary* info;
	if ((info = malloc(sizeof(struct dictionary))) == NULL) {
		return NULL;
	}
	info->key = key;
	info->value = value;
	if ((bst_node = binary_tree_node_init(info)) == NULL) {
		free(info);
		return NULL;
	}
	return bst_node;
}

int bst_node_destroy(bst_node_t handle) {
	if (handle) {
		free(bst_node_get_info(handle));
	}
	return binary_tree_node_destroy(handle);
}

static inline long bst_node_degree(bst_node_t handle) { return binary_tree_node_degree(handle); }

static inline int bst_node_swap(bst_node_t handle1, bst_node_t handle2) { return binary_tree_node_swap(handle1, handle2); }

static inline void* bst_node_get_info(bst_node_t handle) { return binary_tree_node_get_info(handle); }

void* bst_node_get_key(bst_node_t handle){
	void* info = bst_node_get_info(handle);
	return ((struct dictionary*)info)->key;
}

void* bst_node_get_value(bst_node_t handle) {
	void* info = bst_node_get_info(handle);
	return ((struct dictionary*)info)->value;
}

static inline bst_node_t bst_node_get_father(bst_node_t handle) { return binary_tree_node_get_father(handle); }

static inline int bst_node_set_father(bst_node_t handle, bst_node_t father) { return binary_tree_node_set_father(handle, father); }

static inline bst_node_t bst_node_get_left_son(bst_node_t handle) { return binary_tree_node_get_left_son(handle); }

static inline int bst_node_set_left_son(bst_node_t handle, bst_node_t left_son) { return binary_tree_node_set_left_son(handle, left_son); }

static inline bst_node_t bst_node_get_right_son(bst_node_t handle) { return binary_tree_node_get_right_son(handle); }

static inline int bst_node_set_right_son(bst_node_t handle, bst_node_t right_son) {	return binary_tree_node_set_right_son(handle, right_son); }

static inline int bst_node_is_left_son(bst_node_t handle) { return binary_tree_node_is_left_son(handle); }

static inline int bst_node_is_right_son(bst_node_t handle) { return binary_tree_node_is_right_son(handle); }

bst_node_t bst_node_get_max(bst_node_t handle) {
	bst_node_t current = handle;
	while (binary_tree_node_get_right_son(current)) {
		current = binary_tree_node_get_right_son(current);
	}
	return current;
}

bst_node_t bst_node_get_pred(bst_node_t handle) {
	if (handle == NULL) {
		return NULL;
	}
	if (binary_tree_node_get_left_son(handle)) {
		return bst_node_get_max(binary_tree_node_get_left_son(handle));
	}
	bst_node_t current = handle;
	while (binary_tree_node_is_left_son(current)) {
		current = binary_tree_node_get_father(current);
	}
	return current;
}

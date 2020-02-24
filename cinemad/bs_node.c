#include "bs_node.h"

#include <stdlib.h>

#include "binary_node.h"

struct bs_node {
	binary_node_t node;
};

struct info {
	void* key;
	void* value;
};

bs_node_t bs_node_init(void* key, void* value) {
	struct bs_node* bs_node;
	if ((bs_node = malloc(sizeof(struct bs_node))) == NULL) {
		return NULL;
	}
	struct info* info;
	if ((info = malloc(sizeof(struct info))) == NULL) {
		free(bs_node);
		return NULL;
	}
	if ((bs_node->node = binary_node_init(info)) == NULL) {
		free(info);
		free(bs_node);
		return NULL;
	}
	return bs_node;
}

int bs_node_destroy(bs_node_t handle) {
	struct bs_node* bs_node = (struct bs_node*)handle;
	if (bs_node) {
		binary_node_destroy(bs_node->node);
		free(bs_node);
	}
	return 0;
}

void* bs_node_get_key(bs_node_t handle) {
	struct bs_node* bs_node = (struct bs_node*)handle;
	if (bs_node) {
		return ((struct info*)node_get_info(bs_node->node))->key;
	}
	return NULL;
}

void* bs_node_get_value(bs_node_t handle) {
	struct bs_node* bs_node = (struct bs_node*)handle;
	if (bs_node) {
		return ((struct info*)node_get_info(bs_node->node))->value;
	}
	return NULL;
}

int bs_node_is_left_son(bs_node_t handle) {
	struct bs_node* bs_node = (struct bs_node*)handle;
	binary_node_t father = node_get_father(bs_node->node);
	if (bs_node->node == binary_node_get_left_son(father)) {
		return 1;
	}
	return 0;
}

#include <avl_tree_node.h>

#include <stdlib.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

struct avl_tree_node {
	int height;
	void* key;
	void* value;
	avl_tree_node_t father;
	avl_tree_node_t left_son;
	avl_tree_node_t right_son;
};

avl_tree_node_t avl_tree_node_init(const void* key, const void* value) {
	struct avl_tree_node* node;
	if ((node = malloc(sizeof(struct avl_tree_node))) == NULL) {
		return NULL;
	}
	node->height = 0;
	node->key = (void*)key;
	node->value = (void*)value;
	node->father = NULL;
	node->left_son = NULL;
	node->right_son = NULL;
	return node;
}

int avl_tree_node_destroy(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	free(node);
	return 0;
}

inline long avl_tree_node_degree(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	long degree = 0;

	if (node) {
		if (node->left_son) {
			degree++;
		}
		if (node->right_son) {
			degree++;
		}
	}
	return degree;
}

inline int avl_tree_node_swap(const avl_tree_node_t handle1, const avl_tree_node_t handle2) {
	struct avl_tree_node* node1 = (struct avl_tree_node*)handle1;
	struct avl_tree_node* node2 = (struct avl_tree_node*)handle2;
	void* tmp;
	tmp = node1->key;
	node1->key = node2->key;
	node2->key = tmp;
	tmp = node1->value;
	node1->value = node2->value;
	node2->value = tmp;
	return 0;
}

inline void* avl_tree_node_get_key(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->key : NULL;
}

inline void* avl_tree_node_get_value(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->value : NULL;
}

inline int avl_tree_node_get_height(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->height : -1;
}

int avl_tree_node_set_height(const avl_tree_node_t handle, int height) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		node->height = height;
		return 0;
	}
	return 1;
}

void avl_tree_node_update_height(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		int left_son_height = avl_tree_node_get_height(avl_tree_node_get_left_son(node));
		int right_son_height = avl_tree_node_get_height(avl_tree_node_get_right_son(node));
		avl_tree_node_set_height(node, max(left_son_height, right_son_height) + 1);
	}
}

int avl_tree_node_get_balance_factor(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		int left_son_height = avl_tree_node_get_height(avl_tree_node_get_left_son(node));
		int right_son_height = avl_tree_node_get_height(avl_tree_node_get_right_son(node));
		return  left_son_height - right_son_height;
	}
	return 0;
}

inline avl_tree_node_t avl_tree_node_get_father(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->father : NULL;
}

int avl_tree_node_set_father(const avl_tree_node_t handle, const avl_tree_node_t father) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		node->father = father;
		return 0;
	}
	return 1;
}

inline avl_tree_node_t avl_tree_node_get_left_son(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->left_son : NULL;
}

int avl_tree_node_set_left_son(const avl_tree_node_t handle, const avl_tree_node_t left_son) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		node->left_son = left_son;
		return 0;
	}
	return 1;
}

inline avl_tree_node_t avl_tree_node_get_right_son(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	return node ? node->right_son : NULL;
}

int avl_tree_node_set_right_son(const avl_tree_node_t handle, const avl_tree_node_t right_son) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		node->right_son = right_son;
		return 0;
	}
	return 1;
}

int avl_tree_node_is_left_son(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		struct avl_tree_node* father = node->father;
		if (father) {
			if (father->left_son == node) {
				return 1;
			}
		}
	}
	return 0;
}

int avl_tree_node_is_right_son(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node) {
		struct avl_tree_node* father = node->father;
		if (father) {
			if (father->right_son == node) {
				return 1;
			}
		}
	}
	return 0;
}

inline avl_tree_node_t avl_tree_node_get_max(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	while (avl_tree_node_get_right_son(node)) {
		node = avl_tree_node_get_right_son(node);
	}
	return node;
}

inline avl_tree_node_t avl_tree_node_get_pred(const avl_tree_node_t handle) {
	struct avl_tree_node* node = (struct avl_tree_node*)handle;
	if (node == NULL) {
		return NULL;
	}
	if (avl_tree_node_get_left_son(node)) {
		return avl_tree_node_get_max(avl_tree_node_get_left_son(node));
	}
	while (avl_tree_node_is_left_son(node)) {
		node = avl_tree_node_get_father(node);
	}
	return node;
}

#include <avl_tree.h>

#include <stdlib.h>

#include <stack.h>

struct avl_tree {
	avl_tree_node_t root;
	int n;
	avl_tree_comparison_function* compare;
};

static void right_rotation(const avl_tree_t handle, const avl_tree_node_t node);
static void left_rotation(const avl_tree_t handle, const avl_tree_node_t node);
static void rotate(const avl_tree_t handle, const avl_tree_node_t node);
static void balance_insert(const avl_tree_t handle, const avl_tree_node_t node);
static void balance_delete(const avl_tree_t handle, const avl_tree_node_t node);
static int cut_single_son(const avl_tree_t handle, const avl_tree_node_t node);
static int subtree_nodes_number(const avl_tree_t node);
static int defualt_comparison_function(const void* key1, const void* key2);

avl_tree_t avl_tree_init(const avl_tree_comparison_function* comparison_function) {
	struct avl_tree* tree;
	if ((tree = malloc(sizeof(struct avl_tree))) == NULL) {
		return NULL;
	}
	tree->root = NULL;
	tree->n = 0;
	tree->compare = comparison_function ? comparison_function : &defualt_comparison_function;
	return tree;
}

int avl_tree_destroy(const avl_tree_t handle) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	struct avl_tree* left_subtree;
	struct avl_tree* right_subtree;
	left_subtree = avl_tree_cut_left(tree, tree->root);
	if (!left_subtree) {
		return 1;
	}
	right_subtree = avl_tree_cut_right(tree, tree->root);
	if (!right_subtree) {
		return 1;
	}
	if (left_subtree->root) {
		if (avl_tree_destroy(left_subtree)) {
			return 1;
		}
	}
	else {
		free(left_subtree);
	}
	if (right_subtree->root) {
		if (avl_tree_destroy(right_subtree)) {
			return 1;
		}
	}
	else {
		free(right_subtree);
	}
	if (avl_tree_node_destroy(tree->root)) {
		return 1;
	}
	free(tree);
	return 0;
}

inline long avl_tree_nodes_number(const avl_tree_t handle) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	return tree->n;
}

inline avl_tree_node_t avl_tree_get_root(const avl_tree_t handle) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	return tree->root;
}

inline void avl_tree_set_root(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	tree->root = node;
	tree->n = subtree_nodes_number(node);
}

void avl_tree_insert_as_left_subtree(const avl_tree_t handle, const avl_tree_node_t node, const avl_tree_t subtree) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	struct avl_tree* avl_tree_subtree = (struct avl_tree*)subtree;
	if (avl_tree_subtree->root) {
		avl_tree_node_set_father(avl_tree_subtree->root, node);
	}
	avl_tree_node_set_left_son(node, avl_tree_subtree->root);
	tree->n += subtree_nodes_number(avl_tree_subtree->root);
}

void avl_tree_insert_as_right_subtree(const avl_tree_t handle, const avl_tree_node_t node, const avl_tree_t subtree) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	struct avl_tree* avl_tree_subtree = (struct avl_tree*)subtree;
	if (avl_tree_subtree->root) {
		avl_tree_node_set_father(avl_tree_subtree->root, node);
	}
	avl_tree_node_set_right_son(node, avl_tree_subtree->root);
	tree->n += subtree_nodes_number(avl_tree_subtree->root);
}

avl_tree_t avl_tree_cut(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	if (node) {
		avl_tree_node_t father = avl_tree_node_get_father(node);

		if (node == avl_tree_get_root(tree)) {
			tree->root = NULL;
			tree->n = 0;
		}
		else if (node == avl_tree_node_get_left_son(father)) {
			if (avl_tree_node_degree(node) == 0) {
				avl_tree_node_set_left_son(father, NULL);
				tree->n--;
			}
			else {
				return avl_tree_cut_left(tree, node);
			}
		}
		else if (node == avl_tree_node_get_right_son(father)) {
			if (avl_tree_node_degree(node) == 0) {
				avl_tree_node_set_right_son(father, NULL);
				tree->n--;
			}
			else {
				return avl_tree_cut_right(tree, node);
			}
		}
	}
	avl_tree_t cutted = avl_tree_init(tree->compare);
	if (cutted) {
		avl_tree_set_root(cutted, node);
	}
	return cutted;
}

avl_tree_t avl_tree_cut_left(const avl_tree_t handle, const avl_tree_node_t father) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_t new_tree;
	avl_tree_node_t son = avl_tree_node_get_left_son(father);
	if ((new_tree = avl_tree_init(tree->compare)) == NULL) {
		return NULL;
	}
	avl_tree_set_root(new_tree, son);
	tree->n -= subtree_nodes_number(son);
	avl_tree_node_set_left_son(father, NULL);
	return new_tree;
}

avl_tree_t avl_tree_cut_right(const avl_tree_t handle, const avl_tree_node_t father) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_t new_tree;
	avl_tree_node_t son = avl_tree_node_get_right_son(father);
	if ((new_tree = avl_tree_init(tree->compare)) == NULL) {
		return NULL;
	}
	avl_tree_set_root(new_tree, son);
	tree->n -= subtree_nodes_number(son);
	avl_tree_node_set_right_son(father, NULL);
	return new_tree;
}

avl_tree_t avl_tree_cut_one_son_node(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_t cutted_tree;
	avl_tree_node_t son = NULL;
	if (avl_tree_node_get_left_son(node)) {
		son = avl_tree_node_get_left_son(node);
	}
	else if (avl_tree_node_get_right_son(node)) {
		son = avl_tree_node_get_right_son(node);
	}
	if (!son) {		//node is a leaf
		cutted_tree = avl_tree_cut(tree, node);
	}
	else {
		avl_tree_node_swap(node, son);
		cutted_tree = avl_tree_cut(tree, node);
		avl_tree_insert_as_left_subtree(tree, node, avl_tree_cut_left(cutted_tree, son));
		avl_tree_insert_as_right_subtree(tree, node, avl_tree_cut_right(cutted_tree, son));
		//memory leack
	}
	return cutted_tree;
}

void avl_tree_insert_single_node_tree(const avl_tree_t handle, const void* key, const avl_tree_t new_tree) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	int cmp;
	if (avl_tree_get_root(tree) == NULL) {
		avl_tree_set_root(tree, avl_tree_get_root(new_tree));
		//memory leack
	}
	else {
		avl_tree_node_t current = avl_tree_get_root(tree);
		avl_tree_node_t prev = NULL;
		while (current) {
			prev = current;
			cmp = tree->compare(key, avl_tree_node_get_key(current));
			if (cmp < 1) {
				current = avl_tree_node_get_left_son(current);
			}
			else {
				current = avl_tree_node_get_right_son(current);
			}
		}
		cmp = tree->compare(key, avl_tree_node_get_key(prev));
		if (cmp < 1) {
			avl_tree_insert_as_left_subtree(tree, prev, new_tree);
		}
		else {
			avl_tree_insert_as_right_subtree(tree, prev, new_tree);
		}
	}
}

avl_tree_node_t avl_tree_search_node(const avl_tree_t handle, const void* key) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_node_t current = avl_tree_get_root(tree);
	int cmp;
	while (current) {
		cmp = tree->compare(key, avl_tree_node_get_key(current));
		if (!cmp) {
			return current;
		}
		else if (cmp == -1) {
			current = avl_tree_node_get_left_son(current);
		}
		else {
			current = avl_tree_node_get_right_son(current);
		}
	}
	return NULL;
}

inline void* avl_tree_search(const avl_tree_t handle, const void* key) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	return avl_tree_node_get_value(avl_tree_search_node(tree, key));
}

int avl_tree_insert(const avl_tree_t handle, const void* key, const void* value) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_node_t node = avl_tree_node_init(key, value);
	avl_tree_t new_tree = avl_tree_init(tree->compare);
	avl_tree_set_root(new_tree, node);
	avl_tree_insert_single_node_tree(tree, key, new_tree);
	balance_insert(tree, node);
	avl_tree_set_root(new_tree, NULL);
	avl_tree_destroy(new_tree);
	return 0;
}

int avl_tree_delete(const avl_tree_t handle, const void* key) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_node_t node = avl_tree_search_node(tree, key);
	if (node) {
		if (avl_tree_node_degree(node) < 2) {
			return cut_single_son(tree, node);
		}
		else {
			avl_tree_node_t pred = avl_tree_node_get_pred(node);
			avl_tree_node_swap(node, pred);
			int tmp_h = avl_tree_node_get_height(node);
			avl_tree_node_set_height(node, avl_tree_node_get_height(pred));
			avl_tree_node_set_height(pred, tmp_h);
			return cut_single_son(tree, pred);
		}
	}
	return 0;
}

int avl_tree_delete_node(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	if (node) {
		if (avl_tree_node_degree(node) < 2) {
			return cut_single_son(tree, node);
		}
		else {
			avl_tree_node_t pred = avl_tree_node_get_pred(node);
			avl_tree_node_swap(node, pred);
			int tmp_h = avl_tree_node_get_height(node);
			avl_tree_node_set_height(node, avl_tree_node_get_height(pred));
			avl_tree_node_set_height(pred, tmp_h);
			return cut_single_son(tree, pred);
		}
	}
	return 0;
}

static void right_rotation(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	avl_tree_node_t left_son = avl_tree_node_get_left_son(node);
	avl_tree_node_swap(node, left_son);

	avl_tree_t l_tree = avl_tree_cut_left(tree, node);
	avl_tree_t r_tree = avl_tree_cut_right(tree, node);
	avl_tree_t l_tree_l = avl_tree_cut_left(l_tree, left_son);
	avl_tree_t l_tree_r = avl_tree_cut_right(l_tree, left_son);

	avl_tree_insert_as_right_subtree(l_tree, avl_tree_get_root(l_tree), r_tree);
	avl_tree_insert_as_left_subtree(l_tree, avl_tree_get_root(l_tree), l_tree_r);
	avl_tree_insert_as_right_subtree(tree, node, l_tree);
	avl_tree_insert_as_left_subtree(tree, node, l_tree_l);

	avl_tree_node_update_height(avl_tree_node_get_right_son(node));
	avl_tree_node_update_height(node);

	avl_tree_set_root(l_tree, NULL);
	avl_tree_set_root(r_tree, NULL);
	avl_tree_set_root(l_tree_l, NULL);
	avl_tree_set_root(l_tree_r, NULL);
	avl_tree_destroy(l_tree);
	avl_tree_destroy(r_tree);
	avl_tree_destroy(l_tree_l);
	avl_tree_destroy(l_tree_r);
}

static void left_rotation(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;

	avl_tree_node_t right_son = avl_tree_node_get_right_son(node);
	avl_tree_node_swap(node, right_son);

	avl_tree_t r_tree = avl_tree_cut_right(tree, node);
	avl_tree_t l_tree = avl_tree_cut_left(tree, node);
	avl_tree_t r_tree_l = avl_tree_cut_left(r_tree, right_son);
	avl_tree_t r_tree_r = avl_tree_cut_right(r_tree, right_son);

	avl_tree_insert_as_left_subtree(r_tree, avl_tree_get_root(r_tree), l_tree);
	avl_tree_insert_as_right_subtree(r_tree, avl_tree_get_root(r_tree), r_tree_l);
	avl_tree_insert_as_left_subtree(tree, node, r_tree);
	avl_tree_insert_as_right_subtree(tree, node, r_tree_r);

	avl_tree_node_update_height(avl_tree_node_get_left_son(node));
	avl_tree_node_update_height(node);

	avl_tree_set_root(r_tree, NULL);
	avl_tree_set_root(l_tree, NULL);
	avl_tree_set_root(r_tree_l, NULL);
	avl_tree_set_root(r_tree_r, NULL);
	avl_tree_destroy(r_tree);
	avl_tree_destroy(l_tree);
	avl_tree_destroy(r_tree_l);
	avl_tree_destroy(r_tree_r);
}

static void rotate(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;

	long balanced_factor = avl_tree_node_get_balance_factor(node);
	if (balanced_factor == 2) {		//height of the node's left son is 2 times greater than the right one
		if (avl_tree_node_get_balance_factor(avl_tree_node_get_left_son(node)) >= 0) {	//LL balancing
			right_rotation(tree, node);
		}
		else {	//LR balancing
			left_rotation(tree, avl_tree_node_get_left_son(node));
			right_rotation(tree, node);
		}
	}
	else if (balanced_factor == -2) {	//height of the node's right son is 2 times greater than the left one
		if (avl_tree_node_get_balance_factor(avl_tree_node_get_left_son(node)) >= 0) {	//RR balancing
			left_rotation(tree, node);
		}
		else {	//RL balancing
			right_rotation(tree, avl_tree_node_get_right_son(node));
			left_rotation(tree, node);
		}
	}
}

static void balance_insert(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;

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
		rotate(tree, current);
	}
}

static void balance_delete(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;

	avl_tree_node_t current = avl_tree_node_get_father(node);
	while (current) {
		if (abs(avl_tree_node_get_balance_factor(current)) == 2) {
			rotate(tree, current);
		}
		else {
			avl_tree_node_update_height(current);
		}
		current = avl_tree_node_get_father(current);
	}
}

static int cut_single_son(const avl_tree_t handle, const avl_tree_node_t node) {
	struct avl_tree* tree = (struct avl_tree*)handle;
	struct avl_tree* cutted = (struct avl_tree*)handle;
	cutted = avl_tree_cut_one_son_node(tree, node);
	balance_delete(tree, node);
	return avl_tree_destroy(cutted);
}

static int subtree_nodes_number(const avl_tree_t node) {
	int nodes_number = 0;
	_stack_t stack = stack_init();
	if (node) {
		stack_push(stack, node);
		while (!stack_is_empty(stack)) {
			struct binary_tree_node* current_node;
			current_node = stack_pop(stack);
			nodes_number++;
			if (avl_tree_node_get_left_son(current_node)) {
				stack_push(stack, avl_tree_node_get_left_son(current_node));
			}
			if (avl_tree_node_get_right_son(current_node)) {
				stack_push(stack, avl_tree_node_get_right_son(current_node));
			}
		}
	}
	stack_destroy(stack);
	return nodes_number;
}

static int defualt_comparison_function(const void* key1, const void* key2) {
	if (key1 < key2) {
		return -1;
	}
	else if (key1 > key2) {
		return 1;
	}
	return 0;
}

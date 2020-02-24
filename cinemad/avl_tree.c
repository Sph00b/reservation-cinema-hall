#include "avl_tree.h"

#include <stdlib.h>

#include "binary_search_tree.h"

#define HEIGHT_INDEX 2

struct avl_tree {
    binary_search_tree_t binary_search_tree;
};

//Node's info now is a triple [ key, value, height]

avl_tree_t avl_tree_init(void* (*order_function)(void*, void*)) {
    struct avl_tree* avl_tree;
    if ((avl_tree = malloc(sizeof(struct avl_tree))) == NULL) {
        return NULL;
    }
    avl_tree->binary_search_tree = binary_search_tree_init(order_function);
}

int avl_tree_destroy(avl_tree_t handle) {
    struct avl_tree* avl_tree = (struct avl_tree*)handle;
    if (binary_search_tree_destroy(avl_tree->binary_search_tree)) {
        return 1;
    }
    free(avl_tree);
    return 0;
}

int height(avl_tree_t handle, node_t node) {
    struct avl_tree* avl_tree = (struct avl_tree*)handle;
    if (node == NULL) {
        return -1;
    }
    return tree_get_info(avl_tree->binary_search_tree, node)[HEIGHT_INDEX];
}

void set_height(avl_tree_t handle, node_t node, int h) {
    struct avl_tree* avl_tree = (struct avl_tree*)handle;
    if (node) {
        tree_set_info(avl_tree->binary_search_tree, node)[HEIGHT_INDEX] = h;
    }
}

void balanceFactor(avl_tree_t handle, node_t node) {
    struct avl_tree* avl_tree = (struct avl_tree*)handle;
    if (node) {
        return height(avl_tree, get_left_son(node)) - height(avl_tree, get_right_son(node));
    }
    return 0;
}

void update_height(avl_tree_t handle, node_t node) {
    struct avl_tree* avl_tree = (struct avl_tree*)handle;
    if (node) {
        set_height(avl_tree, node, max(height(get_left_son(node)), height(get_right_son(node))) + 1);
    }
}

#undef HEIGHT_INDEX

#pragma once

#include "bst_node.h"

/*
    Questo modulo contiene l'implementazione
    dell'albero binario di ricerca (BST)

    Un albero binario di ricerca e' un albero che soddisfa le seguenti proprieta':
    1. ogni nodo v contiene un valore (info[1]) cui e' associata una chiave (info[0])
        presa da n dominio totalmente ordinato.
    2. Le chiavi nel sottoalbero sinistro di v sono <= chiave(v).
    3. Le chiavi nel sottoalbero destro di v sono >= chiave(v).

*/

typedef void* bst_t;

typedef int *bst_comparison_function(const void* key1, const void* key2);

bst_t bst_init(bst_node_t root, bst_comparison_function* comparison_function);

int bst_destroy(bst_t handle);

long bst_nodes_number(bst_t handle);

bst_node_t bst_get_root(bst_t handle);

int bst_set_root(bst_t handle, bst_node_t node);

int bst_insert_as_left_subtree(bst_t handle, bst_node_t node, bst_t subtree);

int bst_insert_as_right_subtree(bst_t handle, bst_node_t node, bst_t subtree);

bst_t bst_cut(bst_t handle, bst_node_t node);

bst_t bst_cut_left(bst_t handle, bst_node_t node);

bst_t bst_cut_right(bst_t handle, bst_node_t node);

bst_t bst_cut_one_son_node(bst_t handle, bst_node_t node);

void bst_insert_single_node_tree(bst_t handle, void* key, bst_t new_tree);

bst_node_t bst_search_node(bst_t handle, void* key);

void* bst_search(bst_t handle, void* key);

int bst_insert(bst_t handle, void* key, void* value);

void bst_delete(bst_t handle, void* key);
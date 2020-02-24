#pragma once

#include "node.h"

typedef void* tree_t;

extern node_t tree_get_root(tree_t handle);

extern void tree_set_root(tree_t handle, node_t node);

extern long tree_nodes_number(tree_t handle);

extern tree_t tree_cut(tree_t handle, node_t node);

/*
	:param node: radice del sottoalbero
	:return: numero di nodi del sottoalbero radicato in node
*/
extern int subtree_nodes_number(tree_t handle, node_t node);
/*
	incrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
extern void increase_nodes_number_by_subtree(tree_t handle, node_t node);
/*
	decrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
extern void decrease_nodes_number_by_subtree(tree_t handle, node_t node);

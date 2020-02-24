#pragma once

#include "node.h"

typedef void* tree_t;

long tree_nodes_number(tree_t handle);

tree_t tree_cut(tree_t handle, node_t node);

/*
	:param node: radice del sottoalbero
	:return: numero di nodi del sottoalbero radicato in node
*/
int subtree_nodes_number(tree_t handle, node_t node);
/*
	incrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
void increase_nodes_number_by_subtree(tree_t handle, node_t node);
/*
	decrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
void decrease_nodes_number_by_subtree(tree_t handle, node_t node);

#pragma once

#include "tree.h"
#include "bool.h"

typedef void* binary_tree_t;

/*
	:param root: radice dell'albero. Se non  specificata,
	albero vuoto
*/
binary_tree_t binary_tree_init(node_t root);

int binary_tree_destroy(binary_tree_t handle);

/*
	:param node:
	:return: true se node è una foglia, false altrimenti
*/
bool_t binary_tree_is_leaf(binary_tree_t handle, node_t node);

/*
	Permette di inserire la radice di un sottoalbero come figlio sinistro
	del nodo father
	: param father : nodo su cui attaccare subtree
	: param subtree : da inserire
*/
void binary_tree_insert_as_left_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree);

/*
	Permette di inserire la radice di un sottoalbero come figlio destro
	del nodo father
	:param father: nodo su cui attaccare subtree
	:param subtree: da inserire
*/
void binary_tree_insert_as_right_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree);

/*
	Permette di rimuovere l'intero sottoalbero che parte dal figlio
	sinistro del nodo father
	:param father:
	:return: sottoalbero radicato nel figlio sinistro di father
*/
binary_tree_t binary_tree_cut_left(binary_tree_t handle, node_t node);

/*
	Permette di rimuovere l'intero sottoalbero che parte dal figlio
	destro del nodo father
	:param father:
	:return: sottoalbero radicato nel figlio destro di father
*/
binary_tree_t binary_tree_cut_right(binary_tree_t handle, node_t node);

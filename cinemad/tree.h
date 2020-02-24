#pragma once

/*
    Questo modulo contiene la classe Tree che rappresenta
    un generico albero.
    La classe va estesa per creare uno specifico tipo di albero
    (binario, d-ario, AVL, ecc)
*/

#include "list.h"

typedef void* node_t;
typedef void* tree_t;

/*
    :return: numero di nodi dell'albero
*/
long tree_nodes_number(tree_t handle);

/*
    :param node :
    : return : numero di figli di node
*/
long tree_degree(tree_t handle, node_t node);

node_t tree_get_root(tree_t handle);

/*
    :param node :
    : return : padre di node
*/
node_t tree_get_father(tree_t handle, node_t node);

/*
    :param node:
    :return: lista dei figli di node
*/
list_t tree_get_sons(tree_t handle, node_t node);

/*
    Sradica l'albero da node (compreso)
    :param node:
    :return:
*/
tree_t tree_cut(tree_t handle, node_t node);

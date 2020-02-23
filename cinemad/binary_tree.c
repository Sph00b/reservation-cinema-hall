#include "binary_tree.h"

/*
	Questo modulo contiene implementazione di alberi binari
*/

#include <stdlib.h>

#include "stack.h"

/*	Binary Node	*/

struct binary_node {
	binary_node_t father;
	binary_node_t left_son;
	binary_node_t right_son;
	void* data;
};

/*
	param data: una struttura dati qualsiasi (lista, dizionario, ecc)
	o una semplice variabile. Permette di aggiungere informazioni
	aggiuntive
*/
binary_node_t binary_node_init(void* data) {
	struct binary_node* binary_node;
	if ((binary_node = malloc(sizeof(struct binary_node))) == NULL) {
		return NULL;
	}
	binary_node->data = data;
	binary_node->father = NULL;
	binary_node->left_son = NULL;
	binary_node->right_son = NULL;
	return binary_node;
}

int binary_node_destroy(binary_node_t handle) {
	struct binary_node* binary_node;
	binary_node_destroy(binary_node->left_son);
	binary_node_destroy(binary_node->right_son);
	//free(binary_node->data);
	free(binary_node);
	return 0;
}

/*	Binary Tree	*/

struct binary_tree {
	binary_node_t root;
	long n;
};

int subtreeNodesNumber(binary_node_t node);
void increaseNodesNumberBySubtree(binary_tree_t handle, binary_node_t node);
void decreaseNodesNumberBySubtree(binary_tree_t handle, binary_node_t node);

/*
	:param root: radice dell'albero. Se non  specificata,
	albero vuoto
*/
binary_tree_t binary_tree_init(binary_node_t root) {
	struct binary_tree* binary_tree;
	struct binary_node* binary_node = (struct binary_node*)root;
	if ((binary_tree = malloc(sizeof(struct binary_tree))) == NULL) {
		return NULL;
	}
	binary_tree->root = binary_node;
	binary_tree->n = 0;
	if (binary_tree->root) {
		increaseNodesNumberBySubtree(binary_tree, binary_node);
	}
	return binary_tree;
}

int binary_tree_destroy(binary_tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	return binary_node_destroy(binary_tree->root);
	free(binary_tree);
}

/*
	:param node:
	:return: true se node è una foglia, false altrimenti
*/
bool_t binary_tree_is_leaf(binary_tree_t handle, binary_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	if (list_lenght(node_sons_of(binary_node))) {
		return false;
	}
	return true;
}

/*
	Permette di inserire la radice di un sottoalbero come figlio sinistro
	del nodo father
	: param father : nodo su cui attaccare subtree
	: param subtree : da inserire
*/
void binary_tree_insert_as_left_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root != NULL) {
		binary_subtree->root->father = binary_node;
		increaseNodesNumberBySubtree(binary_tree, binary_subtree->root);
	}
	binary_node->left_son = binary_subtree->root->father;   //memory leack
}

/*
	Permette di inserire la radice di un sottoalbero come figlio destro
	del nodo father
	:param father: nodo su cui attaccare subtree
	:param subtree: da inserire
*/
void binary_tree_insert_as_right_subtree(binary_tree_t handle, node_t node, binary_tree_t subtree) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* binary_subtree = (struct binary_tree*)subtree;
	if (binary_subtree->root != NULL) {
		binary_subtree->root->father = binary_node;
		increaseNodesNumberBySubtree(binary_tree, binary_subtree->root);
	}
	binary_node->right_son = binary_subtree->root->father;   //memory leack
}

/*
	Permette di rimuovere l'intero sottoalbero che parte dal figlio
	sinistro del nodo father
	:param father:
	:return: sottoalbero radicato nel figlio sinistro di father
*/
binary_tree_t binary_tree_cutLeft(binary_tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* new_tree = binary_tree_init(binary_node->left_son);
	decreaseNodesNumberBySubtree(binary_tree, binary_node->left_son);
	binary_node->left_son = NULL;	//memory leack?
	return new_tree;
}

/*
	Permette di rimuovere l'intero sottoalbero che parte dal figlio
    destro del nodo father
    :param father:
    :return: sottoalbero radicato nel figlio destro di father
*/
binary_tree_t binary_tree_cutRight(binary_tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	struct binary_tree* new_tree = binary_tree_init(binary_node->right_son);
	decreaseNodesNumberBySubtree(binary_tree, binary_node->right_son);
	binary_node->right_son = NULL;	//memory leack?
	return new_tree;
}

/*
	:param node: radice del sottoalbero
	:return: numero di nodi del sottoalbero radicato in node
*/
int subtreeNodesNumber(binary_node_t node) {
	struct binary_node* binary_node = (struct binary_node*)node;
	list_t res = list_init();
	_stack_t stack = stack_init();
	if (binary_node) {
		stack_push(stack, binary_node);
		while (!stack_is_empty(stack)) {
			struct binary_node* current_node;
			current_node = stack_pop(stack);
			list_append(res, current_node);
			if (current_node->left_son) {
				stack_push(stack, current_node->left_son);
			}
			if (current_node->right_son) {
				stack_push(stack, current_node->right_son);
			}
		}
	}
	return list_lenght(res);
}

/*
	incrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
void increaseNodesNumberBySubtree(binary_tree_t handle, binary_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	binary_tree->n += subtreeNodesNumber(binary_node);
}

/*
	decrementa il numero di nodi nell'albero con il
	numero dei nodi del sottoalbero radicato in node
	:param node:
*/
void decreaseNodesNumberBySubtree(binary_tree_t handle, binary_node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	binary_tree->n -= subtreeNodesNumber(binary_node);
}

/*
	METODI EREDITATI DALLA CLASSE TREE.
	VEDERE LA CLASSE PER DESCRIZIONE DEI METODI
*/

long node_degree(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	return list_lenght(node_sons_of(binary_node));
}

node_t node_father_of(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	if (binary_node == NULL) {
		return NULL;
	}
	return binary_node->father;
}

list_t node_sons_of(node_t handle) {
	struct binary_node* binary_node = (struct binary_node*)handle;
	list_t sons;
	if ((sons = list_init()) == NULL) {
		return NULL;
	}
	if (binary_node != NULL) {
		if (binary_node->left_son != NULL) {
			list_append(sons, binary_node->left_son);
		}
		if (binary_node->right_son != NULL) {
			list_append(sons, binary_node->right_son);
		}
	}
	return sons;
}

long tree_nodes_number(tree_t handle) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	return binary_tree->n;
}

tree_t tree_cut(tree_t handle, node_t node) {
	struct binary_tree* binary_tree = (struct binary_tree*)handle;
	struct binary_node* binary_node = (struct binary_node*)node;
	if (binary_node == NULL) {
		return binary_tree_init(NULL);
	}
	if (binary_node->father == NULL) {  // the node is the root
		binary_tree->root = NULL;	   // memory leack
		binary_tree->n = 0;
		return binary_tree_init(binary_node);
	}
	if (is_leaf(binary_tree, binary_node)) {
		if (binary_node->father->left_son == binary_node) {
			binary_node->father->left_son = NULL;   //memory leack
		}
		else {
			binary_node->father->right_son = NULL;  //memory leack
		}
		binary_tree->n--;
		return binary_tree_init(binary_node);
	}
	else if (binary_node->father->left_son == binary_node) {
		return cut_left(binary_tree, binary_node);
	}
	else {
		return cut_right(binary_tree, binary_node);
	}
}

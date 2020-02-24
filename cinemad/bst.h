#pragma once

#include "tree.h"

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

bst_t bst_init(void* (*order_function)(void*, void*));

int bst_destroy(bst_t handle);

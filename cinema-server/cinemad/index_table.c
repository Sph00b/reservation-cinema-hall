#include "index_table.h"

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <try.h>
#include <data-structure/avl_tree.h>
#include <data-structure/stack.h>

struct index_table {
	avl_tree_t avl_tree;
	pthread_rwlock_t lock;
	index_record_t (*record_init)();
	int (*record_destroy)(void* key, void* value);
};

extern index_table_t index_table_init(
	const index_record_t(*record_init)(),
	const int (*record_destroy)(void* key, void* value),
	const int (*comparison_function)(const void* key1, const void* key2)) {

	struct index_table* index_table;
	index_table = malloc(sizeof * index_table);
	if (index_table) {
		try(index_table->avl_tree = avl_tree_init(comparison_function), NULL, error);
		try_pthread_rwlock_init(&index_table->lock, cleanup);
		index_table->record_init = record_init;
		index_table->record_destroy = record_destroy;
	}
	return index_table;

cleanup:
	avl_tree_destroy(index_table->avl_tree);
error:
	free(index_table);
	return NULL;
}

extern int index_table_destroy(const index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	try_pthread_rwlock_destroy(&index_table->lock, error);
	// TODO: extract this ------------------------------------------------------
	avl_tree_node_t node = avl_tree_get_root(index_table->avl_tree);
	_stack_t stack = stack_init();
	if (node) {
		stack_push(stack, node);
		while (!stack_is_empty(stack)) {
			struct binary_tree_node* current_node;
			current_node = stack_pop(stack);
			if (avl_tree_node_get_left_son(current_node)) {
				stack_push(stack, avl_tree_node_get_left_son(current_node));
			}
			if (avl_tree_node_get_right_son(current_node)) {
				stack_push(stack, avl_tree_node_get_right_son(current_node));
			}
			if (index_table->record_destroy(avl_tree_node_get_key(current_node), avl_tree_node_get_value(current_node))) {
				return 1;
			}
		}
	}
	stack_destroy(stack);
	//--------------------------------------------------------------------------
	try(avl_tree_destroy(index_table->avl_tree), !0, error);
	free(index_table);
	return 0;

error:
	return 1;
}

int index_table_insert(const index_table_t handle, const void* key, const void* record) {
	struct index_table* index_table = (struct index_table*)handle;
	int result;
	try_pthread_rwlock_wrlock(&index_table->lock, error);
	result = avl_tree_insert(index_table->avl_tree, key, record);
	try_pthread_rwlock_unlock(&index_table->lock, error);
	return result;

error:
	return 1;
}

extern int index_table_delete(const index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	int result;
	int ret;
	try_pthread_rwlock_wrlock(&index_table->lock, error);
	avl_tree_node_t node = avl_tree_search_node(index_table->avl_tree, key);
	try(index_table->record_destroy(avl_tree_node_get_key(node), avl_tree_node_get_value(node)), !0, error);
	result = avl_tree_delete_node(index_table->avl_tree, node);
	try_pthread_rwlock_unlock(&index_table->lock, error);
	return result;

error:
	return 1;
}

extern index_record_t index_table_search(const index_table_t handle, void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	void* result;
	try_pthread_rwlock_rdlock(&index_table->lock, error);
	if ((result = avl_tree_search(index_table->avl_tree, key)) == NULL) {
		try_pthread_rwlock_unlock(&index_table->lock, error);
		try_pthread_rwlock_wrlock(&index_table->lock, error);
		if ((result = avl_tree_search(index_table->avl_tree, key)) == NULL) {
			index_record_t record;
			try(record = index_table->record_init(), NULL, error);
			try(avl_tree_insert(index_table->avl_tree, key, record), !0, error);
			result = record;
		}
	}
	else {
		free(key);	// ?
	}
	try_pthread_rwlock_unlock(&index_table->lock, error);
	return result;

error:
	return NULL;
}

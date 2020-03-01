#include "index_table.h"

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "avl_tree.h"
#include "stack.h"

struct index_table {
	avl_tree_t avl_tree;
	pthread_rwlock_t lock;
	index_record_t (*record_init)();
	int (*record_destroy)(void* key, void* value);
};

index_table_t index_table_init(
	index_record_t(*record_init)(),
	int (*record_destroy)(void* key, void* value),
	int (*comparison_function)(const void* key1, const void* key2)) {

	struct index_table* index_table;
	if ((index_table = malloc(sizeof(struct index_table))) == NULL) {
		return NULL;
	}
	if ((index_table->avl_tree = avl_tree_init(comparison_function)) == NULL) {
		free(index_table);
		return NULL;
	}
	int ret;
	while ((ret = pthread_rwlock_init(&index_table->lock, NULL)) && errno == EINTR);
	if (ret) {
		free(index_table);
		return NULL;
	}
	index_table->record_init = record_init;
	index_table->record_destroy = record_destroy;
	return index_table;
}

int index_table_destroy(index_table_t handle) {
	struct index_table* index_table = (struct index_table*)handle;
	int ret;
	while ((ret = pthread_rwlock_destroy(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
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
	if (avl_tree_destroy(index_table->avl_tree)) {
		return 1;
	}
	free(index_table);
	return 0;
}

int index_table_insert(index_table_t handle, const void* key, const void* record) {
	struct index_table* index_table = (struct index_table*)handle;
	int result;
	int ret;
	while ((ret = pthread_rwlock_wrlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	result = avl_tree_insert(index_table->avl_tree, key, record);
	while ((ret = pthread_rwlock_unlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return result;
}

int index_table_delete(index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	int result;
	int ret;
	while ((ret = pthread_rwlock_wrlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	avl_tree_node_t node = avl_tree_search_node(index_table->avl_tree, key);
	if (index_table->record_destroy(avl_tree_node_get_key(node), avl_tree_node_get_value(node))) {
		return 1;
	}
	result = avl_tree_delete_node(index_table->avl_tree, node);
	while ((ret = pthread_rwlock_unlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return 1;
	}
	return result;
}

index_record_t index_table_search(index_table_t handle, const void* key) {
	struct index_table* index_table = (struct index_table*)handle;
	void* result;
	int ret;
	while ((ret = pthread_rwlock_rdlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return NULL;
	}
	if ((result = avl_tree_search(index_table->avl_tree, key)) == NULL) {
		while ((ret = pthread_rwlock_unlock(&index_table->lock)) && errno == EINTR);
		if (ret) {
			return NULL;
		}
		while ((ret = pthread_rwlock_wrlock(&index_table->lock)) && errno == EINTR);
		if (ret) {
			return NULL;
		}
		if ((result = avl_tree_search(index_table->avl_tree, key)) == NULL) {
			index_record_t record;
			if ((record = index_table->record_init()) == NULL) {
				return NULL;
			}
			if (avl_tree_insert(index_table->avl_tree, key, record)) {
				return NULL;
			}
			result = record;
		}
	}
	else {
		free(key);
	}
	while ((ret = pthread_rwlock_unlock(&index_table->lock)) && errno == EINTR);
	if (ret) {
		return NULL;
	}
	return result;
}

#include "list.h"

#include <stdlib.h>

struct list_node {
	void* data;
	struct list_node* next;
	struct list_node* last;
};

list_t list_init() {
	struct list_node* list;
	if ((list = malloc(sizeof(struct list_node))) == NULL) {
		return NULL;
	}
	list->data = NULL;
	list->next = NULL;
	list->last = list;
	return list;
}

int list_destroy(list_t handle) {
	struct list_node* list = (struct list_node*)handle;
	if (list->next) {
		list_destroy(list->next);
	}
	free(list);
}

long list_lenght(list_t handle) {
	struct list_node* list = (struct list_node*)handle;
	struct list_node* tmp = list;
	long len;
	for (len = 0; tmp->next; len++) {
		tmp = tmp->next;
	}
	return len;
}

int list_append(list_t handle, void* item) {
	struct list_node* list = (struct list_node*)handle;
	struct list_node* node;
	if ((node = malloc(sizeof(struct list_node))) == NULL) {
		return 1;
	}
	node->data = item;
	node->next = NULL;
	list->last->next = node;
	list->last = node;
	return 0;
}

#include "flag.h"
#include <stdlib.h>

struct flag {
	int value;
};

flag_t stack_init() {
	struct flag* flag;
	if ((flag = malloc(sizeof(struct flag))) == NULL) {
		return NULL;
	}
	flag->value = 0;
	return flag;
}

void flag_destroy(const flag_t handle) {
	struct flag* flag = (struct flag*)handle;
	free(flag);
}

int flag_status(const flag_t handle) {
	struct flag* flag = (struct flag*)handle;
	return flag->value;
}

void flag_set(const flag_t handle) {
	struct flag* flag = (struct flag*)handle;
	flag->value = 1;
}

void flag_unset(const flag_t handle) {
	struct flag* flag = (struct flag*)handle;
	flag->value = 0;
}

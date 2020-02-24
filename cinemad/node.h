#pragma once

#include "list.h"

typedef void* node_t;

long node_degree(node_t handle);

node_t node_get_father(node_t handle);

void node_set_father(node_t handle, node_t father);

list_t node_get_sons(node_t handle);

void* node_get_info(node_t handle);

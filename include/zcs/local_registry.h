#ifndef LOCAL_REGISTRY_H
#define LOCAL_REGISTRY_H

#include "zcs_structs.h"
#include "zcs_utils.h"

void start_local_registry();

void add_node_to_registry(zcs_node_t *node);

zcs_node_t *find_node_in_registry(char *name);

zcs_node_t *get_head_of_registry();

void free_registry();

#endif

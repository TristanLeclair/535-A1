#ifndef ZCS_UTILS_H
#define ZCS_UTILS_H

#include "zcs_structs.h"
#include "../logs/log_manager.h"
#include <string.h>

zcs_node_t *find_node_by_name(node_list_t *node_list, char *name);

void copy_array(const zcs_attribute_t given_attributes[],
                zcs_attribute_t **local_attribute_array, int num);

#endif

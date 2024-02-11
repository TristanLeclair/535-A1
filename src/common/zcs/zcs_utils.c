#include "../../../include/zcs/zcs_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

zcs_node_t *find_node_by_name(node_list_t *node_list, char *name) {
  if (name == NULL) {
    return NULL;
  }
  zcs_node_t *current = node_list->head;
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}


void copy_array(const zcs_attribute_t given_attributes[],
                zcs_attribute_t **local_attribute_array, int num) {
  *local_attribute_array = malloc(num * sizeof(zcs_attribute_t));

  if (*local_attribute_array == NULL) {
    exit(EXIT_FAILURE);
  }

  memcpy(*local_attribute_array, given_attributes,
         num * sizeof(zcs_attribute_t));
}

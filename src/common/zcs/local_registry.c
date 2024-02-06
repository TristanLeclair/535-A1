#include "../../../include/zcs/local_registry.h"
#include <stdlib.h>

node_list_t *local_registry;

void start_local_registry() { local_registry = malloc(sizeof(node_list_t)); }

void add_node_to_registry(zcs_node_t *node) {
  if (local_registry->head == NULL) {
    local_registry->head = node;
    local_registry->tail = node;
  } else {
    local_registry->tail->next = node;
    local_registry->tail = node;
  }
}

zcs_node_t *find_node_in_registry(char *name) {
  return find_node_by_name(local_registry, name);
}

zcs_node_t *get_head_of_registry() {
  return local_registry->head;
}

void free_registry() {
  free(local_registry);
}

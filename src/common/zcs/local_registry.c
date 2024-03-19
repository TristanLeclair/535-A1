#include "zcs/local_registry.h"
#include "zcs/zcs_utils.h"
#include <stdlib.h>

node_list_t *local_registry;
int emtpy = 1;

void start_local_registry() { local_registry = malloc(sizeof(node_list_t)); }

int local_registry_empty() {
  return emtpy;
}

void add_node_to_registry(zcs_node_t *node) {
  if (local_registry->head == NULL) {
    emtpy = 0;
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
  emtpy = 1;
  free(local_registry);
}

#include "logs/log_structs.h"
#include <stdlib.h>

log_list_t *log_list;

up_down_log_t *get_log_head() { return log_list->head; }

void add_log(up_down_log_t *log) {
  if (log_list == NULL) {
    log_list = (log_list_t *)malloc(sizeof(log_list_t));
  }

  if (log_list->head == NULL) {
    log_list->head = log;
    log_list->tail = log;
    log_list->current_size = 1;
  } else {
    log_list->tail->next = log;
    log_list->tail = log;
    log_list->current_size++;

    if (log_list->current_size > MAX_LOG_SIZE) {
      up_down_log_t *to_delete = log_list->head;
      log_list->head = to_delete->next;
      free(to_delete->log_entry);
      free(to_delete);
    }
  }
}

void free_logs() {
  if (log_list == NULL) {
    return;
  }
  up_down_log_t *current = get_log_head();
  up_down_log_t *next;
  while (current != NULL) {
    next = current->next;
    free(current->log_entry);
    free(current);
    current = next;
  }

  free(log_list);
}

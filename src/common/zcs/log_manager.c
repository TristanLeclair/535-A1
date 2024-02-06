#include "../../../include/zcs/log_manager.h"
#include <stdlib.h>
#include <string.h>

void create_log(log_list_t *log_list, const char *log_entry) {
  up_down_log_t *log_object = (up_down_log_t *)malloc(sizeof(up_down_log_t));
  if (log_object == NULL) {
    // TODO: handle error
    return;
  }

  strncpy(log_object->log_entry, log_entry, LOG_ENTRY_SIZE - 1);
  log_object->log_entry[LOG_ENTRY_SIZE - 1] = '\0';
  add_log(log_list, log_object);
}

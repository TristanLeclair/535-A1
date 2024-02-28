#include "logs/log_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_log(const char *log_entry) {
  up_down_log_t *log_object = (up_down_log_t *)malloc(sizeof(up_down_log_t));
  if (log_object == NULL) {
    return;
  }

  strncpy(log_object->log_entry, log_entry, LOG_ENTRY_SIZE - 1);
  log_object->log_entry[LOG_ENTRY_SIZE - 1] = '\0';
  add_log(log_object);
}

void create_up_down_log(char *service_name,
                        enum Status status) {
  char *status_log = status_to_text(status);
  char buffer[69];
  snprintf(buffer, 69, "%s: %s", service_name, status_log);
  create_log(buffer);
}

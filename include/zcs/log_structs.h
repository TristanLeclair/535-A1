#ifndef LOG_STRUCTS_H
#define LOG_STRUCTS_H

#define MAX_LOG_ENTRY_SIZE 69
#define MAX_LOG_SIZE 50

typedef struct up_down_log {
  char log_entry[MAX_LOG_ENTRY_SIZE];
  struct up_down_log *next;
} up_down_log_t;

typedef struct {
  up_down_log_t *head;
  up_down_log_t *tail;
  int current_size;
} log_list_t;

void add_log(log_list_t *log_list, up_down_log_t *log);

#endif

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "log_structs.h"

#define LOG_ENTRY_SIZE 69

void create_log(log_list_t *log_list, const char *log_entry);

#endif

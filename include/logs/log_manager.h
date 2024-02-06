#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "log_structs.h"
#include "../zcs/zcs_structs.h"

#define LOG_ENTRY_SIZE 69

void create_log(const char *log_entry);

void create_up_down_log(char *service_name,
                        enum Status status);

#endif

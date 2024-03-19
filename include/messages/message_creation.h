#ifndef MESSAGE_CREATION_H
#define MESSAGE_CREATION_H

#include "../zcs/zcs_structs.h"

char *create_discovery_msg(char lan);
char *create_heartbeat_msg(char lan, char *service_name);
char *create_notification_msg(char lan, char *service_name, int num_attr, zcs_attribute_t *attribute_array);
char *create_ad_msg(char lan, char *service_name, char *ad_name, char *ad_value);

#endif

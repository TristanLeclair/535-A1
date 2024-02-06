#include "../../../include/messages/message_creation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create_discovery_msg() {
  size_t len = snprintf(NULL, 0, "%d#", DISCOVERY) + 1;

  char *result = malloc(len);

  snprintf(result, len, "%d#", DISCOVERY);

  return result;
}

char *create_heartbeat_msg(char *service_name) {
  size_t len = snprintf(NULL, 0, "%d#%s#", HEARTBEAT, service_name) + 1;

  char *result = malloc(len);

  snprintf(result, len, "%d#%s#", HEARTBEAT, service_name);

  return result;
}

char *create_notification_msg(char *service_name, int num_attr, zcs_attribute_t *attribute_array) {
  enum Msg_type type = NOTIFICATION;
  size_t header_len = snprintf(NULL, 0, "%d#%s#", type, service_name);
  header_len += 1;
  size_t total_len = header_len;

  for (int i = 0; i < num_attr; ++i) {
    total_len += snprintf(NULL, 0, "%s;%s#", attribute_array[i].attr_name,
                          attribute_array[i].value);
  }

  char *result = malloc(total_len);

  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed!\n");
    exit(EXIT_FAILURE);
  }

  snprintf(result, header_len, "%d#%s#", type, service_name);

  for (int i = 0; i < num_attr; ++i) {
    strcat(result, attribute_array[i].attr_name);
    strcat(result, ";");
    strcat(result, attribute_array[i].value);
    strcat(result, "#");
  }

  return result;
}

char *create_ad_msg(char *service_name, char *ad_name, char *ad_value) {
  enum Msg_type type = AD;
  size_t len =
      snprintf(NULL, 0, "%d#%s#%s#%s#", type, service_name, ad_name, ad_value) +
      1;

  char *result = malloc(len);

  snprintf(result, len, "%d#%s#%s#%s#", type, service_name, ad_name, ad_value);

  return result;
}

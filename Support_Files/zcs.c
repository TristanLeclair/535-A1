#include "zcs.h"
#include "multicast.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define foreach(item, array)                                                   \
  for (int keep = 1, count = 0, size = sizeof(array) / sizeof(*(array));       \
       keep && count != size; keep = !keep, count++)                           \
    for (item = (array) + count; keep; keep = !keep)

enum Msg_type { NOTIFICATION, DISCOVERY, HEARTBEAT, AD };

typedef struct _zcs_node_t {
  // struct sockaddr_in address;
  char *name;
  int status; // 0 for down, 1 for up
  time_t hearbeat_time;
  zcs_cb_f cback;
  struct _zcs_node_t *next;
  zcs_attribute_t *attriutes[];
} zcs_node_t;

typedef struct _node_list_t {
  zcs_node_t *head;
  zcs_node_t *tail;
} node_list_t;

typedef struct ad_node {
  char *name;
  zcs_cb_f cback;
  struct ad_node *next;
} ad_node_t;

typedef struct ad_list {
  ad_node_t *head;
  ad_node_t *tail;
} ad_list_t;

typedef struct ad_notification {
  char *service_name;
  char *name;
  char *value;
} ad_notification_t;

mcast_t *m;
node_list_t *local_registry;
char *service_name;
zcs_attribute_t *attribute_array;
int num_attr;
ad_list_t *ad_list;
int STARTED = 0;
int INITIALIZED = 0;

// Global var to stop thread
int stopThread = 0;

// Methods

void add_node(zcs_node_t *node) {
  if (local_registry->head == NULL) {
    local_registry->head = node;
    local_registry->tail = node;
  } else {
    local_registry->tail->next = node;
    local_registry->tail = node;
  }
}

void add_ad_node(ad_node_t *node) {
  if (ad_list->head == NULL) {
    ad_list->head = node;
    ad_list->tail = node;
  } else {
    ad_list->tail->next = node;
    ad_list->tail = node;
  }
}

void copy_array(const zcs_attribute_t given_attributes[],
                zcs_attribute_t **local_attribute_array, int num) {
  *local_attribute_array = malloc(num * sizeof(zcs_attribute_t));

  if (*local_attribute_array == NULL) {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  memcpy(*local_attribute_array, given_attributes,
         num * sizeof(zcs_attribute_t));
}

// Deserialize the header of the message
int deserialize_header(char *msg) {
  int i = 0;
  char *header_str = (char *)malloc(sizeof(char) * 64);
  strcpy(header_str, "");
  int header;
  while (msg[i] != '#') {
    strcat(header_str, &msg[i]);
    i++;
  }
  header = atoi(header_str);
  return header;
}

char *deserialize_heartbeat(char *msg) {
  // Deserialize the message and get the name of the node
  char *name = (char *)malloc(sizeof(char) * 64);
  strcpy(name, "");
  int i = 0;
  int past_header = 0;
  // DOUBLE CHECK THAT SENDTO SENDS A MESSAGE WITH A NULL TETMINATOR
  while (msg[i] != '\0') {
    if (msg[i] == '#') {
      past_header = 1;
    }
    if (past_header == 1) {
      strcat(name, &msg[i]);
    }
    i++;
  }
  return name;
}

void deserialize_notification(char *msg, zcs_node_t *node) {
  int location_in_msg = 0;
  int i = 0;
  int reading_key = 0;
  int reading_value = 0;
  int reading_name = 0;
  // Start nume attributes at -1 so that the first attribute is at 0
  int num_attributes = -1;

  while (msg[i] != '\0') {

    // If the message is a # then we are at a new location in the message
    if (msg[i] == '#') {
      location_in_msg++;
      // If the location is 1 then we are reading the name of the node
      if (location_in_msg == 1) {
        reading_name = 1;
      }
      // After the second #, we are reading the key and value of the attributes
      else if (location_in_msg >= 2) {
        reading_name = 0;
        reading_key = 1;
        reading_value = 0;
        num_attributes++;
      }
      continue;
    } else if (msg[i] == ';') {
      reading_key = 0;
      reading_value = 1;
      continue;
    }

    if (reading_name == 1) {
      strcat(node->name, &msg[i]);
      continue;
    } else if (reading_key == 1) {
      strcpy(node->attriutes[num_attributes]->attr_name, &msg[i]);
    } else if (reading_value == 1) {
      strcpy(node->attriutes[num_attributes]->value, &msg[i]);
    }

    i++;
  }
}

void deserialize_ad(char *msg, ad_notification_t *ad) {
  int location_in_msg = 0;
  int i = 0;
  int reading_service_name = 0;
  int reading_name = 0;
  int reading_value = 0;

  while (msg[i] != '\0') {
    if (msg[i] == '#') {
      location_in_msg++;
    }
    if (location_in_msg == 1) {
      reading_service_name = 1;
      continue;
    } else if (location_in_msg == 2) {
      reading_service_name = 0;
      reading_name = 1;
      continue;
    } else if (location_in_msg == 3) {
      reading_name = 0;
      reading_value = 1;
      continue;
    }

    if (reading_service_name == 1) {
      strcat(ad->service_name, &msg[i]);
    } else if (reading_name == 1) {
      strcat(ad->name, &msg[i]);
    } else if (reading_value == 1) {
      strcat(ad->value, &msg[i]);
    }
  }
}

char *create_discovery_msg() {
  size_t len = snprintf(NULL, 0, "%d#", DISCOVERY) + 1;

  char *result = malloc(len);

  snprintf(result, len, "%d#", DISCOVERY);

  return result;
}

char *create_heartbeat_msg() {
  size_t len = snprintf(NULL, 0, "%d#%s#", HEARTBEAT, service_name) + 1;

  char *result = malloc(len);

  snprintf(result, len, "%d#%s#", HEARTBEAT, service_name);

  return result;
}

char *create_notification_msg() {
  enum Msg_type type = NOTIFICATION;
  size_t header_len = snprintf(NULL, 0, "%d#%s#", type, service_name);
  header_len += 1;
  size_t total_len = header_len;

  for (int i = 0; i < num_attr; ++i) {
    total_len += snprintf(NULL, 0, "%s;%s#", attribute_array[i].attr_name, attribute_array[i].value);
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

char *create_ad_msg(char *ad_name, char *ad_value) {
  enum Msg_type type = AD;
  size_t len =
      snprintf(NULL, 0, "%d#%s#%s#%s#", type, service_name, ad_name, ad_value) +
      1;

  char *result = malloc(len);

  snprintf(result, len, "%d#%s#%s#%s#", type, service_name, ad_name, ad_value);

  return result;
}

/*
  Help functions
*/

void* run_receive_service_message() {
  while (1) {
    // Check for messages
    int rc = multicast_check_receive(m);

    // If there is a message, then process it
    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      // Receive the message
      multicast_receive(m, msg, sizeof(msg));

      int header = deserialize_header(msg);

      // NOTIFICATION message
      if (header == NOTIFICATION) {
        int node_exists = 0;

        // Deserialize the message and get the name of the node and create a new
        // node in the local registry

        zcs_node_t *node = (zcs_node_t *)malloc(sizeof(zcs_node_t));
        deserialize_notification(msg, node);

        // Check if the node is in local_registry
        zcs_node_t *current = local_registry->head;

        while (current != NULL) {
          if (strcmp(current->name, node->name) == 0) {
            node_exists = 1;
            // Update the status of the node
            current->status = 1;
            // THis may not be necessary
            current->hearbeat_time = time(NULL);
          }
          current = current->next;
        }

        // If the node is not in the local_registry, then add it
        if (node_exists == 0) {
          node->status = 1;
          node->hearbeat_time = time(NULL);
          add_node(node);
        } else {
          // If the node already exists free the memory
          free(node);
        }
      }
      // AD message
      else if (header == AD) {
        //  Create new AD notification
        ad_notification_t *ad =
            (ad_notification_t *)malloc(sizeof(ad_notification_t));

        // Deserialize the message and get the name of the node
        deserialize_ad(msg, ad);

        zcs_node_t *current = local_registry->head;

        while (current != NULL) {
          if (strcmp(current->name, ad->service_name) == 0) {
            // Set the call back function of the node
            current->cback(ad->name, ad->value);
          }
          current = current->next;
        }

      } else if (header == HEARTBEAT) {

        // Deserialize the message and get the name of the node
        char *name = deserialize_heartbeat(msg);
        // Update the status of the node
        zcs_node_t *current = local_registry->head;
        while (current != NULL) {
          if (strcmp(current->name, name) == 0) {
            current->status = 1;
            current->hearbeat_time = time(NULL);
          }
          current = current->next;
        }
      }
    }
  }

  return 0;
}

/*
  Should this function return an int if for success or fail?
  Should the thread be stopped in this function?
*/
void* run_receive_discovery_message() {
  while (stopThread == 0) {
    // Continually check for DISCOVERY messages
    int rc = multicast_check_receive(m);
    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      multicast_receive(m, msg, sizeof(msg));

      int header = deserialize_header(msg);
      // If the incoming message is a DISCOVERY message, then send a
      // NOTIFICATION message
      if (header == DISCOVERY) {

        // char *notification = (char *)malloc(sizeof(char) * 1024);
        // serialize_notification(notification, local_registry->head);
        char *notification = create_notification_msg();
        int sent = multicast_send(m, notification, sizeof(notification));
        // if (sent < 0) {
        //   return -1;
        // }
      }
    }
  }
  return 0;
}

/*
  Should this function return an int if for success or fail?
  Should the thread be stopped in this function?
*/
void* run_send_heartbeat() {
  while (stopThread == 0) {
    sleep(3);
    // Continually send HEARTBEAT messages
    char *heartbeat = create_heartbeat_msg();
    int sent = multicast_send(m, heartbeat, sizeof(heartbeat));
    // if (sent < 0) {
    //   return -1;
    // }
  }
  return 0;
}

void* run_heartbeat_service() {
  // Check the heartbeat count of all the nodes every 5 seconds
  while (1) {
    sleep(6);
    zcs_node_t *current = local_registry->head;
    while (current != NULL) {
      // If the node heartbeat count doesn't equal the required heartbeat count,
      // then set the status to DOWN
      if ((time(NULL) - current->hearbeat_time) > 3) {
        current->status = 0;
      }
      current = current->next;
    }
  }
  return 0;
}

/*
This initializes the ZCS library. This library function must be called before
issuing any other calls to the library. It sets up the parameters and performs
the initializations necessary for the library to work.
Returns a 0 if the initialization was a success.
Otherwise, it returns a -1.
 *
 */
int zcs_init(int type) {

  pthread_t tid;

  // If the type is ZCS_APP_TYPE, then the node is an application
  if (type == ZCS_APP_TYPE) {
    m = multicast_init("239.1.1.1", 5000, 8080);

    if (m == NULL) {
      return -1;
    }

    // Support for receivig messages
    multicast_setup_recv(m);

    // Create thread to continually check for messages
    int result = pthread_create(&tid, NULL, run_receive_service_message, m);
    if (result != 0) {
      return -1;
    }

    int heartbeat_thread = pthread_create(&tid, NULL, run_heartbeat_service, m);

    // Send a DISCOVERY message to the network
    char *disc_msg = create_discovery_msg();
    int disc = multicast_send(m, disc_msg, sizeof(disc_msg));
    if (disc < 0) {
      return -1;
    }

  }
  // If the type is ZCS_SERVICE_TYPE, then the node is a discovery node
  else if (type == ZCS_SERVICE_TYPE) {
    m = multicast_init("239.1.1.1", 8080, 5000);

    if (m == NULL) {
      return -1;
    }
    printf("Service created\n");
  }

  INITIALIZED = 1;
  return 0;
}

/*
This call puts the node online. The node has a name, and it is mandatory. It is
an ASCII string without spaces that is NULL terminated. It can have a maximum
length of 64 characters including the NULL termination. In addition to the name,
the node can have optional attributes. In practice, a node would be started with
at least one attribute (e.g., the node type). The attributes are specified as
key-value pairs and would remain unchanged until the node shuts down. The key
and value fields are ASCII characters. If you want to include integers, floats,
or other data types in the attribute values, they must be converted to ASCII
strings before specified as attributes. The last parameter specifies the number
of attributes passed into the node. Returns a 0 if the node start was a success.
Otherwise, it returns a -1, which happens if the start was attempted before the
initialization was called.
 */
int zcs_start(char *name, zcs_attribute_t attr[], int num) {
  // Check if the node was initialized
  if (INITIALIZED == 0) {
    return -1;
  }
  printf("Service started\n");

  int i = 0;
  while (name[i] != '\0') {
    if (name[i] > 127 || name[i] < 0) {
      return -1;
    }
    i++;
  }
  if (i > 63) {
    return -1;
  }
  service_name = name;
  copy_array(attr, &attribute_array, num);

  // Set up message receiving
  multicast_setup_recv(m);

  // Send a NOTIFICATION message to the network
  char *notification = create_notification_msg();
  int sent = multicast_send(m, notification, sizeof(notification));
  if (sent < 0) {
    return -1;
  }

  pthread_t tid1;
  pthread_t tid2;

  // Create a thread to run receive_discovery_message
  int result = pthread_create(&tid1, NULL, run_receive_discovery_message, m);

  if (result != 0) {
    return -1;
  }

  // Create a thread to run send_heartbeat
  int result2 = pthread_create(&tid2, NULL, run_send_heartbeat, m);

  if (result2 != 0) {
    return -1;
  }

  STARTED = 1;
  return 0;
}

/*
This is used to post an advertisement with the given name and value.
Advertisements are different from attributes. While attributes are node
properties that are valid until the node goes down, advertisements are messages
broadcasted by the node as soon as the above function is executed by the node.
The advertisement duration and repeat attempts
are pre-set in the ZCS library. The node will attempt to deliver the
advertisements to other nodes in the network according to the duration and
repeat attempts. Returns the number of times the advertisement was posted on the
network. It will return 0 (no posting) to indicate an error condition. This will
happen if the posting was called before the node was started.
  */
int zcs_post_ad(char *ad_name, char *ad_value) {
  // Send an ADD message to the network

  // Needs a bit of work to repeat attempts
  char *ad_msg = create_ad_msg(ad_name, ad_value);
  int sent = multicast_send(m, ad_msg, sizeof(ad_msg));
  if (sent < 0) {
    return -1;
  }
  printf("Ad posted\n");
  return 0;
}

/*
This function is used to scan for nodes with a given value for a given
attribute. If no matching nodes are found, the call returns a 0. Otherwise, the
call returns the number of nodes found in the network. The names of the nodes
found are stored in the node_names. A call to zcs_query() can fail to find a
matching node if there are no nodes with matching attributes or the calling node
is not in the same network as the matching node.
*/
int zcs_query(char *attr_name, char *attr_value, char *node_names[],
              int namelen) {
  int i = 0;
  zcs_node_t *current = local_registry->head;
  while (current != NULL && i < namelen) {
    if (strcmp(current->attriutes[i]->attr_name, attr_name) == 0 &&
        strcmp(current->attriutes[i]->value, attr_value) == 0) {
      node_names[i] = current->name;
      i++;
    }
    current = current->next;
  }
  return i;
}

/*
This function is used to get the full list of attributes of a node that is
returned by the zcs_query() function. The first argument is the name of the
node. The second argument is an attribute array that is already allocated. The
third argument is set to the number of slots allocated in the attribute array.
The function sets it to the number of actual attributes read from the node. The
return value of the function is 0 if there is no error and is -1 if there is an
error.
*/
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num) {
  zcs_node_t *current = local_registry->head;
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      for (int i = 0; i < *num; i++) {
        attr[i] = *current->attriutes[i];
      }
      return 0;
    }
    current = current->next;
  }
  return -1;
}

/*
This function takes two arguments. The first is a name of the target node and
the second is the callback that will be triggered when the target posts an
advertisement. The callback has two arguments: name of the advertisement and the
value of the advertisement. There is no mechanism for un-listening to an
advertisement.
*/
int zcs_listen_ad(char *name, zcs_cb_f cback) {
  zcs_node_t *current = local_registry->head;
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      current->cback = cback;
      return 0;
    }
    current = current->next;
  }
  return -1;
}

/*
This function is called to terminate the activities of the ZCS by a program
before it terminates. The call returns a 0 if it is a success. Otherwise, it
will return a -1. For example, if the call to shutdown is made before the node
was started it will return a -1.
*/
int zcs_shutdown() {
  // Check if the node was started
  if (STARTED == 0) {
    return -1;
  }

  // Close the multicast socket
  multicast_destroy(m);

  // Set global var so that the receive thread can stop
  stopThread = 1;

  // Free memory
  free(local_registry);
  free(ad_list);

  printf("Service shut down\n");

  return 0;
}

/*
This function prints the node UP and DOWN logs. That is, every time a node fails
(goes down) the observing node makes a note of that event in its log. Similarly,
every time a node boots up (comes up) the observing node makes a note. This
function prints the log that is maintained at the local node. The log can be
tcloseruncated once it reaches a predefined length in size or time. There is no
return value for this function.
*/
void zcs_log() {
  zcs_node_t *current = local_registry->head;
  while (current != NULL) {
    if (current->status == 0) {
      printf("%s is DOWN\n", current->name);
    } else {
      printf("%s is UP\n", current->name);
    }
    current = current->next;
  }
}

#include "zcs.h"
#include "multicast.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define foreach(item, array)                                                   \
  for (int keep = 1, count = 0, size = sizeof(array) / sizeof(*(array));       \
       keep && count != size; keep = !keep, count++)                           \
    for (item = (array) + count; keep; keep = !keep)

typedef struct _zcs_node_t {
  struct sockaddr_in address;
  char *name;
  int status;
  struct _zcs_node_t *next;
  zcs_attribute_t *attriutes[];
} zcs_node_t;

typedef struct _node_list_t {
  zcs_node_t *head;
  zcs_node_t *tail;
} node_list_t;

mcast_t *m;
node_list_t *local_registry;

#define DISCOVERY "DISCOVERY"
#define NOTIFICATION "NOTIFICATION"
#define HEARTBEAT "HEARTBEAT"

void add_node(zcs_node_t *node) {
  if (local_registry->head == NULL) {
    local_registry->head = node;
    local_registry->tail = node;
  } else {
    local_registry->tail->next = node;
    local_registry->tail = node;
  }
}

/*
This initializes the ZCS library. This library function must be called before
issuing any other calls to the library. It sets up the parameters and performs
the initializations necessary for the library to work.
Returns a 0 if the initialization was a success.
Otherwise, it returns a -1.
 *
 */
int zcs_init() {
  m = multicast_init("239.1.1.1", 1234, 1235);
  local_registry = (node_list_t *)malloc(sizeof(node_list_t));

  if (m == NULL) {
    return -1;
  }

  multicast_setup_recv(m);

  int sent = multicast_send(m, DISCOVERY, sizeof(DISCOVERY));
  if (sent < 0) {
    return -1;
  }

  while (1) {
    int rc = multicast_check_receive(m);
    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      multicast_receive(m, msg, sizeof(msg));
      if (strcmp(msg, NOTIFICATION) == 0) {
        zcs_node_t *node = (zcs_node_t *)malloc(sizeof(zcs_node_t));
        node->address = m->my_addr;
      }
    }
  }

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

  zcs_node_t *node = (zcs_node_t *)malloc(sizeof(zcs_node_t));
  strcpy(node->name, name);

  for (i = 0; i < num; i++) {
    strcpy(node->attriutes[i]->attr_name, attr->attr_name);
    strcpy(node->attriutes[i]->value, attr->value);
  }

  int sent = multicast_send(m, NOTIFICATION, sizeof(NOTIFICATION));
  if (sent < 0) {
    return -1;
  }

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
int zcs_post_ad(char *ad_name, char *ad_value);

int zcs_query(char *attr_name, char *attr_value, char *node_names[]);

int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num);

int zcs_listen_ad(char *name, zcs_cb_f cback);

int zcs_shutdown();

void zcs_log();

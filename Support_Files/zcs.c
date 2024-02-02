#include "zcs.h"
#include "multicast.h"
#include "reveive_send.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define foreach(item, array)                                                   \
  for (int keep = 1, count = 0, size = sizeof(array) / sizeof(*(array));       \
       keep && count != size; keep = !keep, count++)                           \
    for (item = (array) + count; keep; keep = !keep)


mcast_t *m;
node_list_t *local_registry;
ad_list_t *ad_list;
int STARTED = 0;
int INITIALIZED = 0;


// Global var to stop thread
int stopThread = 0;

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

void add_ad_node(ad_node_t *node) {
  if (ad_list->head == NULL) {
    ad_list->head = node;
    ad_list->tail = node;
  } else {
    ad_list->tail->next = node;
    ad_list->tail = node;
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
    int disc = multicast_send(m, DISCOVERY, sizeof(DISCOVERY));
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


  }

  INTIALIZED = 1;
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

  // Set up message receiving
  multicast_setup_recv(m);

  // Send a NOTIFICATION message to the network
  int sent = multicast_send(m, NOTIFICATION, sizeof(NOTIFICATION));
  if (sent < 0) {
    return -1;
  }

  pthread_t tid1;
  pthread_t tid2;

  // Create a thread to run receive_discovery_message
  int result = pthread_create(&tid1, NULL, run_receive_discovery_message, m);

  // Create a thread to run send_heartbeat
  int result = pthread_create(&tid2, NULL, run_send_heartbeat, m);

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
  int sent = multicast_send(m, ADD, sizeof(ADD));
  if (sent < 0) {
    return -1;
  }
  return 0;
}


/*
This function is used to scan for nodes with a given value for a given attribute. 
If no matching nodes are found, the call returns a 0. Otherwise, the call returns 
the number of nodes found in the network. The names of the nodes found are stored 
in the node_names. A call to zcs_query() can fail to find a matching node if there 
are no nodes with matching attributes or the calling node is not in the same 
network as the matching node.
*/
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen) {
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
This function is used to get the full list of attributes of a node that is returned
by the zcs_query() function. The first argument is the name of the node. The second
argument is an attribute array that is already allocated. The third argument is set
to the number of slots allocated in the attribute array. The function sets it to the
number of actual attributes read from the node. The return value of the function is 0
if there is no error and is -1 if there is an error.
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
This function takes two arguments. The first is a name of the target node and the second
is the callback that will be triggered when the target posts an advertisement. The callback
has two arguments: name of the advertisement and the value of the advertisement. 
There is no mechanism for un-listening to an advertisement.
*/
int zcs_listen_ad(char *name, zcs_cb_f cback) {
  zcs_node_t *current = local_registry->head;
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      ad_node_t *ad = (ad_node_t *)malloc(sizeof(ad_node_t));
      add_ad_node(ad);
      return 0;
    }
    current = current->next;
  }
  return -1;

}


/*
This function is called to terminate the activities of the ZCS by a program before it
terminates. The call returns a 0 if it is a success. Otherwise, it will return a -1.
For example, if the call to shutdown is made before the node was started it will return a -1.
*/
int zcs_shutdown() {
  // Check if the node was started
  if (STARTED == 0) {
    return -1;
  }

  // Close the multicast socket
  multicast_close(m);
  
  // Set global var so that the receive thread can stop
  stopThread = 1;

  // Free memory
  free(local_registry);
  free(ad_list);

  return 0;

}

/*
This function prints the node UP and DOWN logs. That is, every time a node fails (goes down)
the observing node makes a note of that event in its log. Similarly, every time a node
boots up (comes up) the observing node makes a note. This function prints the log that is
maintained at the local node. The log can be truncated once it reaches a predefined length in
size or time. There is no return value for this function.
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

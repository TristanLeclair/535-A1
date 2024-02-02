#include "zcs.h"
#include "multicast.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void run_receive_service_message(mcast_t *m, node_list_t *local_registry, ad_list_t *ad_list){
  while (1) {
    // Check for messages
    int rc = multicast_check_receive(m);

    // If there is a message, then process it
    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      // Receive the message
      multicast_receive(m, msg, sizeof(msg));

      // Check the message type
      if (strcmp(msg, NOTIFICATION) == 0) {
        // Deserialize the message and get the name of the node and create a new node in the local registry
        zcs_node_t *node = (zcs_node_t *)malloc(sizeof(zcs_node_t));
        deserialize_notification(msg, node);
        add_node(node);
        node->status = 1;
        // This may be unnecessary
        node->hearbeat_time = time(NULL);

      }
      else if(strcmp(msg, ADD)){
        // Deserialize the message and get the name of the node

        // Check if the node is in the ad_list
        ad_node_t *current = ad_list->head;
        while (current != NULL) {
          if (strcmp(current->name, name) == 0) {
            // Set the call back function of the node
            current->cback(name, value);
          }
          current = current->next;
        }
      }
      else if(strcmp(msg, HEARTBEAT)){
        // Deserialize the message and get the name of the node

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

}

void run_receive_discovery_message(int stopThread, mcast_t *m, node_list_t *local_registry) {
  while(stopThread == 0){
    // Continually check for DISCOVERY messages
    int rc = multicast_check_receive(m);
    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      multicast_receive(m, msg, sizeof(msg));
      // If the incoming message is a DISCOVERY message, then send a NOTIFICATION message
      if (strcmp(msg, DISCOVERY) == 0) {
        char *notification = (char *)malloc(sizeof(char) * 1024);
        serialize_notification(notification, local_registry->head);
        int sent = multicast_send(m, notification, sizeof(notification));
        if (sent < 0) {
          return -1;
        }
      }
    }
  }
}

void run_send_heartbeat(int stopThread, mcast_t *m, char *msg) {
  while(stopThread == 0){
    sleep(3);
    // Continually send HEARTBEAT messages
    int sent = multicast_send(m, msg, sizeof(msg));
    if (sent < 0) {
      return -1;
    }
  }
}

void run_heartbeat_service(node_list_t *local_registry){
  // Check the heartbeat count of all the nodes every 5 seconds
  while(1){
    sleep(6);
    zcs_node_t *current = local_registry->head;
    while (current != NULL) {
      // If the node heartbeat count doesn't equal the required heartbeat count, then set the status to DOWN
      if ((time(NULL)  -current->hearbeat_time) > 3 ) {
        current->status = 0;
      }
      current = current->next;
    }
  }
}
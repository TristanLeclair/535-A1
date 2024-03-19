#include "multicast/multicast.h"
#include "networking/networking.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct mcast_pair_t {
  mcast_t *in;
  mcast_t *out;
  char source;
} mcast_pair;

/**
 * @brief Run a pair of mcasts to send from one LAN to another
 *
 * @param mcasts a pair of mcasts
 *
 */
void *run_relay(void *mcasts) {
  mcast_pair *pair = (mcast_pair *)mcasts;
  while (1) {
    int rc = multicast_check_receive(pair->in);

    if (rc > 0) {
      char msg[1024];
      multicast_receive(pair->in, msg, sizeof(msg));
      if (msg[0] == pair->source) {
        multicast_send(pair->out, msg, strlen(msg));
      }
      memset(msg, 0, sizeof(msg));
    }
  }
  return 0;
}

pthread_t start_relay(char source, char *rec_from_ip, char *send_to_ip,
                      int port_in, int port_out) {
  // mcast that will send to other LAN
  mcast_t *out = multicast_init(send_to_ip, port_out, PORT_TRASH);

  // mcast that will listen to messages sent in LAN
  mcast_t *in = multicast_init(rec_from_ip, PORT_TRASH, port_in);
  multicast_setup_recv(in);

  mcast_pair *pair = (mcast_pair *)malloc(sizeof(mcast_pair));

  pair->in = in;
  pair->out = out;
  pair->source = source;

  pthread_t tid;

  int result = pthread_create(&tid, NULL, run_relay, pair);
  if (result != 0) {
    printf("Failed to create thread");
    exit(EXIT_FAILURE);
  }

  return tid;
}

int main() {
  pthread_t tid1 = start_relay('A', LAN_IP_SERVICE_A, LAN_IP_SERVICE_B,
                               PORT_APP_REC_A, PORT_SERVICE_SEND_B);

  pthread_t tid2 = start_relay('B', LAN_IP_APP_B, LAN_IP_APP_A,
                               PORT_SERVICE_REC_B, PORT_APP_SEND_A);

  pthread_t tid3 = start_relay('B', LAN_IP_SERVICE_B, LAN_IP_SERVICE_A,
                               PORT_APP_REC_B, PORT_SERVICE_SEND_A);

  pthread_t tid4 = start_relay('A', LAN_IP_APP_A, LAN_IP_APP_B,
                               PORT_SERVICE_REC_A, PORT_APP_SEND_B);

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  pthread_join(tid4, NULL);
}

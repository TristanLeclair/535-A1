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
} mcast_pair;

void *run_relay(void *mcasts) {
  mcast_pair *pair = (mcast_pair *)mcasts;
  printf("Hello from thread\n");
  printf("Expecting from port %d\n", PORT_APP_REC_IN);
  fflush(stdout);
  while (1) {
    int rc = multicast_check_receive(pair->in);
    printf("Checking multicast");
    fflush(stdout);

    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      multicast_receive(pair->in, msg, 1024);
      printf("Caught message '%s'\n", msg);
      fflush(stdout);
      multicast_send(pair->out, msg, strlen(msg));
      free(msg);
    }
  }
}

int main() {
  mcast_t *m_app_rec_a =
      multicast_init(LAN_IP_APP_A, PORT_APP_REC_OUT, PORT_APP_REC_IN);
  mcast_t *m_app_rec_b =
      multicast_init(LAN_IP_SERVICE_B, PORT_APP_REC_IN, PORT_APP_REC_OUT);
  mcast_t *m_app_send_a =
      multicast_init(LAN_IP_APP_A, PORT_APP_SEND_OUT, PORT_APP_SEND_IN);
  mcast_t *m_app_send_b =
      multicast_init(LAN_IP_APP_B, PORT_APP_SEND_OUT, PORT_APP_SEND_IN);
  mcast_t *m_service_rec_a = multicast_init(
      LAN_IP_SERVICE_A, PORT_SERVICE_REC_OUT, PORT_SERVICE_REC_IN);
  mcast_t *m_service_rec_b = multicast_init(
      LAN_IP_SERVICE_B, PORT_SERVICE_REC_OUT, PORT_SERVICE_REC_IN);
  mcast_t *m_service_send_a =
      multicast_init(LAN_IP_SERVICE_A, PORT_APP_REC_OUT, PORT_APP_REC_IN);
  mcast_t *m_service_send_b = multicast_init(
      LAN_IP_SERVICE_B, PORT_SERVICE_SEND_OUT, PORT_SERVICE_SEND_IN);

  // A to B app means A app to B service
  mcast_pair *pair_a_to_b_app = (mcast_pair *)malloc(sizeof(mcast_pair));
  mcast_pair *pair_b_to_a_app = (mcast_pair *)malloc(sizeof(mcast_pair));
  mcast_pair *pair_a_to_b_service = (mcast_pair *)malloc(sizeof(mcast_pair));
  mcast_pair *pair_b_to_a_service = (mcast_pair *)malloc(sizeof(mcast_pair));

  pair_a_to_b_app->in = m_app_send_a;
  multicast_setup_recv(m_app_send_a);
  pair_a_to_b_app->out = m_service_rec_b;

  pair_b_to_a_app->in = m_app_send_b;
  multicast_setup_recv(m_app_send_b);
  pair_b_to_a_app->out = m_service_rec_a;

  pair_a_to_b_service->in = m_service_send_a;
  multicast_setup_recv(m_service_send_a);
  pair_a_to_b_service->out = m_app_rec_b;

  pair_b_to_a_service->in = m_service_send_b;
  multicast_setup_recv(m_service_send_b);
  pair_b_to_a_service->out = m_app_rec_a;

  pthread_t tid;

  printf("Pairing service a to b's input on ports %d and %d\n",
         PORT_SERVICE_SEND_OUT, PORT_APP_REC_IN);
  printf("Going from IP %s to IP %s\n", LAN_IP_SERVICE_A, LAN_IP_APP_B);

  printf("Starting thread...\n");
  fflush(stdout);

  int result = pthread_create(&tid, NULL, run_relay, pair_a_to_b_service);
  if (result != 0) {
    return -1;
  }

  pthread_join(tid, NULL);

  printf("Exited thread\n");
}

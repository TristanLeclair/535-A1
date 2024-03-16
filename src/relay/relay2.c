#include "multicast/multicast.h"
#include "networking/networking.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct mcast_pair_t {
  mcast_t *in;
  mcast_t *out;
} mcast_pair;

mcast_t *m_app_rec_b;
mcast_t *m_service_send_a;

void *run_relay() {
  // mcast_pair *pair = (mcast_pair *)mcasts;
  printf("Hello from thread\n");
  printf("Expecting from port %d\n", PORT_APP_REC_IN);
  fflush(stdout);
  while (1) {
    int rc = multicast_check_receive(m_service_send_a);
    printf("Checking multicast");
    fflush(stdout);

    if (rc > 0) {
      char *msg = (char *)malloc(sizeof(char) * 1024);
      multicast_receive(m_service_send_a, msg, 1024);
      printf("Caught message '%s'\n", msg);
      fflush(stdout);
      multicast_send(m_app_rec_b, msg, 1024);
      free(msg);
    }
  }
  return 0;
}

int main() {
  m_app_rec_b =
      multicast_init(LAN_IP_SERVICE_B, PORT_APP_REC_IN, PORT_SERVICE_SEND_IN);
  m_service_send_a =
      multicast_init(LAN_IP_SERVICE_A, PORT_APP_REC_OUT, PORT_APP_REC_IN);

  // mcast_pair *pair_a_to_b_service = (mcast_pair *)malloc(sizeof(mcast_pair));

  // pair_a_to_b_service->in = m_service_send_a;
  multicast_setup_recv(m_service_send_a);
  // pair_a_to_b_service->out = m_app_rec_b;

  pthread_t tid;

  printf("Pairing service a to b's input on ports %d and %d\n",
         PORT_SERVICE_SEND_OUT, PORT_APP_REC_IN);
  printf("Going from IP %s to IP %s\n", LAN_IP_SERVICE_A, LAN_IP_APP_B);

  printf("Starting thread...\n");
  fflush(stdout);

  int result = pthread_create(&tid, NULL, run_relay, m_app_rec_b);
  if (result != 0) {
    return -1;
  }

  pthread_join(tid, NULL);
}

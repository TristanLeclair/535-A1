#include "multicast/multicast.h"
#include "networking/networking.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// TODO: to be potentially used to pass mcasts into the thread runner
typedef struct mcast_pair_t {
  mcast_t *in;
  mcast_t *out;
} mcast_pair;

/**
 * @brief Run a pair of mcasts to send from one LAN to another
 *
 * @param mcasts a pair of mcasts
 *
 * TODO: In the real relay we'll have 4 of these threads running.
 * App LAN A -> Service LAN B
 * App LAN B -> Service LAN A DONE
 * App LAN A <- Service LAN B
 * App LAN B <- Service LAN A DONE
 */
void *run_relay(void *mcasts) {
  mcast_pair *pair = (mcast_pair *)mcasts;
  while (1) {
    int rc = multicast_check_receive(pair->in);
    // printf("Checking multicast");
    // fflush(stdout);

    if (rc > 0) {
      char msg[1024];
      multicast_receive(pair->in, msg, sizeof(msg));
      printf("Caught message '%s'\n", msg);
      fflush(stdout);
      multicast_send(pair->out, msg, strlen(msg));
      memset(msg, 0, sizeof(msg));
    }
  }
  return 0;
}

// Service to app
pthread_t start_relay_service(char *rec_from_ip, char *send_to_ip) {
  // mcast that will send to other LAN
  mcast_t *out =
      multicast_init(send_to_ip, PORT_SERVICE_SEND_OUT, PORT_SERVICE_SEND_IN);

  // mcast that will listen to messages sent in LAN
  mcast_t *in = multicast_init(rec_from_ip, PORT_APP_REC_OUT, PORT_APP_REC_IN);
  // WARN: DONT FORGET THIS (I SPENT 3 HOURS BASHING MY HEAD AGAINST A WALL)
  multicast_setup_recv(in);

  mcast_pair *pair = (mcast_pair *)malloc(sizeof(mcast_pair));

  pair->in = in;
  pair->out = out;

  pthread_t tid;

  int result = pthread_create(&tid, NULL, run_relay, pair);
  if (result != 0) {
    printf("Failed to create thread");
    exit(EXIT_FAILURE);
  }

  return tid;
}

// App to service
pthread_t start_relay_app(char *rec_from_ip, char *send_to_ip) {
  // mcast that will send to other LAN
  mcast_t *out =
      multicast_init(send_to_ip, PORT_APP_SEND_OUT, PORT_APP_SEND_IN);

  // mcast that will listen to messages sent in LAN
  mcast_t *in =
      multicast_init(rec_from_ip, PORT_SERVICE_REC_OUT, PORT_SERVICE_REC_IN);
  // WARN: DONT FORGET THIS (I SPENT 3 HOURS BASHING MY HEAD AGAINST A WALL)
  multicast_setup_recv(in);

  mcast_pair *pair = (mcast_pair *)malloc(sizeof(mcast_pair));

  pair->in = in;
  pair->out = out;

  pthread_t tid;

  // int result = pthread_create(&tid, NULL, run_relay, pair);
  int result = pthread_create(&tid, NULL, run_relay, pair);
  if (result != 0) {
    printf("Failed to create thread");
    exit(EXIT_FAILURE);
  }

  return tid;
}

int main() {
  pthread_t tid1 = start_relay_service(LAN_IP_SERVICE_A, LAN_IP_APP_B);

  pthread_t tid2 = start_relay_app(LAN_IP_APP_B, LAN_IP_SERVICE_A);

  pthread_t tid3 = start_relay_service(LAN_IP_SERVICE_B, LAN_IP_APP_A);

  pthread_t tid4 = start_relay_app(LAN_IP_APP_A, LAN_IP_SERVICE_B);

  // // mcast that will send to services in LAN A
  // mcast_t *m_service_rec_a =
  //     multicast_init(LAN_IP_APP_A, PORT_APP_SEND_OUT, PORT_APP_SEND_IN);
  //
  // // mcast that will listen to messages sent by apps in LAN B
  // mcast_t *m_app_send_b =
  //     multicast_init(LAN_IP_APP_B, PORT_SERVICE_REC_OUT,
  //     PORT_SERVICE_REC_IN);
  // // WARN: DONT FORGET THIS (I SPENT 3 HOURS BASHING MY HEAD AGAINST A WALL)
  // multicast_setup_recv(m_app_send_b);
  //
  // mcast_pair *pair_b_to_a_apps = (mcast_pair *)malloc(sizeof(mcast_pair));
  //
  // pair_b_to_a_apps->in = m_app_send_b;
  // pair_b_to_a_apps->out = m_service_rec_a;
  //
  // // mcast that will send to apps in LAN A
  // mcast_t *m_app_rec_a = multicast_init(LAN_IP_SERVICE_A,
  // PORT_SERVICE_SEND_OUT,
  //                                       PORT_SERVICE_SEND_IN);
  //
  // // mcast that will listen to messages sent by services in LAN B
  // mcast_t *m_service_send_b =
  //     multicast_init(LAN_IP_SERVICE_B, PORT_APP_REC_OUT, PORT_APP_REC_IN);
  // // WARN: DONT FORGET THIS (I SPENT 3 HOURS BASHING MY HEAD AGAINST A WALL)
  // multicast_setup_recv(m_service_send_b);
  //
  // mcast_pair *pair_b_to_a_service = (mcast_pair *)malloc(sizeof(mcast_pair));
  //
  // pair_b_to_a_service->in = m_service_send_b;
  // pair_b_to_a_service->out = m_app_rec_a;
  //
  // // mcast that will send to services in LAN B
  // mcast_t *m_service_rec_b =
  //     multicast_init(LAN_IP_APP_B, PORT_APP_SEND_OUT, PORT_APP_SEND_IN);
  //
  // // mcast that will listen to messages sent by apps in LAN B
  // mcast_t *m_app_send_a =
  //     multicast_init(LAN_IP_APP_A, PORT_SERVICE_REC_OUT,
  //     PORT_SERVICE_REC_IN);
  // // WARN: DONT FORGET THIS (I SPENT 3 HOURS BASHING MY HEAD AGAINST A WALL)
  // multicast_setup_recv(m_app_send_a);
  //
  // mcast_pair *pair_a_to_b_apps = (mcast_pair *)malloc(sizeof(mcast_pair));
  //
  // pair_a_to_b_apps->in = m_app_send_a;
  // pair_a_to_b_apps->out = m_service_rec_b;
  //
  // pthread_t tid1;
  // pthread_t tid2;
  // pthread_t tid3;
  // pthread_t tid4;

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  pthread_join(tid4, NULL);
}

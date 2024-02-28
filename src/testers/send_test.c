#include "multicast/multicast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buffer[100];

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("Usage: send_test send-to-port msg \n");
    exit(0);
  }

  int sport = atoi(argv[1]);
  printf("Send-to port: %d\n", sport);
  srand(0);
  int myport = sport + random() % sport;
  // int myport = 5000;
  printf("Receiving port: %d\n", myport);
  char *msg = argv[2];

  mcast_t *m = multicast_init("224.1.1.1", sport, myport);

  while (1) {
    sleep(1);
    printf("Sending.. \n");
    multicast_send(m, msg, strlen(msg));
  }
}

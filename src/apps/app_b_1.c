#include "zcs/zcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void hello(char *s, char *r) {
  printf("Ad received in app3: %s, with value: %s\n", s, r);
  zcs_log();
}

int main() {
  int rv;
  rv = zcs_init(ZCS_APP_TYPE, 1);
  if (rv != 0) {
    fprintf(stderr, "failed to init");
    exit(EXIT_FAILURE);
  }
  char *names[10];
  while (1) {
    int serv_len = zcs_query("type", "speaker", names, 10);
    for (int i = 0; i < serv_len; ++i) {
      zcs_attribute_t attrs[5];
      int anum = 5;
      rv = zcs_get_attribs(names[i], attrs, &anum);
      if ((strcmp(attrs[1].attr_name, "location") == 0) &&
          (strcmp(attrs[1].value, "basement") == 0)) {
        rv = zcs_listen_ad(names[i], hello);
      }
    }
  }
}

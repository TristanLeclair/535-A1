#include "../../../include/zcs/status.h"
#include <stdlib.h>

char *status_to_text(enum Status status) {
  switch (status) {
  case UP:
    return "UP";
  case DOWN:
    return "DOWN";
  default:
    exit(EXIT_FAILURE);
    return "NOT A STATUS";
  }
}

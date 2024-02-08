#include "../../../include/messages/messages.h"

int validate_message_type(int type) {
  return (type >= MIN_TYPE_NUMBER && type < MAX_MESSAGE_TYPE);
}

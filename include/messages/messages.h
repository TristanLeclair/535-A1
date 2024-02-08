#ifndef MESSAGES_H
#define MESSAGES_H

#define MIN_TYPE_NUMBER 1
enum Msg_type {
  NOTIFICATION = MIN_TYPE_NUMBER,
  DISCOVERY,
  HEARTBEAT,
  AD,

  // ALWAYS KEEP THIS LAST
  MAX_MESSAGE_TYPE
};

int validate_message_type(int type);

#endif

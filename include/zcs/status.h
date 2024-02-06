#ifndef STATUS_H
#define STATUS_H

enum Status { DOWN, UP };

char *status_to_text(enum Status status);

#endif

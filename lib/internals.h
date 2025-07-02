#ifndef INTERNALS_H
#define INTERNALS_H

/*
Global data definitions such as global contexts etc.
*/

#include "utils.h"

#include <zmq.h>

void crss_initialize(const char *fnpath);

void *crss_zmq_ctx(void);

#endif // INTERNALS_H

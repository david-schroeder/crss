#ifndef INTERNALS_H
#define INTERNALS_H

/*
Global data definitions such as global contexts etc.
*/

#include "utils.h"

#include <zmq.h>

/*
Initialize miscellaneous internals.

Creates a global ZMQ context, available via `crss_zmq_ctx()`.
@param fnpath Caller Function Identifier
*/
void crss_initialize(const char *fnpath);

/*
Get current global ZMQ context

@return Pointer to current globally active ZMQ context, `NULL` if not available.
*/
void *crss_zmq_ctx(void);

#endif // INTERNALS_H

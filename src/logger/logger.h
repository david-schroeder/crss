#ifndef LOGGER_H
#define LOGGER_H

#include <zmq.h>
#include "utils.h" // standard libs
#include "internals.h" // naming things is the second hardest thing in compsci
#include "settings.h"

/*
    # Complex Logger

    `logger.h` extends `util.h`'s logging functionality
    by providing an in-process routing network based on ZMQ.

    Really, the name "logger" is a misnomer here; this system really
    is more of a general-purpose message-passing framework, it just
    happens to be really good at logging - so when a message arrives
    with a certain prefix, the rest is counted as a generic log message
    and can be accepted by any subsystem that decides to.

    To achieve this, `run_logger()` first starts a ZMQ PUB socket at
    address `inproc://cmd-broadcast` which will contain the
    broadcast messages that are propagated by the logger. Various
    systems can intelligently subscribe to only the messages affecting
    them as messages are commands whose identifiers typically suffice
    to determine the responsible component(s) or system(s).

    The logger then starts a ZMQ ROUTER socket at address
    `inproc://logger-notify` which waits for `n_connections`
    REQ sockets to connect to it; each from a different program
    subsystem. These connections are short-lived and only serve to send
    a singular message; namely confirmation that the connecting thread
    has subscribed and is listening to the logger broadcast channel.

    After this, the function typically responsible for logging may
    open a REQ socket and connect to `inproc://logger-log` to send
    arbitrary messages. They are republished on the broadcast channel
    and can from there reach the desired endpoints.

    The function finally sets global LOG_SIMPLE to false.

    @param n_connections Number of connections to wait for
    @return Logger exit code
*/
int run_logger(int n_connections);

#endif // LOGGER_H

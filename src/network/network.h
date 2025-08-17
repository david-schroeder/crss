#ifndef NETWORK_H
#define NETWORK_H

#include "minecraft/network_utils.h"
#include "utils.h"
#include "internals.h"

#include "crypto.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <zmq.h>

#define MC_PVN_1_18_2 758

#define CTB_MIN_SIZE 8

typedef struct {
    mcsock_t *conn;
    pthread_t thread;
    void *pair_sock; /* Network master-side communication socket */
} mcclient_t;

/*
Client Pair Socket Protocol

Client pair sockets are bound to the endpoint inproc://network-client-(n), where
(n) is the client's client ID.
Messages typically occur in a request-reply format, going from the client to the
network master. Messages are prefixed with a 4-byte length and an 8-byte operation
identifier (in practice a (meaningful) string). The length specifies the number of
bytes of the rest of the packet, not including the operation identifier.
*/

int run_network_master(char *ip, int port);

#endif // NETWORK_H

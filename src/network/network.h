#ifndef NETWORK_H
#define NETWORK_H

#include "minecraft/network_utils.h"
#include "utils.h"
#include "internals.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#define MC_PVN_1_18_2 758

int run_network_master(char *ip, int port);

#endif // NETWORK_H

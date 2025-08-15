#ifndef HANDLER_758_H
#define HANDLER_758_H

/*
Minecraft protocol handler for PVN 758 (Minecraft 1.18.2).
*/

#include "utils.h"
#include "resources.h"
#include "../../minecraft/network_utils.h"

/*
Communicate with client until end of exchange (EOX) upon a transition to the status state.
@param conn MCSock object representing the connection to the client. The underlying file descriptor is closed and `conn` is freed automatically.
*/
void *handle_status_758(mcsock_t *conn);

/*
Communicate with client until end of exchange (EOX) upon a transition to the play state.
@param conn MCSock object representing the connection to the client. The underlying file descriptor is closed and `conn` is freed automatically.
*/
void *handle_play_758(mcsock_t *conn);

#endif // HANDLER_758_H

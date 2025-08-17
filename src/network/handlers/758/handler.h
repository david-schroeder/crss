#ifndef HANDLER_758_H
#define HANDLER_758_H

/*
Minecraft protocol handler for PVN 758 (Minecraft 1.18.2).
*/

#include "utils.h"
#include "resources.h"
#include "ctconfig.h"
#include "../../minecraft/network_utils.h"
#include "../../crypto.h"
#include "../../../core/coretypes.h"

#include <zmq.h>

/* Packet IDs for Login state */

// S->C
#define PACKID_S2C_DISCONNECT_LOGIN (0x00)
#define PACKID_S2C_ENCRYPTION_REQUEST (0x01)
#define PACKID_S2C_LOGIN_SUCCESS (0x02)
#define PACKID_S2C_SET_COMPRESSION (0x03)

// C->S
#define PACKID_C2S_LOGIN_START (0x00)
#define PACKID_C2S_ENCRYPTION_RESPONSE (0x01)

/* Packet IDs for Play state */

// S->C
#define PACKID_S2C_JOIN_GAME (0x26)

// C->S

/*
Communicate with client until end of exchange (EOX) upon a transition to the status state.
@param conn MCSock object representing the connection to the client. The underlying file descriptor is closed automatically.
*/
void *handle_status_758(mcsock_t *conn);

/*
Communicate with client until end of exchange (EOX) upon a transition to the play state.
@param conn MCSock object representing the connection to the client. The underlying file descriptor is closed automatically.
*/
void *handle_play_758(mcsock_t *conn);

#endif // HANDLER_758_H

#ifndef HANDLER_758_H
#define HANDLER_758_H

/*
Minecraft protocol handler for PVN 758 (Minecraft 1.18.2).
*/

#include "utils.h"
#include "resources.h"
#include "ctconfig.h"
#include "settings.h"
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
#define PACKID_S2C_PLUGIN_MESSAGE (0x18)
#define PACKID_S2C_DISCONNECT_PLAY (0x1A)
#define PACKID_S2C_JOIN_GAME (0x26)
#define PACKID_S2C_PLAYER_INFO (0x36)
#define PACKID_S2C_UPDATE_PLAYER_POS_AND_LOOK (0x38)
#define PACKID_S2C_HELD_ITEM_CHANGE (0x48)

// C->S
#define PACKID_C2S_TELEPORT_CONFIRM (0x00)
#define PACKID_C2S_CHAT_MESSAGE (0x03)
#define PACKID_C2S_CLIENT_STATUS (0x04)
#define PACKID_C2S_CLIENT_SETTINGS (0x05)
#define PACKID_C2S_TAB_COMPLETE (0x06)
#define PACKID_C2S_CLICK_WINDOW_BUTTON (0x07)
#define PACKID_C2S_CLICK_WINDOW (0x08)
#define PACKID_C2S_CLOSE_WINDOW (0x09)
#define PACKID_C2S_PLUGIN_MESSAGE (0x0A)
#define PACKID_C2S_EDIT_BOOK (0x0B)
#define PACKID_C2S_QUERY_ENTITY_NBT (0x0C)
#define PACKID_C2S_KEEP_ALIVE (0x0F)
#define PACKID_C2S_PLAYER_POSITION (0x11)
#define PACKID_C2S_PLAYER_POS_AND_ROT (0x12)
#define PACKID_C2S_PLAYER_ROTATION (0x13)
#define PACKID_C2S_PLAYER_MOVEMENT (0x14)
#define PACKID_C2S_PICK_ITEM (0x17)
#define PACKID_C2S_PLAYER_ABILITIES (0x19)
#define PACKID_C2S_PLAYER_DIGGING (0x1A)
#define PACKID_C2S_ENTITY_ACTION (0x1B)
#define PACKID_C2S_PONG (0x1D)
#define PACKID_C2S_HELD_ITEM_CHANGE (0x25)
#define PACKID_C2S_CREATIVE_INV_ACTION (0x28)
#define PACKID_C2S_UPDATE_SIGN (0x2B)
#define PACKID_C2S_ANIMATION (0x2C)
#define PACKID_C2S_BLOCK_PLACEMENT (0x2E)
#define PACKID_C2S_USE_ITEM (0x2F)

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

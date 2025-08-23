#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include "utils.h"
#include "../../core/coretypes.h"
#include "../crypto.h"

#include <curl/curl.h>
#include <zmq.h>

/*
Packet usage

Packets structs are created after being received from a socket.
They contain a packet ID, data length, an internal buffer as well as a pointer to the
next byte to be processed within the buffer. This pointer is moved along as data
(fields) are consumed from the packet buffer, allowing for a very simple packet reading
interface.

The point of using packet structs over reading directly from the socket is that the
socket can operate in various compression and encryption modes, which an implementation
reading directly from it would have to account for whilst this struct-based
implementation can rather preprocess the packet into a "normalized" (uncompressed and
unencrypted) format before further use.


The flow for sending is as follows:
1. Packet initialization via `new_packet()`
2. Adding data to packet via `mcsock_write_*` methods
3. Sending via `dispatch_packet()`

When sending, the packet_t object containing the response packet has fields whose meaning
slightly differs from when receiving. Specifically, the data_length pointer is an
indicator of the size of the data buffer, whilst data_ptr holds the number of bytes in said buffer that are actually used. If a write operation requires more bytes than are left in the buffer, the buffer size is doubled by reallocation. 
*/

typedef struct {
    int32_t packet_id;
    uint32_t data_length;
    uint32_t data_ptr;
    uint8_t *data;
} packet_t;

/*
Create a new packet for writing.

@param init_buf_size Initial size of the data buffer. Must be a power of two.
*/
packet_t *new_packet(uint32_t init_buf_size);

void free_packet(packet_t *pack);

typedef struct {
    uint32_t length;
    char *data;
} mcstring_t;

mcstring_t *new_mcstring(char *s);

void free_mcstring(mcstring_t *string);

char *mcstring_to_cstring(mcstring_t *s);

typedef struct {
    char operation[9];
    char *data;
} pairsock_op_t;

typedef enum {
    CL_HANDSHAKING,
    CL_STATUS,
    CL_LOGIN,
    CL_PLAY,
    CL_TERMINATED
} mcclient_state_e;

/* Data about a connection and the connected client */
typedef struct {
    int fd;
    bool compressed;
    bool encrypted;
    int client_id;
    mcclient_state_e state;
    char *username;
    keypair_t *server_keypair;
    decrypted_buffer_t *shared_secret; // Buffer with length
    EVP_CIPHER_CTX *s2c_ctx;
    EVP_CIPHER_CTX *c2s_ctx;
    client_data_t client_data;
} mcsock_t;

pairsock_op_t *client_pair_recv(void *sock);

pairsock_op_t *client_pair_recv_blocking(void *sock);

void client_pair_send(void *sock, char *operation, char *data);

packet_t *packet_recv(mcsock_t *s);

void packet_send(packet_t *p, mcsock_t *s);

bool mcsock_read_bool(packet_t *s);

int8_t mcsock_read_byte(packet_t *s);

uint8_t mcsock_read_ubyte(packet_t *s);

int16_t mcsock_read_short(packet_t *s);

uint16_t mcsock_read_ushort(packet_t *s);

int32_t mcsock_read_int(packet_t *s);

int64_t mcsock_read_long(packet_t *s);

float mcsock_read_float(packet_t *s);

double mcsock_read_double(packet_t *s);

mcstring_t *mcsock_read_string(packet_t *s);

mcstring_t *mcsock_read_identifier(packet_t *s);

int32_t mcsock_read_varint(packet_t *s);

int64_t mcsock_read_varlong(packet_t *s);

uint8_t *mcsock_read_byte_array(packet_t *s, uint32_t len);

void mcsock_write_bool(packet_t *p, bool value);

void mcsock_write_byte(packet_t *p, int8_t value);

void mcsock_write_ubyte(packet_t *p, uint8_t value);

void mcsock_write_short(packet_t *p, int16_t value);

void mcsock_write_ushort(packet_t *p, uint16_t value);

void mcsock_write_int(packet_t *p, int32_t value);

void mcsock_write_long(packet_t *p, int64_t value);

void mcsock_write_float(packet_t *p, float value);

void mcsock_write_double(packet_t *p, double value);

void mcsock_write_string(packet_t *p, mcstring_t *string);

/* Same as `mcsock_write_string`, but works with a standard `char*` */
void mcsock_write_c_string(packet_t *p, char *str);

void mcsock_write_identifier(packet_t *p, mcstring_t *identifier);

void mcsock_write_varint(packet_t *p, int32_t value);

void mcsock_write_varlong(packet_t *p, int64_t value);

void mcsock_write_uuid(packet_t *p, uuid_t uuid);

void mcsock_write_byte_array(packet_t *p, uint8_t *array, uint32_t len);

void mcsock_write_nbt_compound(packet_t *p, nbt_node_t *n);

/*
Generate MC-style Hex Digest String from SHA1 hex digest
Overwrites data in `digest` iff MSB of digest is set
*/
mcstring_t *gen_mc_hexdigest(uint8_t *digest, uint32_t len);

mcstring_t *get_request(char *url);

#define SEND_PACKET_FUNCTION_WRAPPER(packid, default_size, inner) \
    packet_t *pack = new_packet(default_size); \
    pack->packet_id = packid; \
    inner \
    packet_send(pack, conn); \
    free_packet(pack);

#define ADD_FIELD_BOOL(name) mcsock_write_bool(pack, name)
#define ADD_FIELD_BYTE(name) mcsock_write_byte(pack, name)
#define ADD_FIELD_UBYTE(name) mcsock_write_ubyte(pack, name)
#define ADD_FIELD_SHORT(name) mcsock_write_short(pack, name)
#define ADD_FIELD_USHORT(name) mcsock_write_ushort(pack, name)
#define ADD_FIELD_INT(name) mcsock_write_int(pack, name)
#define ADD_FIELD_LONG(name) mcsock_write_long(pack, name)
#define ADD_FIELD_FLOAT(name) mcsock_write_float(pack, name)
#define ADD_FIELD_DOUBLE(name) mcsock_write_double(pack, name)
#define ADD_FIELD_CSTRING(name) mcsock_write_c_string(pack, name)
#define ADD_FIELD_VARINT(name) mcsock_write_varint(pack, name)
#define ADD_FIELD_VARLONG(name) mcsock_write_varlong(pack, name)
#define ADD_FIELD_BYTE_ARR(name, len) mcsock_write_byte_array(pack, name, len)
#define ADD_FIELD_UUID(name) mcsock_write_uuid(pack, name)
#define ADD_FIELD_NBT(name) mcsock_write_nbt_compound(pack, name)

#endif // NETWORK_UTILS_H

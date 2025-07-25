#include "internals.h"

static void *_crss_zmq_ctx = NULL;

void crss_initialize(const char *fnpath) {
    FUNCPATH("internals.init");
    _crss_zmq_ctx = zmq_ctx_new();
    if (!_crss_zmq_ctx) {
        LFATAL("Failed to create ZMQ Context!");
        exit(1);
    }
    free((void*)fnpath);
}

void *crss_zmq_ctx(void) {
    return _crss_zmq_ctx;
}

char *s_recv(void *socket, size_t nbytes) {
    char *buffer = malloc(nbytes);
    int received_size = zmq_recv(socket, buffer, nbytes - 1, 0);
    if (received_size == -1) {
        free(buffer);
        return NULL;
    }
    buffer[received_size < nbytes ? received_size : nbytes - 1] = '\0';
    return buffer;
}

int s_send(void *socket, char *string) {
    return zmq_send(socket, string, strlen(string), 0);
}

void dispatch_command(char *cmd) {
    void *ctx = crss_zmq_ctx();
    void *req = zmq_socket(ctx, ZMQ_REQ);
    zmq_connect(req, "inproc://run-cmd");
    s_send(req, cmd);
    zmq_close(req);
}

void free_router_msg(router_msg_t *msg) {
    free(msg->address);
    free(msg->addr_string);
    free(msg->data);
    free(msg);
}

static char nibble_to_char(uint8_t nibble) {
    if (nibble < 10) return '0' + nibble;
    if (nibble < 16) return 'A' + (nibble - 10);
    return 'X';
}

static char *addr_to_string(size_t nbytes, char *data) {
    char *addr_str = malloc(nbytes * 2 + 3);
    addr_str[0] = '0';
    addr_str[1] = 'x';
    addr_str[nbytes * 2 + 2] = '\0';
    for (size_t i = 0; i < nbytes; i++) {
        uint8_t byte = (uint8_t)data[i];
        uint8_t upper_nibble = byte >> 4;
        uint8_t lower_nibble = byte & 0xF;
        addr_str[2*i+2] = nibble_to_char(upper_nibble);
        addr_str[2*i+3] = nibble_to_char(lower_nibble);
    }
    return addr_str;
}

router_msg_t *s_recv_msg_router(void *socket) {
    router_msg_t *msg = malloc(sizeof(router_msg_t));
    zmq_msg_t message;
    int more = 1;
    char *msg_data = NULL;
    char *msg_address = NULL;
    int frameid = 0;
    while (more) {
        zmq_msg_init(&message);
        zmq_msg_recv(&message, socket, 0);
        more = zmq_msg_more(&message);

        if (frameid == 0) { // address frame
            size_t sz = zmq_msg_size(&message);
            msg_address = malloc(sz+1);
            char *data = zmq_msg_data(&message);
            memcpy(msg_address, data, sz);
            msg_address[sz] = '\0';
            msg->addr_string = addr_to_string(sz, data);
        } else if (frameid == 2) { // data frame after null frame
            size_t sz = zmq_msg_size(&message);
            msg_data = malloc(sz+1);
            char *data = zmq_msg_data(&message);
            memcpy(msg_data, data, sz);
            msg_data[sz] = '\0';
        }
        frameid++;

        zmq_msg_close(&message);
    }
    if (msg_data == NULL) {
        DLWARN("Received less than three frames on a ZMQ router message. This may be an error.");
        return calloc(1, sizeof(char));
    }
    msg->data = msg_data;
    msg->address = msg_address;
    return msg;
}

int logger_notify(char *name) {
    void *ctx = crss_zmq_ctx();

    void *req = zmq_socket(ctx, ZMQ_REQ);
    zmq_connect(req, "inproc://logger-notify");

    int ret = s_send(req, name);

    zmq_close(req);

    return ret;
}

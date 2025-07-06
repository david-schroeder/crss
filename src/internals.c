#include "internals.h"

static void *_crss_zmq_ctx = NULL;

void crss_initialize(const char *fnpath) {
    FUNCPATH("internals.init");
    _crss_zmq_ctx = zmq_ctx_new();
    if (!_crss_zmq_ctx) {
        LFATAL("Failed to create ZMQ Context!");
        exit(1);
    }
}

void *crss_zmq_ctx(void) {
    return _crss_zmq_ctx;
}

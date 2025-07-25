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

/*
Receive bytes from ZMQ socket

@param socket ZMQ socket to receive from
@param nbytes Maximum number of bytes to receive
@return Received data, must be freed
*/
char *s_recv(void *socket, size_t nbytes);

typedef struct {
    char *address;
    char *addr_string;
    char *data;
} router_msg_t;

void free_router_msg(router_msg_t *msg);

/*
Receive multipart ZMQ message from ZMQ socket

@param socket ZMQ socket to receive from
@return Received data and address, must be freed
*/
router_msg_t *s_recv_msg_router(void *socket);

/*
Send bytes to ZMQ socket

@param socket ZMQ socket to send to
@param string Data to send
@return Number of sent bytes
*/
int s_send(void *socket, char *string);

/*
Dispatch a command to subsystems

@param cmd Command to broadcast
*/
void dispatch_command(char *cmd);

/*
Notify logger of subsystem startup.

Sends a packet containing only `name` to `inproc://logger-notify`.
@param name Name of subsystem
@return Number of bytes sent
*/
int logger_notify(char *name);

// all for one fancy line of text...
#define THREAD_WRAPPER(name, compound) \
    const char *init_fp = (const char *)fp; \
    const char *fnpath = name "_bootstrap"; \
    LVERBOSE("Launching " name " subsystem..."); \
    fnpath = init_fp; \
    compound \
    return NULL

/*
Declare a wrapper function for a new thread.

@param name Name of the subsystem being launched
@param compound Function body as compound statement
*/
#define DECLARE_THREAD_WRAPPER(name, compound) \
    static void *name ## _wrapper(void *fp) { \
        THREAD_WRAPPER(#name, compound); \
    }

#define _RUN_WRAPPED_THREAD(name) \
    pthread_create(&name ## _thread, NULL, name ## _wrapper, (void *)fnpath)

/*
Run the function `<name>_wrapper` in a new thread, and store the resulting `pthread_t` object in `<name>_thread`
*/
#define LAUNCH_WRAPPED_THREAD(name) \
    pthread_t name ## _thread; \
    _RUN_WRAPPED_THREAD(name)
    

#define JOIN_WRAPPED_THREAD(name) \
    pthread_join(name ## _thread, NULL)

/*
Connect to internal command broadcast socket

*/
#define CONNECT_TO_CMD_BROADCAST() \
    void *__cmd_sub_sock = zmq_socket(crss_zmq_ctx(), ZMQ_SUB); \
    LVERBOSE("Subscribing to command broadcast..."); \
    zmq_connect(__cmd_sub_sock, "inproc://cmd-broadcast")

/*
Subscribe to specific command prefix

@param cmd Command to subscribe to
*/
#define SUBSCRIBE_TO_CMD(cmd) \
    zmq_setsockopt(__cmd_sub_sock, ZMQ_SUBSCRIBE, cmd, strlen(cmd))

/*
Run command handler

Polls broadcast socket for incoming commands in a loop.

@param handler_code Compound statement to handle commands
*/
#define RUN_CMD_HANDLER(loop_code, handler_code) \
    zmq_pollitem_t __cmd_handler_pollitems[] = { \
        { __cmd_sub_sock, 0, ZMQ_POLLIN, 0 } \
    }; \
    bool __cmd_handler_should_close = false; \
    while (!__cmd_handler_should_close) { \
        loop_code \
        zmq_poll(__cmd_handler_pollitems, 1, 0); \
        if (__cmd_handler_pollitems[0].revents & ZMQ_POLLIN) { \
            char *__cmd_handler_cmd_data = s_recv(__cmd_sub_sock, MAX_CMD_LEN); \
            struct charlist_ *__cmd_handler_cmd_args = str_split(__cmd_handler_cmd_data); \
            handler_code \
            free(__cmd_handler_cmd_data); \
            free_charlist(__cmd_handler_cmd_args); \
        } \
    }

#define RECEIVED_CMD (__cmd_handler_cmd_data)
#define RECEIVED_CMD_ARGC (__cmd_handler_cmd_args->length)
#define RECEIVED_CMD_ARGV (__cmd_handler_cmd_args->list)

/*
Exit command handler, effectively making this command handler iteration the last
*/
#define EXIT_CMD_HANDLER() \
    __cmd_handler_should_close = true;

/*
Execute `handler` if current command starts with `command`.
Only to be used in `RUN_CMD_HANDLER`.
*/
#define HANDLE_COMMAND(command, handler) \
    if (strncmp(command, __cmd_handler_cmd_data, strlen(command)) == 0) handler

#define CMD_HANDLER_CLEANUP() \
    zmq_close(__cmd_sub_sock)

#endif // INTERNALS_H

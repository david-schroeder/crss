#include "logger.h"

int run_logger(int n_connections) {
    const char *fnpath = "logger";

    void *ctx = crss_zmq_ctx();

    LDEBUG("Setting up sockets...");

    LVERBOSE("Creating broadcast socket...");
    void *publisher = zmq_socket(ctx, ZMQ_PUB);
    zmq_bind(publisher, "inproc://cmd-broadcast");

    LVERBOSE("Creating notification socket...");
    void *notifier = zmq_socket(ctx, ZMQ_ROUTER);
    zmq_bind(notifier, "inproc://logger-notify");

    LDEBUG("Waiting for subsystems to connect!");

    zmq_pollitem_t items[] = {
        { notifier, 0, ZMQ_POLLIN, 0 }
    };

    while (n_connections) {
        zmq_poll(items, 1, -1);
        if (items[0].revents & ZMQ_POLLIN) {
            router_msg_t *notifier_msg = s_recv_msg_router(notifier);

            LDEBUG("Subsystem %s connected! [ID %s]", notifier_msg->data, notifier_msg->addr_string);

            free_router_msg(notifier_msg);
            
            n_connections--;
        }
    }

    LDEBUG("Switching to complex logger!");
    switch_logger_protocol();

    void *log_sink = zmq_socket(ctx, ZMQ_ROUTER);
    zmq_bind(log_sink, "inproc://logger-log");

    void *cmd_sink = zmq_socket(ctx, ZMQ_ROUTER);
    zmq_bind(cmd_sink, "inproc://run-cmd");

    void *recv_cmd = zmq_socket(ctx, ZMQ_SUB);
    zmq_connect(recv_cmd, "inproc://cmd-broadcast");
    zmq_setsockopt(recv_cmd, ZMQ_SUBSCRIBE, "exit", 4);
    zmq_setsockopt(recv_cmd, ZMQ_SUBSCRIBE, "quit", 4);

    zmq_pollitem_t sink_items[] = {
        { log_sink, 0, ZMQ_POLLIN, 0 },
        { cmd_sink, 0, ZMQ_POLLIN, 0 },
        { recv_cmd, 0, ZMQ_POLLIN, 0 }
    };

    bool shouldClose = false;
    while (!shouldClose) {
        zmq_poll(sink_items, 3, -1);
        if (sink_items[0].revents & ZMQ_POLLIN) {
            router_msg_t *log_msg = s_recv_msg_router(log_sink);
            printf("\r\033[31m*\033[0m%s", log_msg->data);
            fflush(stdout);
            free_router_msg(log_msg);
        }
        if (sink_items[1].revents & ZMQ_POLLIN) {
            router_msg_t *cmd_msg = s_recv_msg_router(cmd_sink);
            char *cmd_data = cmd_msg->data;
            if (cmd_data[0] != '\0') {
                DLVERBOSE("Application Command issued: %s", cmd_data);
                s_send(publisher, cmd_data);
            }
            free_router_msg(cmd_msg);
        }
        if (sink_items[2].revents & ZMQ_POLLIN) {
            switch_logger_protocol();
            LVERBOSE("Exiting complex logger!");
            shouldClose = true;
        }
    }

    zmq_close(publisher);
    zmq_close(notifier);
    zmq_close(log_sink);
    zmq_close(cmd_sink);
    zmq_close(recv_cmd);

    return 0;
}

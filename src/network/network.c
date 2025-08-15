#include "network.h"

/* Protocol Handlers */
#include "handlers/758/handler.h"

int run_network_master(char *ip, int port) {
    char *fnpath = "network";

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        LFATAL("In socket");
        return EXIT_FAILURE;
    }

    /* Make socket non-blocking */
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    /* Set SO_REUSEADDR */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        LFATAL("Failed to set socket option SO_REUSEADDR");
    }

    /* Binding */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // TODO: IP
    address.sin_port = htons(port);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        LFATAL("Failed to bind!");
        return EXIT_FAILURE;
    }

    /* Listen for new connections */
    if (listen(server_fd, 10) < 0)
    {
        LFATAL("In listen");
        return EXIT_FAILURE;
    }

    LVERBOSE("Setting up sockets...");

    CONNECT_TO_CMD_BROADCAST();
    SUBSCRIBE_TO_CMD("network");
    SUBSCRIBE_TO_CMD("exit");
    SUBSCRIBE_TO_CMD("quit");

    LVERBOSE("Notifying Logger...");
    logger_notify("Network Master");

    int exit_code = 0;

    LVERBOSE("Entering main loop...");
    RUN_CMD_HANDLER({
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (errno != EWOULDBLOCK) {
                LFATAL("Critical error while accepting [%d]!", errno);
                EXIT_CMD_HANDLER();
                exit_code = -1;
            }
        } else {
            /* Successfully accepted new connection */
            LINFO("Accepted new connection!");

            mcsock_t *conn = malloc(sizeof(mcsock_t));
            conn->fd = new_socket;
            conn->compressed = false;
            conn->encrypted = false;

            packet_t *pack = packet_recv(conn);

            //LVERBOSE("Received packet %02x [Length %08x/%d]!", pack->packet_id, pack->data_length, pack->data_length);

            if (pack->packet_id == 0x7a) {
                /* Legacy Server List Ping */
                LWARN("Client sent Legacy Server List Ping!");
                free_packet(pack);
                close(new_socket);
                free(conn);
            } else if (pack->packet_id == 0) {
                /* Handshake */
                int32_t pvn = mcsock_read_varint(pack);
                mcstring_t *srv_addr = mcsock_read_string(pack);
                uint16_t srv_port = mcsock_read_ushort(pack);
                int32_t next_state = mcsock_read_varint(pack);
                bool is_status = next_state == 1;
                bool is_play = next_state == 2;

                LDEBUG("Received handshake! [PVN=%d ADDR='%s' PORT=%u NS=%d]", pvn, srv_addr->data, srv_port, next_state);

                free_mcstring(srv_addr);
                free_packet(pack);

                switch (pvn) {
                    case MC_PVN_1_18_2:
                    case -1: // Client is determining what version to use
                        // TODO: threading (also ZMQ pair socket management)
                        if (is_status) handle_status_758(conn);
                        if (is_play) handle_play_758(conn);
                        break;
                    default:
                        LWARN("Invalid PVN %d!", pvn);
                        close(new_socket);
                        free(conn);
                }
            } else {
                LWARN("Invalid handshaking packet ID %02x!", pack->packet_id);
                free_packet(pack);
                close(new_socket);
                free(conn);
            }
        }
    }, {
        HANDLE_COMMAND("exit", {
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            EXIT_CMD_HANDLER();
        })
    })

    LINFO("Freeing socket!!!!");
    CMD_HANDLER_CLEANUP();

    return exit_code;
}

#include "network.h"

/* Protocol Handlers */
#include "handlers/758/handler.h"

static void *dispatch(mcsock_t *conn) {
    char *fnpath = "network.dispatch";
    packet_t *pack = packet_recv(conn);

    //LVERBOSE("Received packet %02x [Length %08x/%d]!", pack->packet_id, pack->data_length, pack->data_length);

    if (pack && pack->packet_id == 0x7a) {
        /* Legacy SLP */
        LWARN("Client sent Legacy Server List Ping!");
        free_packet(pack);
        close(conn->fd);
    } else if (pack && pack->packet_id == 0) {
        /* Handshake */
        int32_t pvn = mcsock_read_varint(pack);
        mcstring_t *srv_addr = mcsock_read_string(pack);
        uint16_t srv_port = mcsock_read_ushort(pack);
        int32_t next_state = mcsock_read_varint(pack);
        bool is_status = next_state == 1;
        bool is_play = next_state == 2;

        LDEBUG("Received handshake! [PVN=%d ADDR='%s' PORT=%u NS=%d CLID=%d]", pvn, srv_addr->data, srv_port, next_state, conn->client_id);

        free_mcstring(srv_addr);
        free_packet(pack);

        if (is_status) conn->state = CL_STATUS;
        if (is_play) conn->state = CL_LOGIN;

        switch (pvn) {
            case MC_PVN_1_18_2:
            case -1: // Client is determining what version to use
                // TODO: threading (also ZMQ pair socket management)
                if (is_status) handle_status_758(conn);
                if (is_play) handle_play_758(conn);
                break;
            default:
                LWARN("Invalid PVN %d!", pvn);
                close(conn->fd);
        }
    } else if (pack) {
        LWARN("Invalid handshaking packet ID %02x!", pack->packet_id);
        free_packet(pack);
        close(conn->fd);
    } else {
        /* Silently drop closed connection */
        close(conn->fd);
    }
    conn->state = CL_TERMINATED;
    return NULL;
}

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

    int client_id = 0;

    /*
    There is a buffer, called the Client Thread Buffer (CTB), which
    manages all client threads. Each entry is an mcclient_t object,
    and retains information about the thread, the socket, and the
    connection state. The CTB dynamically changes size when it gets
    too small for a new request using ctb_size and ctb_used. It is
    doubled in size when it is full and a new request arrives,
    and halves in size when an item is removed and it becomes less
    than a quarter full.
    */
    size_t ctb_size = CTB_MIN_SIZE;
    size_t ctb_used = 0;
    mcclient_t *ctb = malloc(ctb_size*sizeof(mcclient_t));

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

            /* Create mcsock object */
            mcsock_t *conn = malloc(sizeof(mcsock_t));
            conn->fd = new_socket;
            conn->compressed = false;
            conn->encrypted = false;
            conn->client_id = client_id++;
            conn->state = CL_HANDSHAKING;
            conn->username = NULL;

            /* Add to CTB */
            if (ctb_used == ctb_size) {
                /* CTB is full, double its size */
                ctb_size <<= 1;
                ctb = realloc(ctb, ctb_size*sizeof(mcclient_t));
                LVERBOSE("Resized CTB to %ld entries!", ctb_size);
            }
            mcclient_t *this_client = &ctb[ctb_used++];
            this_client->conn = conn;
            this_client->pair_sock = zmq_socket(crss_zmq_ctx(), ZMQ_PAIR);
            char client_endpoint[35];
            sprintf(client_endpoint, "inproc://network-client-%d", conn->client_id);
            zmq_bind(this_client->pair_sock, (const char *)client_endpoint);

            /* Handle the request */
            pthread_create(&this_client->thread, NULL, (void*(*)(void*))dispatch, conn);
        }
        /* Check CTB for terminated threads and for requests */
        for (int i = 0; i < ctb_used; i++) {
            if (ctb[i].conn->state == CL_TERMINATED) {
                if (ctb[i].conn->username) {
                    LDEBUG("Client thread for player %s has terminated!", ctb[i].conn->username);
                } else {
                    LDEBUG("Client %d has terminated!", ctb[i].conn->client_id);
                }

                /* Clean up CTB entry*/
                free(ctb[i].conn);
                zmq_close(ctb[i].pair_sock);

                /* Join thread */
                pthread_join(ctb[i].thread, NULL);

                /* Copy last CTB item to current entry */
                memcpy(&ctb[i], &ctb[--ctb_used], sizeof(mcclient_t));
                /* Potentially make CTB smaller */
                if (ctb_used < (ctb_size >> 2) && ctb_size > CTB_MIN_SIZE) {
                    ctb_size >>= 1;
                    ctb = realloc(ctb, ctb_size * sizeof(mcclient_t));
                    LVERBOSE("Resized CTB to %ld entries!", ctb_size);
                }
            } else {
                pairsock_op_t *msg = client_pair_recv(ctb[i].pair_sock);
                if (msg) {
                    LVERBOSE("Received request [%s] '%s' from client %d! [%d bytes]", msg->operation, msg->data, ctb[i].conn->client_id, strlen(msg->data) + 8);
                    if (strncmp(msg->operation, "statjson", 8) == 0) {
                        /* Request status json */
                        client_pair_send(ctb[i].pair_sock, "statjson", (char *)DEFAULT_STATUS_JSON_RESPONSE);
                    }
                    free(msg->data);
                    free(msg);
                }
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

    /* Free all items in CTB and join threads */
    for (int i = 0; i < ctb_used; i++) {
        LDEBUG("Closing client %d/%d...", i, ctb_used);
        zmq_close(ctb[i].pair_sock);
        pthread_join(ctb[i].thread, NULL);
    }
    free(ctb);

    LINFO("Freeing socket!!!!");
    CMD_HANDLER_CLEANUP();

    return exit_code;
}

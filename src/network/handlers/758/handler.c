#include "handler.h"

void *handle_status_758(mcsock_t *conn) {
    char *fnpath = "network.handlers.758.status";
    (void)(fnpath);

    /* Initialise pair socket */
    void *pair_sock = zmq_socket(crss_zmq_ctx(), ZMQ_PAIR);
    char client_endpoint[35];
    sprintf(client_endpoint, "inproc://network-client-%d", conn->client_id);
    zmq_connect(pair_sock, client_endpoint);

    /* Listen for Status Request */
    packet_t *pack = packet_recv(conn);
    if (pack && pack->packet_id == 0) {
        free_packet(pack);

        /* Request received, respond */
        pack = new_packet(2048);
        client_pair_send(pair_sock, "statjson", "");
        pairsock_op_t *json_response_op = client_pair_recv_blocking(pair_sock);
        if (strncmp(json_response_op->operation, "statjson", 8) != 0) {
            LWARN("Received invalid pair socket response type [%s] (expected 'statjson')!", json_response_op->operation);
            free(json_response_op->data);
            free(json_response_op);
            free_packet(pack);
            close(conn->fd);
            zmq_close(pair_sock);
            return NULL;
        }
        mcstring_t *json_response = new_mcstring(json_response_op->data);
        mcsock_write_string(pack, json_response);
        packet_send(pack, conn);
        
        free(json_response_op->data);
        free(json_response_op);
        free_mcstring(json_response);
        free_packet(pack);

        /* Fetch next packet so that after this basic block, pack must
        contain a ping packet (or NULL) */
        pack = packet_recv(conn);
    }
    if (pack && pack->packet_id == 1) {
        packet_t *resp_pack = new_packet(64);
        resp_pack->packet_id = 1;
        int64_t payload = mcsock_read_long(pack);
        /* Send pong response, as fast as possible */
        mcsock_write_long(resp_pack, payload);
        packet_send(resp_pack, conn);
        /* Cleanup */
        free_packet(pack);
        free_packet(resp_pack);
    }

    close(conn->fd);
    zmq_close(pair_sock);
    
    return NULL;
}

void *handle_play_758(mcsock_t *conn) {
    close(conn->fd);
    return NULL;
}

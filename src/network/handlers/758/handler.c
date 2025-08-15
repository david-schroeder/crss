#include "handler.h"

void *handle_status_758(mcsock_t *conn) {
    char *fnpath = "network.handlers.758.status";
    (void)(fnpath);
    /* Listen for Status Request */
    packet_t *pack = packet_recv(conn);
    if (pack && pack->packet_id == 0) {
        free_packet(pack);

        /* Request received, respond */
        pack = new_packet(2048);
        // TODO: make this string pull data from reasonable sources
        mcstring_t *json_response = new_mcstring((char*)DEFAULT_STATUS_JSON_RESPONSE);
        mcsock_write_string(pack, json_response);
        packet_send(pack, conn);
        
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
    free(conn);
    
    return NULL;
}

void *handle_play_758(mcsock_t *conn) {
    close(conn->fd);
    free(conn);
    return NULL;
}

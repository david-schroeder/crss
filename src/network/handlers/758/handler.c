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

void send_disconnect_login(mcsock_t *conn, char *reason) {
    /* Packet preparation */
    packet_t *pack = new_packet(128);
    pack->packet_id = PACKID_S2C_DISCONNECT_LOGIN;
    
    /* Fields */
    mcsock_write_c_string(pack, reason);
    
    /* Send packet */
    packet_send(pack, conn);

    /* Cleanup */
    free_packet(pack);
}

void send_encryption_request(mcsock_t *conn, uint32_t vtok) {
    /* Packet preparation */
    packet_t *pack = new_packet(2048);
    pack->packet_id = PACKID_S2C_ENCRYPTION_REQUEST;
    
    /* Fields */
    mcsock_write_c_string(pack, PROTOCOL_SERVER_ID);
    keypair_t *kp = conn->server_keypair;
    mcsock_write_varint(pack, kp->pkey_asn1_len);
    mcsock_write_byte_array(pack, kp->pub_key_asn1, kp->pkey_asn1_len);
    mcsock_write_varint(pack, 4);
    mcsock_write_byte_array(pack, (uint8_t *)(&vtok), 4);
    
    /* Send packet */
    packet_send(pack, conn);

    /* Cleanup */
    free_packet(pack);
}

void send_login_success(mcsock_t *conn, uuid_t uuid, char *username) {
    /* Packet preparation */
    packet_t *pack = new_packet(64);
    pack->packet_id = PACKID_S2C_LOGIN_SUCCESS;
    
    /* Fields */
    mcsock_write_uuid(pack, uuid);
    mcsock_write_c_string(pack, username);
    
    /* Send packet */
    packet_send(pack, conn);

    /* Cleanup */
    free_packet(pack);
}

void send_set_compression(mcsock_t *conn, uint32_t pack_size) {
    /* Packet preparation */
    packet_t *pack = new_packet(64);
    pack->packet_id = PACKID_S2C_SET_COMPRESSION;
    
    /* Fields */
    mcsock_write_varint(pack, pack_size);
    
    /* Send packet */
    packet_send(pack, conn);

    /* Cleanup */
    free_packet(pack);
}

static void handle_state_play(mcsock_t *conn);

void *handle_play_758(mcsock_t *conn) {
    char *fnpath = "network.handlers.758.play";
    (void)(fnpath);
    packet_t *pack;

    //////////////////////////
    // INIT AND LOGIN START //
    //////////////////////////

    /* Initialise pair socket */
    void *pair_sock = zmq_socket(crss_zmq_ctx(), ZMQ_PAIR);
    char client_endpoint[35];
    sprintf(client_endpoint, "inproc://network-client-%d", conn->client_id);
    zmq_connect(pair_sock, client_endpoint);

    /* For any errors */
    #define DISCONNECT_AND_CLOSE(msg) {\
        send_disconnect_login(conn, "{\"text\": \"" msg "\"}"); \
        close(conn->fd); \
        zmq_close(pair_sock); \
        return NULL; \
    }
    #define ASSERT_ELSE_CLOSE(cond, else_, msg) \
        if (!(cond)) {else_ DISCONNECT_AND_CLOSE(msg)}
    #define EXPECT_PACKET(packid, errmsg, on_success) \
        pack = packet_recv(conn); \
        ASSERT_ELSE_CLOSE(pack && pack->packet_id == packid, {free(pack);}, errmsg); \
        on_success \
        free_packet(pack);
    
    EXPECT_PACKET(PACKID_C2S_LOGIN_START, "Invalid login start packet", {
        mcstring_t *username = mcsock_read_string(pack);
        conn->username = malloc(username->length + 1);
        strncpy(conn->username, username->data, username->length);
        conn->username[username->length] = '\0';
        free_mcstring(username);
    })
    
    ////////////////
    // ENCRYPTION //
    ////////////////

    /* Generate verify token */
    uint32_t generated_vtok = gen_vtok();
    
    /* Encryption Request */
    send_encryption_request(conn, generated_vtok);

    /* Encryption Response (encrypted) */
    int32_t shared_secret_len, vtok_len;
    uint8_t *shared_secret, *vtok;
    EXPECT_PACKET(PACKID_C2S_ENCRYPTION_RESPONSE, "Invalid encryption response packet", {
        shared_secret_len = mcsock_read_varint(pack);
        shared_secret = mcsock_read_byte_array(pack, shared_secret_len);
        vtok_len = mcsock_read_varint(pack);
        vtok = mcsock_read_byte_array(pack, vtok_len);
    })
    /* Decrypt data */
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(conn->server_keypair->pkey, NULL);

    /* Decrypt shared secret */
    decrypted_buffer_t *decrypted_shared_secret = rsa_decrypt_buffer(ctx, shared_secret, shared_secret_len);
    ASSERT_ELSE_CLOSE(decrypted_shared_secret != NULL, {
        EVP_PKEY_CTX_free(ctx);
        free(vtok);
        free(shared_secret);
        LWARN("Error during RSA decryption [Shared Secret] [%s]", conn->username);
    }, "Internal error! Check console for details.");

    /* Decrypt verify token */
    decrypted_buffer_t *decrypted_vtok = rsa_decrypt_buffer(ctx, vtok, vtok_len);
    free(vtok);
    free(shared_secret);
    ASSERT_ELSE_CLOSE(decrypted_shared_secret != NULL, {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(decrypted_shared_secret->data);
        free(decrypted_shared_secret);
        LWARN("Error during RSA decryption [Verify Token] [%s]", conn->username);
    }, "Internal error! Check console for details.");

    /* Make sure received verify token is correct */
    ASSERT_ELSE_CLOSE(*((uint32_t *)decrypted_vtok->data) == generated_vtok, {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(decrypted_shared_secret->data);
        OPENSSL_free(decrypted_vtok->data);
        free(decrypted_shared_secret);
        free(decrypted_vtok);
        LWARN("Verify tokens did not match! [%s]", conn->username);
    }, "Failed to encrypt");

    /* Print shared secret */
    char shared_secret_bytes[64];
    for (int i = 0; i < decrypted_shared_secret->length; i++) {
        uint8_t byte = decrypted_shared_secret->data[i];
        sprintf(&shared_secret_bytes[3*i], "%02x  ", byte & 0xff);
    }
    LDEBUG("Shared secret for %s: %s", conn->username, shared_secret_bytes);

    /* Free encryption data */
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(decrypted_vtok->data);
    free(decrypted_vtok);

    conn->shared_secret = decrypted_shared_secret;

    /* At this point, key exchange is complete, and the AES/CFB8 ciphers can be created */

    /* S2C (Encryption) Cipher */
    EVP_CIPHER_CTX *s2c_cipher_ctx = EVP_CIPHER_CTX_new();
    // Key and Initial Vector (IV) are both set to the shared secret
    EVP_EncryptInit_ex(s2c_cipher_ctx, EVP_aes_128_cfb8(), NULL, conn->shared_secret->data, conn->shared_secret->data);

    /* C2S (Decryption) Cipher */
    EVP_CIPHER_CTX *c2s_cipher_ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(c2s_cipher_ctx, EVP_aes_128_cfb8(), NULL, conn->shared_secret->data, conn->shared_secret->data);

    /* Store Ciphers in conn and set encrypted mode to true */
    conn->s2c_ctx = s2c_cipher_ctx;
    conn->c2s_ctx = c2s_cipher_ctx;
    /* Switch to encrypted mode for packet exchange */
    conn->encrypted = true;

    ////////////////////
    // AUTHENTICATION //
    ////////////////////

    /* SHA1 Hex Digest (Server Hash) generation */
    uint8_t serverHash[EVP_MAX_MD_SIZE];
    uint32_t hash_len;

    EVP_MD_CTX *sha1_ctx = EVP_MD_CTX_new();
    ASSERT_ELSE_CLOSE((sha1_ctx != NULL) && (EVP_DigestInit_ex(sha1_ctx, EVP_sha1(), NULL) == 1), {
        LFATAL("Failed to initialize SHA1 EVP context for %s!", conn->username);
        EVP_CIPHER_CTX_free(s2c_cipher_ctx);
        EVP_CIPHER_CTX_free(c2s_cipher_ctx);
        OPENSSL_free(decrypted_shared_secret->data);
        free(decrypted_shared_secret);
    }, "Internal error");

    /* SHA1 updated with Server ID, Shared Secret, Server's encoded public key */
    EVP_DigestUpdate(sha1_ctx, PROTOCOL_SERVER_ID, strlen(PROTOCOL_SERVER_ID));
    EVP_DigestUpdate(sha1_ctx, decrypted_shared_secret->data, decrypted_shared_secret->length);
    EVP_DigestUpdate(sha1_ctx, conn->server_keypair->pub_key_asn1, conn->server_keypair->pkey_asn1_len);

    /* Generate hex digest */
    ASSERT_ELSE_CLOSE(EVP_DigestFinal_ex(sha1_ctx, serverHash, &hash_len) == 1, {
        LFATAL("Failed to generate SHA1 Hex Digest for %s!", conn->username);
        EVP_CIPHER_CTX_free(s2c_cipher_ctx);
        EVP_CIPHER_CTX_free(c2s_cipher_ctx);
        EVP_MD_CTX_free(sha1_ctx);
        OPENSSL_free(decrypted_shared_secret->data);
        free(decrypted_shared_secret);
    }, "Internal error");
    EVP_MD_CTX_free(sha1_ctx);

    mcstring_t *mc_hexdigest = gen_mc_hexdigest(serverHash, hash_len);
    LDEBUG("SHA1 Hex Digest for %s: %s", conn->username, mc_hexdigest->data);

    /* Call Mojang API to authenticate */
    char auth_url[192];
    sprintf(auth_url, "https://sessionserver.mojang.com/session/minecraft/hasJoined?username=%s&serverId=%s", conn->username, mc_hexdigest->data);
    free_mcstring(mc_hexdigest);
    mcstring_t *auth_response = get_request(auth_url);
    LVERBOSE("Response:\n%s", auth_response->data);

    /* Parse UUID from response */
    //TODO: make better JSON parsing
    char *response_uuid = strstr(auth_response->data, "\"id\" : \"");
    if (response_uuid == NULL) {
        LWARN("Could not find UUID in response!");
        send_disconnect_login(conn, "Internal error");
        free_mcstring(auth_response);
        EVP_CIPHER_CTX_free(s2c_cipher_ctx);
        EVP_CIPHER_CTX_free(c2s_cipher_ctx);
        OPENSSL_free(decrypted_shared_secret->data);
        free(decrypted_shared_secret);

        close(conn->fd);
        zmq_close(pair_sock);

        return NULL;
    }
    response_uuid = &response_uuid[8]; // first char of UUID
    char formatted_resp_uuid[37];
    strncpy(formatted_resp_uuid, response_uuid, 8);
    formatted_resp_uuid[8] = '-';
    strncpy(&formatted_resp_uuid[9], &response_uuid[8], 4);
    formatted_resp_uuid[13] = '-';
    strncpy(&formatted_resp_uuid[14], &response_uuid[12], 4);
    formatted_resp_uuid[18] = '-';
    strncpy(&formatted_resp_uuid[19], &response_uuid[16], 4);
    formatted_resp_uuid[23] = '-';
    strncpy(&formatted_resp_uuid[24], &response_uuid[20], 12);
    formatted_resp_uuid[36] = '\0';
    free_mcstring(auth_response);
    uuid_t resp_uuid_uuid;
    string_to_uuid(formatted_resp_uuid, &resp_uuid_uuid);
    LDEBUG("Extracted UUID from response: %s", formatted_resp_uuid);

    send_login_success(conn, resp_uuid_uuid, conn->username);

    handle_state_play(conn);

    /* Free Stream Ciphers + Shared Secret after connection is closed */
    EVP_CIPHER_CTX_free(s2c_cipher_ctx);
    EVP_CIPHER_CTX_free(c2s_cipher_ctx);
    OPENSSL_free(decrypted_shared_secret->data);
    free(decrypted_shared_secret);

    close(conn->fd);
    zmq_close(pair_sock);

    return NULL;
}

static void handle_state_play(mcsock_t *conn) {
    
}

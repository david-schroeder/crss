#include "network_utils.h"

static inline pairsock_op_t *_client_pair_recv(void *sock, bool block) {
    uint32_t length;
    if (zmq_recv(sock, &length, 4, block ? 0 : ZMQ_DONTWAIT) < 0) {
        return NULL;
    }
    pairsock_op_t *op = malloc(sizeof(pairsock_op_t));
    op->operation[8] = '\0';
    char *buf = malloc(length + 9);
    if (zmq_recv(sock, buf, length + 8, block ? 0 : ZMQ_DONTWAIT) < 0) {
        DLWARN("Malformed packet [Received length %d] (errno %d)!", length, errno);
        free(buf);
        free(op);
        return NULL;
    }
    char *data = malloc(length + 1);
    memcpy(op->operation, buf, 8);
    memcpy(data, &buf[8], length);
    data[length] = '\0';
    op->data = data;
    free(buf);
    return op;
}

pairsock_op_t *client_pair_recv(void *sock) {
    return _client_pair_recv(sock, false);
}

pairsock_op_t *client_pair_recv_blocking(void *sock) {
    return _client_pair_recv(sock, true);
}

void client_pair_send(void *sock, char *operation, char *data) {
    uint32_t len = strlen(data);
    uint8_t *buf = malloc(8+len);
    memcpy(buf, operation, 8);
    memcpy(&buf[8], data, len);
    zmq_send(sock, &len, 4, 0);
    zmq_send(sock, buf, len+8, 0);
    free(buf);
}

static int32_t sock_read_varint(mcsock_t *s) {
    char *fnpath = "network.utils.sock_read_varint";
    int32_t varint = 0;
    uint8_t next_byte, next_byte_value;
    int outl; // For Decryptupdate
    (void)(outl);
    for (int byte_idx = 0; byte_idx < 5; byte_idx++) {
        if (read(s->fd, &next_byte, 1) < 0) {
            LWARN("Read failed!");
            return -1;
        }
        if (s->encrypted) {
            /* Single-byte decrypt */
            EVP_DecryptUpdate(s->c2s_ctx, &next_byte, &outl, &next_byte, 1);
        }
        next_byte_value = next_byte & 0x7F;
        varint |= next_byte_value << (byte_idx * 7);
        if ((next_byte & 0x80) == 0) break;
    }
    return varint;
}

/*
@returns Packet object or NULL if socket fetch fails
*/
packet_t *packet_recv(mcsock_t *s) {
    char *fnpath = "network.utils.packet_recv";
    if (s->compressed) {
        // TODO: add compression support
        LFATAL("Compressed mode is not yet supported!");
        return NULL;
    }
    int32_t length = sock_read_varint(s);
    if (length <= 0) return NULL;
    packet_t *pack = malloc(sizeof(packet_t));
    pack->data_length = length;
    uint8_t *data = malloc(length);
    read(s->fd, data, length);
    if (s->encrypted) {
        /* Decrypt received data */
        EVP_DecryptUpdate(s->c2s_ctx, data, &length, data, length);
    }
    pack->data = data;
    pack->data_ptr = 0;
    pack->packet_id = mcsock_read_varint(pack);
    //LVERBOSE("Received packet %02x [Length %d]!", pack->packet_id, pack->data_length);
    return pack;
}

static size_t write_varint_to_buf(uint8_t *buf, int32_t value) {
    size_t vi_size = 0; // VarInt size
    bool has_more;
    while (value || vi_size == 0) {
        has_more = value >= 0x80;
        buf[vi_size++] = (value & 0x7F) | (has_more ? 0x80 : 0);
        value >>= 7;
    }
    return vi_size;
}

void packet_send(packet_t *p, mcsock_t *s) {
    char *fnpath = "network.utils.packet_send";
    if (s->compressed) {
        // TODO: add compression support
        LFATAL("Compressed mode is not yet supported!");
        return;
    }
    uint8_t packid_buf[5];
    size_t packid_len = write_varint_to_buf(packid_buf, p->packet_id);
    size_t total_len = packid_len + p->data_ptr;
    uint8_t packlen_buf[5];
    size_t packlen_len = write_varint_to_buf(packlen_buf, total_len);

    if (s->encrypted) {
        // Since lengths are consistent, we can reuse the existing buffers
        EVP_EncryptUpdate(s->s2c_ctx, packlen_buf, (int *)&packlen_len, packlen_buf, packlen_len);
        EVP_EncryptUpdate(s->s2c_ctx, packid_buf, (int *)&packid_len, packid_buf, packlen_len);
        EVP_EncryptUpdate(s->s2c_ctx, p->data, (int *)&p->data_ptr, p->data, p->data_ptr);
    }

    write(s->fd, packlen_buf, packlen_len);
    write(s->fd, packid_buf, packid_len);
    write(s->fd, p->data, p->data_ptr);
}

packet_t *new_packet(uint32_t init_buf_size) {
    packet_t *pack = malloc(sizeof(packet_t));
    pack->data_length = init_buf_size;
    pack->data = malloc(init_buf_size);
    pack->data_ptr = 0;
    pack->packet_id = 0;
    return pack;
}

void free_packet(packet_t *pack) {
    free(pack->data);
    free(pack);
}

bool mcsock_read_bool(packet_t *s) {
    return (bool)(s->data[s->data_ptr++]);
}

int8_t mcsock_read_byte(packet_t *s) {
    return (int8_t)(s->data[s->data_ptr++]);
}

uint8_t mcsock_read_ubyte(packet_t *s) {
    return (uint8_t)(s->data[s->data_ptr++]);
}

int16_t mcsock_read_short(packet_t *s) {
    uint8_t lo, hi;
    hi = s->data[s->data_ptr++];
    lo = s->data[s->data_ptr++];
    return (int16_t)(hi << 8 | lo);
}

uint16_t mcsock_read_ushort(packet_t *s) {
    uint8_t lo, hi;
    hi = s->data[s->data_ptr++];
    lo = s->data[s->data_ptr++];
    return (int16_t)(hi << 8 | lo);
}

int32_t mcsock_read_int(packet_t *s) {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value <<= 8;
        value |= s->data[s->data_ptr++];
    }
    return (int32_t)value;
}

int64_t mcsock_read_long(packet_t *s) {
    uint64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value <<= 8;
        value |= s->data[s->data_ptr++];
    }
    return (int64_t)value;
}

float mcsock_read_float(packet_t *s) {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value <<= 8;
        value |= s->data[s->data_ptr++];
    }
    return (float)value;
}

double mcsock_read_double(packet_t *s) {
    uint64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value <<= 8;
        value |= s->data[s->data_ptr++];
    }
    return (double)value;
}

mcstring_t *mcsock_read_string(packet_t *s) {
    int32_t nbytes = mcsock_read_varint(s);
    char *string = malloc(nbytes + 1);
    memcpy(string, &s->data[s->data_ptr], nbytes);
    string[nbytes] = '\0';
    s->data_ptr += nbytes;
    mcstring_t *mcstring = malloc(sizeof(mcstring_t));
    mcstring->length = nbytes;
    mcstring->data = string;
    return mcstring;
}

mcstring_t *new_mcstring(char *s) {
    mcstring_t *string = malloc(sizeof(mcstring_t));
    string->data = mystrdup(s);
    string->length = strlen(s);
    return string;
}

void free_mcstring(mcstring_t *string) {
    free(string->data);
    free(string);
}

mcstring_t *mcsock_read_identifier(packet_t *s) {
    return mcsock_read_string(s);
}

int32_t mcsock_read_varint(packet_t *s) {
    int32_t varint = 0;
    uint8_t next_byte, next_byte_value;
    for (int byte_idx = 0; byte_idx < 5; byte_idx++) {
        next_byte = s->data[s->data_ptr++];
        next_byte_value = next_byte & 0x7F;
        varint |= next_byte_value << (byte_idx * 7);
        if ((next_byte & 0x80) == 0) break;
    }
    return varint;
}

int64_t mcsock_read_varlong(packet_t *s) {
    int64_t varlong = 0;
    uint8_t next_byte, next_byte_value;
    for (int byte_idx = 0; byte_idx < 10; byte_idx++) {
        next_byte = s->data[s->data_ptr++];
        next_byte_value = next_byte & 0x7F;
        varlong |= next_byte_value << (byte_idx * 7);
        if ((next_byte & 0x80) == 0) break;
    }
    return varlong;
}

uint8_t *mcsock_read_byte_array(packet_t *s, uint32_t len) {
    uint8_t *buf = malloc(len);
    memcpy(buf, &s->data[s->data_ptr], len);
    s->data_ptr += len;
    return buf;
}

/*
Make sure there is enough free space in packet `p` to fit `s` bytes.
*/
static inline void allocate_packet_space(packet_t *p, size_t s) {
    // Free remaining length is given by data_length - data_ptr
    if (s > p->data_length - p->data_ptr) {
        // Reallocate
        // double buffer size until `s` fits
        do {
            p->data_length <<= 1;
        } while (s > p->data_length - p->data_ptr);
        p->data = realloc(p->data, p->data_length);
    }
}

void mcsock_write_bool(packet_t *p, bool value) {
    allocate_packet_space(p, 1);
    p->data[p->data_ptr++] = value ? 1 : 0;
}

void mcsock_write_byte(packet_t *p, int8_t value) {
    allocate_packet_space(p, 1);
    p->data[p->data_ptr++] = value;
}

void mcsock_write_ubyte(packet_t *p, uint8_t value) {
    allocate_packet_space(p, 1);
    p->data[p->data_ptr++] = value;
}

void mcsock_write_short(packet_t *p, int16_t value) {
    allocate_packet_space(p, 2);
    // Big endian -> MSB first
    p->data[p->data_ptr++] = value >> 8;
    p->data[p->data_ptr++] = value & 0xFF;
}

void mcsock_write_ushort(packet_t *p, uint16_t value) {
    allocate_packet_space(p, 2);
    p->data[p->data_ptr++] = value >> 8;
    p->data[p->data_ptr++] = value & 0xFF;
}

void mcsock_write_int(packet_t *p, int32_t value) {
    allocate_packet_space(p, 4);
    for (int i = 0; i < 4; i++) {
        p->data[p->data_ptr++] = (value >> 24) & 0xFF;
        value <<= 8;
    }
}

void mcsock_write_long(packet_t *p, int64_t value) {
    allocate_packet_space(p, 8);
    for (int i = 0; i < 8; i++) {
        p->data[p->data_ptr++] = (value >> 56) & 0xFF;
        value <<= 8;
    }
}

void mcsock_write_float(packet_t *p, float value) {
    // Requires strict aliasing to be off
    uint32_t n = *((uint32_t *)&value);
    allocate_packet_space(p, 4);
    for (int i = 0; i < 4; i++) {
        p->data[p->data_ptr++] = (n >> 24) & 0xFF;
        n <<= 8;
    }
}

void mcsock_write_double(packet_t *p, double value) {
    uint64_t n = *((uint64_t *)&value);
    allocate_packet_space(p, 8);
    for (int i = 0; i < 8; i++) {
        p->data[p->data_ptr++] = (n >> 56) & 0xFF;
        n <<= 8;
    }
}

void mcsock_write_string(packet_t *p, mcstring_t *string) {
    mcsock_write_varint(p, string->length);
    allocate_packet_space(p, string->length);
    memcpy(&p->data[p->data_ptr], string->data, string->length);
    p->data_ptr += string->length;
}

void mcsock_write_c_string(packet_t *p, char *str) {
    mcstring_t *s = new_mcstring(str);
    mcsock_write_string(p, s);
    free_mcstring(s);
}

void mcsock_write_identifier(packet_t *p, mcstring_t *identifier) {
    mcsock_write_string(p, identifier);
}

void mcsock_write_varint(packet_t *p, int32_t value) {
    uint8_t buf[5];
    size_t vi_size = write_varint_to_buf(buf, value);
    allocate_packet_space(p, vi_size);
    memcpy(&p->data[p->data_ptr], buf, vi_size);
    p->data_ptr += vi_size;
}

void mcsock_write_varlong(packet_t *p, int64_t value) {
    uint8_t buf[10] = {0,0,0,0,0,0,0,0,0,0};
    uint32_t vl_size = 0; // VarLong size
    bool has_more;
    while (value || vl_size == 0) {
        has_more = value >= 0x80;
        buf[vl_size++] = (value & 0x7F) | (has_more ? 0x80 : 0);
        value >>= 7;
    }
    allocate_packet_space(p, vl_size);
    memcpy(&p->data[p->data_ptr], buf, vl_size);
    p->data_ptr += vl_size;
}

void mcsock_write_uuid(packet_t *p, uuid_t uuid) {
    allocate_packet_space(p, 16);
    /* memcpy bytes of UUID in reverse order (endiannes) */
    for (int i = 0; i < 16; i++) {
        p->data[p->data_ptr++] = ((uint8_t *)uuid)[15-i];
    }
}

void mcsock_write_byte_array(packet_t *p, uint8_t *array, uint32_t len) {
    allocate_packet_space(p, len);
    memcpy(&p->data[p->data_ptr], array, len);
    p->data_ptr += len;
}

mcstring_t *gen_mc_hexdigest(uint8_t *digest, uint32_t len) {
    /* Minecraft hex digest
    
    Interpret digest (big endian) as 2's complement and prefix with '-',
    if necessary */

    if (len == 0) return NULL;

    mcstring_t *digest_mcstring = malloc(sizeof(mcstring_t));
    char *digest_string = malloc(len*2+2); // 2 base 16 digits per byte + sign + zero byte
    digest_mcstring->data = digest_string;

    bool is_negative = (digest[0] & 0x80) > 0; /* MSB */

    if (is_negative) {
        /* Traverse digest from LSB to MSB, inverting (and propagating carry) */
        uint8_t carry = 1; /* 2's comp inversion requires adding 1 */
        for (int i = len-1; i >= 0; i--) {
            /* Flip byte i bits and add carry */
            digest[i] = 0xff - digest[i] + carry;
            /* If byte i (inverted) in hex digest is 0xff, keep carrying */
            // There can be no generates past byte 0 when adding 1; only propagates
            carry = digest[i] == 0xff && carry ? 1 : 0;
        }
    }
    
    /* Traverse through digest from MSB to LSB, generating output string */
    int digest_ptr = is_negative ? 1 : 0; // start writing digits at second position if negative
    if (is_negative) digest_string[0] = '-';
    bool is_leading_zero = true; // true until first bit is set
    for (int i = 0; i < len; i++) {
        sprintf(&digest_string[digest_ptr], is_leading_zero ? "%x" : "%02x", (digest[i] & 0xff));
        if (is_leading_zero) {
            if ((digest[i] & 0xff) > 0x0f) {
                digest_ptr++;
            }
            if ((digest[i] & 0xff) > 0x00) {
                digest_ptr++; /* 2nd increment if digest[i] > 0x0f */
                is_leading_zero = false;
            }
        } else digest_ptr += 2; // If we're past leading zeroes, every byte gets 2 hex digits
    }
    digest_mcstring->data[digest_ptr] = '\0';
    digest_mcstring->length = digest_ptr;
    return digest_mcstring;
}

static size_t _get_req_write_cb(void *data, size_t size, size_t nmemb, void *user_ptr) {
    /* Preparation */
    size_t byte_size = size * nmemb;
    mcstring_t *string = (mcstring_t *)user_ptr;

    /* Update string */
    string->data = realloc(string->data, string->length + byte_size + 1);
    memcpy(&string->data[string->length], data, byte_size);
    string->length += byte_size;
    string->data[string->length] = '\0';

    return byte_size;
}

mcstring_t *get_request(char *url) {
    CURL *curl;
    CURLcode status_code;

    DLVERBOSE("Requesting data from '%s' via GET...", url);

    mcstring_t *response = malloc(sizeof(mcstring_t));
    response->data = malloc(1);
    response->data[0] = '\0';
    response->length = 0;

    curl = curl_easy_init();
    if (!curl) return NULL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _get_req_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    status_code = curl_easy_perform(curl);
    if (status_code != CURLE_OK) {
        free(response->data);
        free(response);
        DLWARN("Status code %d while performing GET on '%s'!", status_code, url);
        return NULL;
    }
    curl_easy_cleanup(curl);
    return response;
}

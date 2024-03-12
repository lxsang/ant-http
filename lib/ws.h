#ifndef WS_H
#define WS_H

#include <stdint.h>

#define WS_TEXT 0x1
#define WS_BIN 0x2
#define WS_CLOSE 0x8
#define WS_PING 0x9
#define WS_PONG 0xA
#define ws_t(c, d) (ws_send_text(c, d, 0))
#define ws_b(c, d, z) (ws_send_binary(c, d, z, 0))
#define ws_f(c, f) (ws_send_file(c, f, 0))
#define ws_close(c, r) (ws_send_close(c, r, 0))

typedef struct
{
    unsigned int port;
    int usessl;
    int sock;
    antd_proto_t type;
} ws_port_config_t;

typedef struct
{
	uint8_t fin;
	uint8_t opcode;
	unsigned int plen;
	uint8_t mask;
	uint8_t mask_key[4];
} ws_msg_header_t;

typedef struct
{
	const char *host;
	const char *resource;
	antd_client_t *antdsock;
	// ssl
	const char *sslcert;
	const char *sslkey;
	const char *sslpasswd;
	const char *ciphersuit;
	const char *verify_location;
	void *ssl_ctx;
} ws_client_t;

ws_msg_header_t *ws_read_header(void *);
int ws_send_frame(void *, uint8_t *, ws_msg_header_t);
int ws_pong(void *client, ws_msg_header_t *, int mask);

int ws_ping(void *client, const char *echo, int mask);

int ws_send_text(void *client, const char *data, int mask);
int ws_send_close(void *client, unsigned int status, int mask);
int ws_send_file(void *client, const char *file, int mask);
int ws_send_binary(void *client, uint8_t *data, int l, int mask);

int ws_read_data(void *, ws_msg_header_t *, int, uint8_t *);

// client

void ws_client_close(ws_client_t *wsclient);
int ws_client_connect(ws_client_t *wsclient, ws_port_config_t pcnf);
int ws_open_handshake(ws_client_t *client);
#endif
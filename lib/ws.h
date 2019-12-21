#ifndef WS_H
#define WS_H
#include <resolv.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdint.h>
#include<netdb.h> //hostent
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <sys/time.h>

#include "utils.h"
#include "handle.h"
#define CONN_TIME_OUT_S 3
#define BITV(v,i)  ((v & (1 << i)) >> i) 
#define WS_TEXT	0x1
#define WS_BIN	0x2
#define WS_CLOSE 0x8
#define WS_PING	 0x9
#define WS_PONG 0xA
#define ws_t(c ,d) (ws_send_text(c,d,0))
#define ws_b(c , d,z) (ws_send_binary(c,d,z,0))
#define ws_f(c,f) (ws_send_file(c,f,0))
#define ws_close(c,r) (ws_send_close(c,r,0))
#define MAX_BUFF 1024

#define PREFERRED_WS_CIPHERS "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS"
#define CLIENT_RQ "GET /%s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n"
#define SERVER_WS_KEY "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="

typedef struct{
	uint8_t fin;
	uint8_t opcode;
	unsigned int plen;
	uint8_t mask;
	uint8_t mask_key[4];
} ws_msg_header_t;

typedef struct{
	const char* host;
	const char* resource;
	antd_client_t* antdsock;
	// ssl
	const char* sslcert;
	const char* sslkey;
	const char* sslpasswd;
	const char* ciphersuit;
	const char* verify_location;
	void * ssl_ctx;
} ws_client_t;


ws_msg_header_t * ws_read_header(void*);
void ws_send_frame(void* , uint8_t* , ws_msg_header_t );
void ws_pong(void* client, ws_msg_header_t*, int mask);

void ws_ping(void* client, const char* echo, int mask);

void ws_send_text(void* client, const char* data,int mask);
void ws_send_close(void* client, unsigned int status, int mask);
void ws_send_file(void* client, const char* file, int mask);
void ws_send_binary(void* client, uint8_t* data, int l, int mask);

int ws_read_data(void* , ws_msg_header_t*, int, uint8_t*);
int request_socket(const char* ip, int port);
int ip_from_hostname(const char * hostname , char* ip);
//int ws_open_hand_shake(const char* host, int port, const char* resource);
char* get_ip_address();

// client

void ws_client_close(ws_client_t* wsclient);
int ws_client_connect(ws_client_t* wsclient);
int ws_open_handshake(ws_client_t* client);
#endif
#ifndef WS_H
#define WS_H

#include <sys/socket.h>
#include <stdio.h>
#include <stdint.h>

#define BITV(v,i)  ((v & (1 << i)) >> i) 
#define WS_TEXT	0x1
#define WS_BIN	0x2
#define WS_CLOSE 0x8
#define WS_PING	 0x9
#define WS_PONG 0xA
typedef struct{
	uint8_t fin;
	uint8_t opcode;
	unsigned int plen;
	uint8_t mask_key[4];
} ws_msg_header_t;

ws_msg_header_t * ws_read_header(int);
void ws_send_frame(int , uint8_t* , ws_msg_header_t );
void ws_t(int , const char* );
void ws_b(int , uint8_t* data, int);
void ws_f(int, const char*);
void ws_close(int, unsigned int);
void pong(int client, int len);
int ws_read_data(int , ws_msg_header_t*, int, uint8_t*);
#endif
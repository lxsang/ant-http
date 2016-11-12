#include "ws.h"

/**
* Read a frame header
* based on this header, we'll decide
* the appropriate handle for frame data
*/
ws_msg_header_t * ws_read_header(int client)
{
	
	uint8_t byte;
	uint8_t bytes[8];
	ws_msg_header_t* header = (ws_msg_header_t*) malloc(sizeof(*header));
	
	// get first byte
	if(recv(client, &byte, sizeof(byte), 0) <0) goto fail;
	if(BITV(byte,6) || BITV(byte,5) || BITV(byte,4)) goto fail;// all RSV bit must be 0
	
	//printf("FIN: %d, RSV1: %d, RSV2: %d, RSV3:%d, opcode:%d\n", BITV(byte,7), BITV(byte,6), BITV(byte,5), BITV(byte,4),(byte & 0x0F) );
	// find and opcode
	header->fin = BITV(byte,7);
	header->opcode = (byte & 0x0F);
	
	// get next byte
	if(recv(client, &byte, sizeof(byte), 0) <0) goto fail;
	
	//printf("MASK: %d paylen:%d\n", BITV(byte,7), (byte & 0x7F));
	// check mask bit, should be 1
	if(!BITV(byte,7))
	{
		// close the connection with protocol error
		ws_close(client, 1002);
		goto fail;
	}
	// get the data length of the frame
	int len = (byte & 0x7F);
	if(len <= 125)
	{
		header->plen = len;
	} else if(len == 126)
	{
		if(recv(client,bytes, 2*sizeof(uint8_t), 0) <0) goto fail;
		header->plen = (bytes[0]<<8) + bytes[1];
		
	} else
	{
		//read only last 4 byte
		if(recv(client,bytes, 8*sizeof(uint8_t), 0) <0) goto fail;
		header->plen = (bytes[4]<<24) + (bytes[5]<<16) + (bytes[6] << 8) + bytes[7] ;
	}
	//printf("len: %d\n", header->plen);
	// last step is to get the maskey
	if(recv(client,header->mask_key, 4*sizeof(uint8_t), 0) <0) goto fail;
	//printf("key 0: %d key 1: %d key2:%d, key3: %d\n",header->mask_key[0],header->mask_key[1],header->mask_key[2], header->mask_key[3] );
	
	// check wheather it is a ping or a close message
	// process it and return NULL
	//otherwise return the header
	//return the header
	switch(header->opcode){
		case WS_CLOSE: // client requests to close the connection
		// send back a close message
		ws_close(client,1000);
		//goto fail;
		break;
		
		case WS_PING: // client send a ping
		// send back a pong message
		pong(client,header->plen);
		break;
		
		default: break;
	}
	return header;
	
	fail:
	free(header);
	return NULL;
	
}
/**
* Read data from client
* and unmask data using the key
*/
int ws_read_data(int client, ws_msg_header_t* header, int len, uint8_t* data)
{
	// if len  == -1 ==> read all remaining data to 'data';
	if(header->plen == 0) return 0;
	int dlen = (len==-1 || len > header->plen)?header->plen:len;
	if((dlen = recv(client,data, dlen, 0)) <0) return -1;
	header->plen = header->plen - dlen;
	// unmask received data
	for(int i = 0; i < dlen; ++i)
		data[i] = data[i]^ header->mask_key[i%4];
	
	return dlen;
}
void _send_header(int client, ws_msg_header_t header)
{
	uint8_t byte = 0;
	uint8_t bytes[8];
	for(int i=0; i< 8; i++) bytes[i] = 0;
	//first byte |FIN|000|opcode|
	byte = (header.fin << 7) + header.opcode;
	send(client, &byte, 1, 0);
	// second byte, payload length
	// mask = 0
	if(header.plen <= 125)
	{
		byte = header.plen;
		send(client, &byte, 1, 0);
	}
	else if(header.plen < 65536) // 16 bits
	{
		byte = 126;
		bytes[0] = (header.plen) >> 8;
		bytes[1] = (header.plen) & 0x00FF;
		send(client, &byte, 1, 0);
		send(client, &bytes, 2, 0);
	}
	else // > 16 bits
	{
		byte = 127;
		bytes[4] = (header.plen) >> 24;
		bytes[5] = ((header.plen)>>16) & 0x00FF;
		bytes[6] = ((header.plen)>>8) & 0x00FF;
		bytes[7] = (header.plen) & 0x00FF;
		send(client, &byte, 1, 0);
		send(client, &bytes, 8, 0);
	}
}
/**
* send a text data frame to client
*/
void ws_t(int client, const char* data)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_TEXT;
	header.plen = strlen(data);
	_send_header(client,header);
	send(client, data, header.plen,0);
}
/**
* send a binary data fram to client
* not tested yet, but should work
*/
void ws_b(int client, uint8_t* data, int l)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_BIN;
	header.plen = l;
	_send_header(client,header);
	send(client, data, header.plen,0);
}
/**
* Not tested yet
* but should work
*/
void pong(int client, int len)
{
	ws_msg_header_t pheader;
	pheader.fin = 1;
	pheader.opcode = WS_PONG;
	pheader.plen = len;
	uint8_t data[len];
	if(recv(client,data, len, 0) < 0) return;
	_send_header(client, pheader);
	send(client, data, len, 0);
}
/*
* Not tested yet, but should work
*/
void ws_close(int client, unsigned int status)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_CLOSE;
	header.plen = 2;
	uint8_t bytes[2];
	bytes[0] = status >> 8;
	bytes[1] = status & 0xFF;
	_send_header(client, header);
	send(client,bytes,2,0);
}
int ws_status(int client)
{
	fd_set sk;
	FD_ZERO(&sk);
	FD_SET(0, &sk);
	FD_SET(client, &sk);
	int result = select(client + 1, sk, NULL, NULL, NULL);
	if(result == 1)
	{
		return FD_ISSET(client, &sk);
	}
	return result;
}
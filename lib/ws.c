
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include <sys/time.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include "utils.h"
#include "handle.h"

#include "ws.h"
static void ws_gen_mask_key(ws_msg_header_t *header)
{
	int r = rand();
	header->mask_key[0] = (r >> 24) & 0xFF;
	header->mask_key[1] = (r >> 16) & 0xFF;
	header->mask_key[2] = (r >> 8) & 0xFF;
	header->mask_key[3] = r & 0xFF;
}
/**
* Read a frame header
* based on this header, we'll decide
* the appropriate handle for frame data
*/
ws_msg_header_t *ws_read_header(void *client)
{

	uint8_t byte = 0;
	uint8_t bytes[8];
	ws_msg_header_t *header = (ws_msg_header_t *)malloc(sizeof(*header));

	// get first byte
	if (antd_recv(client, &byte, sizeof(byte)) <= 0)
		goto fail;
	if (BITV(byte, 6) || BITV(byte, 5) || BITV(byte, 4))
		goto fail; // all RSV bit must be 0

	//printf("FIN: %d, RSV1: %d, RSV2: %d, RSV3:%d, opcode:%d\n", BITV(byte,7), BITV(byte,6), BITV(byte,5), BITV(byte,4),(byte & 0x0F) );
	// find and opcode
	header->fin = BITV(byte, 7);
	header->opcode = (byte & 0x0F);

	// get next byte
	if (antd_recv(client, &byte, sizeof(byte)) <= 0)
		goto fail;

	//printf("MASK: %d paylen:%d\n", BITV(byte,7), (byte & 0x7F));
	// check mask bit, should be 1
	header->mask = BITV(byte, 7);
	/*if(!BITV(byte,7))
	{
		// close the connection with protocol error
		ws_close(client, 1002);
		goto fail;
	}*/
	// get the data length of the frame
	int len = (byte & 0x7F);
	if (len <= 125)
	{
		header->plen = len;
	}
	else if (len == 126)
	{
		if (antd_recv(client, bytes, 2 * sizeof(uint8_t)) <= 0)
			goto fail;
		header->plen = (bytes[0] << 8) + bytes[1];
	}
	else
	{
		//read only last 4 byte
		if (antd_recv(client, bytes, 8 * sizeof(uint8_t)) <= 0)
			goto fail;
		header->plen = (bytes[4] << 24) + (bytes[5] << 16) + (bytes[6] << 8) + bytes[7];
	}
	//printf("len: %d\n", header->plen);
	// last step is to get the maskey
	if (header->mask)
		if (antd_recv(client, header->mask_key, 4 * sizeof(uint8_t)) <= 0)
			goto fail;
	//printf("key 0: %d key 1: %d key2:%d, key3: %d\n",header->mask_key[0],header->mask_key[1],header->mask_key[2], header->mask_key[3] );

	// check wheather it is a ping or a close message
	// process it and return NULL
	//otherwise return the header
	//return the header
	switch (header->opcode)
	{
	case WS_CLOSE: // client requests to close the connection
		// send back a close message
		UNUSED(ws_send_close(client, 1000, header->mask ? 0 : 1));
		//goto fail;
		break;

	case WS_PING: // client send a ping
		// send back a pong message
		UNUSED(ws_pong(client, header, header->mask ? 0 : 1));
		break;

	default:
		break;
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
int ws_read_data(void *client, ws_msg_header_t *header, int len, uint8_t *data)
{
	// if len  == -1 ==> read all remaining data to 'data';
	if (header->plen == 0)
		return 0;
	int dlen = (len == -1 || len > (int)header->plen) ? (int)header->plen : len;
	if ((dlen = antd_recv(client, data, dlen)) <= 0)
		return -1;
	header->plen = header->plen - dlen;
	// unmask received data
	if (header->mask)
		for (int i = 0; i < dlen; ++i)
			data[i] = data[i] ^ header->mask_key[i % 4];
	data[dlen] = '\0';
	return dlen;
}
int _send_header(void *client, ws_msg_header_t header)
{
	uint8_t byte = 0;
	uint8_t bytes[8];
	for (int i = 0; i < 8; i++)
		bytes[i] = 0;
	//first byte |FIN|000|opcode|
	byte = (header.fin << 7) + header.opcode;
	//printf("BYTE: %d\n", byte);
	if (antd_send(client, &byte, 1) != 1)
		return -1;
	// second byte, payload length
	// mask may be 0 or 1
	//if(header.mask == 1)
	//	printf("Data is masked\n");
	if (header.plen <= 125)
	{
		byte = (header.mask << 7) + header.plen;
		if (antd_send(client, &byte, 1) != 1)
			return -1;
	}
	else if (header.plen < 65536) // 16 bits
	{
		byte = (header.mask << 7) + 126;
		bytes[0] = (header.plen) >> 8;
		bytes[1] = (header.plen) & 0x00FF;
		if (antd_send(client, &byte, 1) != 1)
			return -1;
		if (antd_send(client, bytes, 2) != 2)
			return -1;
	}
	else // > 16 bits
	{
		byte = (header.mask << 7) + 127;
		bytes[4] = (header.plen) >> 24;
		bytes[5] = ((header.plen) >> 16) & 0x00FF;
		bytes[6] = ((header.plen) >> 8) & 0x00FF;
		bytes[7] = (header.plen) & 0x00FF;
		if (antd_send(client, &byte, 1) != 1)
			return -1;
		if (antd_send(client, bytes, 8) != 8)
			return -1;
	}
	// send mask key
	if (header.mask)
	{
		if (antd_send(client, header.mask_key, 4) != 4)
			return -1;
	}
	return 0;
}
/**
* Send a frame to client
*/
int ws_send_frame(void *client, uint8_t *data, ws_msg_header_t header)
{
	uint8_t *masked;
	masked = data;
	int ret;
	if (header.mask)
	{
		ws_gen_mask_key(&header);
		masked = (uint8_t *)malloc(header.plen);
		for (int i = 0; i < (int)header.plen; ++i)
			masked[i] = data[i] ^ header.mask_key[i % 4];
	}
	if (_send_header(client, header) != 0)
		return -1;
	if (header.opcode == WS_TEXT)
		ret = antd_send(client, (char *)masked, header.plen);
	else
		ret = antd_send(client, (uint8_t *)masked, header.plen);
	if (masked && header.mask)
		free(masked);
	if (ret != (int)header.plen)
	{
		return -1;
	}
	return 0;
}
/**
* send a text data frame to client
*/
int ws_send_text(void *client, const char *data, int mask)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_TEXT;
	header.mask = mask;
	header.plen = strlen(data);
	//_send_header(client,header);
	//send(client, data, header.plen,0);
	return ws_send_frame(client, (uint8_t *)data, header);
}
/**
* send a single binary data fram to client
* not tested yet, but should work
*/
int ws_send_binary(void *client, uint8_t *data, int l, int mask)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_BIN;
	header.plen = l;
	header.mask = mask;
	return ws_send_frame(client, data, header);
	//_send_header(client,header);
	//send(client, data, header.plen,0);
}
/*
* send a file as binary data
*/
int ws_send_file(void *client, const char *file, int mask)
{
	uint8_t buff[1024];
	FILE *ptr;
	ptr = fopen(file, "rb");
	if (!ptr)
	{
		return ws_send_close(client, 1011, mask);
	}

	ws_msg_header_t header;
	size_t size;
	int first_frame = 1;
	int ret = 0;
	//ws_send_frame(client,buff,header);
	header.mask = mask;
	while (!feof(ptr))
	{
		size = fread(buff, 1, 1024, ptr);
		if (feof(ptr))
			header.fin = 1;
		else
			header.fin = 0;
		// clear opcode
		if (first_frame)
		{
			header.opcode = WS_BIN;
			first_frame = 0;
		}
		else
			header.opcode = 0;
		header.plen = size;
		//printf("FIN: %d OC:%d\n", header.fin, header.opcode);
		ret += ws_send_frame(client, buff, header);
	}
	fclose(ptr);
	if (ret != 0)
	{
		return -1;
	}
	return 0;
}
/**
* Not tested yet
* but should work
*/
int ws_pong(void *client, ws_msg_header_t *oheader, int mask)
{
	ws_msg_header_t pheader;
	int ret;
	pheader.fin = 1;
	pheader.opcode = WS_PONG;
	pheader.plen = oheader->plen;
	pheader.mask = mask;
	uint8_t *data = (uint8_t *)malloc(oheader->plen);
	if (!data)
		return -1;

	if (ws_read_data(client, oheader, pheader.plen, data) == -1)
	{
		ERROR("Cannot read ping data %d", pheader.plen);
		free(data);
		return -1;
	}
	ret = ws_send_frame(client, data, pheader);
	free(data);
	//_send_header(client, pheader);
	//send(client, data, len, 0);
	return ret;
}
int ws_ping(void *client, const char *echo, int mask)
{
	ws_msg_header_t pheader;
	pheader.fin = 1;
	pheader.opcode = WS_PING;
	pheader.plen = strlen(echo);
	pheader.mask = mask;
	return ws_send_frame(client, (uint8_t *)echo, pheader);
}
/*
* Not tested yet, but should work
*/
int ws_send_close(void *client, unsigned int status, int mask)
{
	//printf("CLOSED\n");
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_CLOSE;
	header.plen = 2;
	header.mask = mask;
	uint8_t bytes[2];
	bytes[0] = status >> 8;
	bytes[1] = status & 0xFF;
	/*if(mask)
	{
		// XOR itself
		header.mask_key[0] = bytes[0];
		header.mask_key[1] = bytes[1];
		bytes[0] = bytes[1] ^ bytes[1];
	}*/
	return ws_send_frame(client, bytes, header);
	//_send_header(client, header);
	//send(client,bytes,2,0);
}

void ws_client_close(ws_client_t *wsclient)
{
	antd_close(wsclient->antdsock);

#ifdef USE_OPENSSL
	if (wsclient->ssl_ctx)
	{
		SSL_CTX_free(wsclient->ssl_ctx);
		// DEPRECATED: FIPS_mode_set(0);
		// DEPRECATED: CONF_modules_unload(1);
		EVP_cleanup();
		EVP_PBE_cleanup();
		// DEPRECATED:ENGINE_cleanup();
		CRYPTO_cleanup_all_ex_data();
		// DEPRECATED: ERR_remove_state(0);
		ERR_free_strings();
	}
#endif
}

//this is for the client side, not use for now
int ws_client_connect(ws_client_t *wsclient, port_config_t pcnf)
{
	char* ip = ip_from_hostname(wsclient->host);
	if (ip == NULL)
		return -1;
	int sock = request_socket(ip, pcnf.port);
	if (sock <= 0)
	{
		ERROR("Cannot request socket");
		return -1;
	}
	// time out setting
	struct timeval timeout;
	timeout.tv_sec = CONN_TIME_OUT_S;
	timeout.tv_usec = 0; //3 s
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		ERROR("setsockopt failed:%s", strerror(errno));

	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		ERROR("setsockopt failed:%s", strerror(errno));
        
	// will be free
	wsclient->antdsock->sock = sock;
	wsclient->antdsock->z_status = 0;
	wsclient->antdsock->last_io = time(NULL);
	wsclient->antdsock->zstream = NULL;
#ifdef USE_OPENSSL
	if (pcnf.usessl)
	{
		SSL_library_init();
		SSL_load_error_strings();
		ERR_load_crypto_strings();
		OpenSSL_add_ssl_algorithms();
		const SSL_METHOD *method;
		unsigned long ssl_err = 0;
		method = SSLv23_client_method();
		ssl_err = ERR_get_error();
		if (!method)
		{
			ERROR("SSLv23_method: %s", ERR_error_string(ssl_err, NULL));
			return -1;
		}
		wsclient->ssl_ctx = SSL_CTX_new(method);
		ssl_err = ERR_get_error();
		if (!wsclient->ssl_ctx)
		{
			ERROR("SSL_CTX_new: %s", ERR_error_string(ssl_err, NULL));
			return -1;
		}
		// configure the context
#if defined(SSL_CTX_set_ecdh_auto)
		SSL_CTX_set_ecdh_auto(wsclient->ssl_ctx, 1);
#else
		SSL_CTX_set_tmp_ecdh(wsclient->ssl_ctx, EC_KEY_new_by_curve_name(NID_X9_62_prime256v1));
#endif
		SSL_CTX_set_options(wsclient->ssl_ctx, SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_SSLv2 | SSL_OP_NO_TICKET);
		// set the cipher suit
		const char *suit = wsclient->ciphersuit ? wsclient->ciphersuit : PREFERRED_WS_CIPHERS;
		//const char* suit = "AES128-SHA";
		if (SSL_CTX_set_cipher_list(wsclient->ssl_ctx, suit) != 1)
		{
			ssl_err = ERR_get_error();
			// TODO Close the context
			ERROR("SSL_CTX_set_cipher_list: %s", ERR_error_string(ssl_err, NULL));
			return -1;
		}

		if (wsclient->sslcert && wsclient->sslkey)
		{
			if (SSL_CTX_use_certificate_file(wsclient->ssl_ctx, wsclient->sslcert, SSL_FILETYPE_PEM) <= 0)
			{
				ssl_err = ERR_get_error();
				ERROR("SSL_CTX_use_certificate_file: %s", ERR_error_string(ssl_err, NULL));
				return -1;
			}
			if (wsclient->sslpasswd)
				SSL_CTX_set_default_passwd_cb_userdata(wsclient->ssl_ctx, (void *)wsclient->sslpasswd);
			if (SSL_CTX_use_PrivateKey_file(wsclient->ssl_ctx, wsclient->sslkey, SSL_FILETYPE_PEM) <= 0)
			{
				ssl_err = ERR_get_error();
				ERROR("SSL_CTX_use_PrivateKey_file: %s", ERR_error_string(ssl_err, NULL));
				return -1;
			}
			if (SSL_CTX_check_private_key(wsclient->ssl_ctx) == 0)
			{
				ssl_err = ERR_get_error();
				ERROR("SSL_CTX_check_private_key: %s", ERR_error_string(ssl_err, NULL));
				return -1;
			}
		}
		//

		// validate
		if (wsclient->verify_location)
		{
			SSL_CTX_set_verify(wsclient->ssl_ctx, SSL_VERIFY_PEER, NULL);
			SSL_CTX_set_verify_depth(wsclient->ssl_ctx, 5);
			if (!SSL_CTX_load_verify_locations(wsclient->ssl_ctx, wsclient->verify_location, NULL))
			{
				ssl_err = ERR_get_error();
				// TODO Close the context
				ERROR("SSL_CTX_load_verify_locations: %s", ERR_error_string(ssl_err, NULL));
				return -1;
			}
		}
		else
		{
			SSL_CTX_set_verify(wsclient->ssl_ctx, SSL_VERIFY_NONE, NULL);
		}

		wsclient->antdsock->ssl = (void *)SSL_new(wsclient->ssl_ctx);
		if (!wsclient->antdsock->ssl)
		{
			ssl_err = ERR_get_error();
			ERROR("SSL_new: %s", ERR_error_string(ssl_err, NULL));
			return -1;
		}
		SSL_set_fd((SSL *)wsclient->antdsock->ssl, wsclient->antdsock->sock);
		int stat, ret;
		ERR_clear_error();
		while ((ret = SSL_connect(wsclient->antdsock->ssl)) <= 0)
		{
			stat = SSL_get_error(wsclient->antdsock->ssl, ret);
			switch (stat)
			{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_NONE:
				continue;
			default:
				ERR_print_errors_fp(stderr);
				ERROR("Error performing SSL handshake %d", stat);
				return -1;
			}
		}
	}
#endif
	return 0;
}

int ws_open_handshake(ws_client_t *client)
{
	char buf[MAX_BUFF];
	// now send ws request handshake
	sprintf(buf, CLIENT_RQ, client->resource, client->host);
	//printf("Send %s\n", buf);
	int size = antd_send(client->antdsock, buf, strlen(buf));
	if (size != (int)strlen(buf))
	{
		ERROR("Cannot send request \n");
		return -1;
	}
	// now verify if server accept the socket
	size = read_buf(client->antdsock, buf, MAX_BUFF);
	char *token;
	int done = 0;
	while (size > 0 && strcmp("\r\n", buf))
	{
		char *line = buf;
		token = strsep(&line, ":");
		trim(token, ' ');
		if (token != NULL && strcasecmp(token, "Sec-WebSocket-Accept") == 0)
		{
			token = strsep(&line, ":");
			trim(token, ' ');
			trim(token, '\n');
			trim(token, '\r');
			if (strcasecmp(token, SERVER_WS_KEY) == 0)
			{
				//LOG("Handshake sucessfull\n");
				done = 1;
			}
			else
			{
				ERROR("WS handshake, Wrong key %s vs %s", token, SERVER_WS_KEY);
				return -1;
			}
		}
		//if(line) free(line);
		size = read_buf(client->antdsock, buf, MAX_BUFF);
	}
	if (done)
		return 0;
	return -1;
}
char *get_ip_address()
{
	struct ifaddrs *addrs;
	getifaddrs(&addrs);
	struct ifaddrs *tmp = addrs;
	char *ip;
	while (tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
			ip = inet_ntoa(pAddr->sin_addr);
			if (strcmp(ip, "127.0.0.1") != 0)
				return ip;
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return "127.0.0.1";
}
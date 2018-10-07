#include "ws.h"
static void ws_gen_mask_key(ws_msg_header_t * header)
{
	int r = rand();
	header->mask_key[0]  = (r >> 24) & 0xFF;
	header->mask_key[1]  = (r >> 16) & 0xFF;
	header->mask_key[2]  = (r >> 8) & 0xFF;
	header->mask_key[3]  = r & 0xFF;
}
/**
* Read a frame header
* based on this header, we'll decide
* the appropriate handle for frame data
*/
ws_msg_header_t * ws_read_header(void* client)
{
	
	uint8_t byte = 0;
	uint8_t bytes[8];
	ws_msg_header_t* header = (ws_msg_header_t*) malloc(sizeof(*header));
	
	// get first byte
	if(antd_recv(client, &byte, sizeof(byte)) <0) goto fail;
	if(BITV(byte,6) || BITV(byte,5) || BITV(byte,4)) goto fail;// all RSV bit must be 0
	
	//printf("FIN: %d, RSV1: %d, RSV2: %d, RSV3:%d, opcode:%d\n", BITV(byte,7), BITV(byte,6), BITV(byte,5), BITV(byte,4),(byte & 0x0F) );
	// find and opcode
	header->fin = BITV(byte,7);
	header->opcode = (byte & 0x0F);
	
	// get next byte
	if(antd_recv(client, &byte, sizeof(byte)) <0) goto fail;
	
	//printf("MASK: %d paylen:%d\n", BITV(byte,7), (byte & 0x7F));
	// check mask bit, should be 1
	header->mask = BITV(byte,7);
	/*if(!BITV(byte,7))
	{
		// close the connection with protocol error
		ws_close(client, 1002);
		goto fail;
	}*/
	// get the data length of the frame
	int len = (byte & 0x7F);
	if(len <= 125)
	{
		header->plen = len;
	} else if(len == 126)
	{
		if(antd_recv(client,bytes, 2*sizeof(uint8_t)) <0) goto fail;
		header->plen = (bytes[0]<<8) + bytes[1];
		
	} else
	{
		//read only last 4 byte
		if(antd_recv(client,bytes, 8*sizeof(uint8_t)) <0) goto fail;
		header->plen = (bytes[4]<<24) + (bytes[5]<<16) + (bytes[6] << 8) + bytes[7] ;
	}
	//printf("len: %d\n", header->plen);
	// last step is to get the maskey
	if(header->mask)
		if(antd_recv(client,header->mask_key, 4*sizeof(uint8_t)) <0) goto fail;
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
int ws_read_data(void* client, ws_msg_header_t* header, int len, uint8_t* data)
{
	// if len  == -1 ==> read all remaining data to 'data';
	if(header->plen == 0) return 0;
	int dlen = (len==-1 || len > (int)header->plen)?header->plen:len;
	if((dlen = antd_recv(client,data, dlen)) <0) return -1;
	header->plen = header->plen - dlen;
	// unmask received data
	if(header->mask)
		for(int i = 0; i < dlen; ++i)
			data[i] = data[i]^ header->mask_key[i%4];
	data[dlen] = '\0';
	return dlen;
}
void _send_header(void* client, ws_msg_header_t header)
{
	uint8_t byte = 0;
	uint8_t bytes[8];
	for(int i=0; i< 8; i++) bytes[i] = 0;
	//first byte |FIN|000|opcode|
	byte = (header.fin << 7) + header.opcode;
	//printf("BYTE: %d\n", byte);
	antd_send(client, &byte, 1);
	// second byte, payload length
	// mask may be 0 or 1
	//if(header.mask == 1)
	//	printf("Data is masked\n");
	if(header.plen <= 125)
	{
		byte =  (header.mask << 7) + header.plen;
		antd_send(client, &byte, 1);
	}
	else if(header.plen < 65536) // 16 bits
	{
		byte = (header.mask << 7) + 126;
		bytes[0] = (header.plen) >> 8;
		bytes[1] = (header.plen) & 0x00FF;
		antd_send(client, &byte, 1);
		antd_send(client, &bytes, 2);
	}
	else // > 16 bits
	{
		byte =  (header.mask << 7) + 127;
		bytes[4] = (header.plen) >> 24;
		bytes[5] = ((header.plen)>>16) & 0x00FF;
		bytes[6] = ((header.plen)>>8) & 0x00FF;
		bytes[7] = (header.plen) & 0x00FF;
		antd_send(client, &byte, 1);
		antd_send(client, &bytes, 8);
	}
	// send mask key
	if(header.mask)
	{
		antd_send(client, header.mask_key,4);
	}
}
/**
* Send a frame to client
*/
void ws_send_frame(void* client, uint8_t* data, ws_msg_header_t header)
{
	uint8_t * masked;
	masked = data;
	if(header.mask)
	{
		ws_gen_mask_key(&header);
		masked = (uint8_t*) malloc(header.plen);
		for(int i = 0; i < (int)header.plen; ++i)
			masked[i] = data[i]^ header.mask_key[i%4];
	}
	_send_header(client, header);
	if(header.opcode == WS_TEXT)
		antd_send(client,(char*)masked,header.plen);
	else
		antd_send(client,(uint8_t*)masked,header.plen);
	if(masked && header.mask)
		free(masked);
}
/**
* send a text data frame to client
*/
void ws_send_text(void* client, const char* data,int mask)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_TEXT;
	header.mask = mask;
	header.plen = strlen(data);
	//_send_header(client,header);
	//send(client, data, header.plen,0);
	ws_send_frame(client,(uint8_t*)data,header);
}
/**
* send a single binary data fram to client
* not tested yet, but should work
*/
void ws_send_binary(void* client, uint8_t* data, int l, int mask)
{
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_BIN;
	header.plen = l;
	header.mask = mask;
	ws_send_frame(client,data, header);
	//_send_header(client,header);
	//send(client, data, header.plen,0);
}
/*
* send a file as binary data
*/
void ws_send_file(void* client, const char* file, int mask)
{
	uint8_t buff[1024];
	FILE *ptr;
	ptr = fopen(file,"rb");
	if(!ptr)
	{
		ws_close(client,1011);
		return;
	}

	ws_msg_header_t header;
	size_t size;
	int first_frame = 1;
	//ws_send_frame(client,buff,header);
	header.mask = mask;
	while(!feof(ptr))
	{
		size = fread(buff,1,1024,ptr);
		if(size >= 0)
		{
			if(feof(ptr))
				header.fin = 1;
			else
				header.fin = 0;
			// clear opcode
			if(first_frame)
			{
				header.opcode = WS_BIN;
				first_frame = 0;
			}
			else
				header.opcode = 0;
			header.plen = size;
			//printf("FIN: %d OC:%d\n", header.fin, header.opcode);
			ws_send_frame(client,buff,header);
		} 
	}
	fclose(ptr);
}
/**
* Not tested yet
* but should work
*/
void pong(void* client, int len)
{
	//printf("PONG\n");
	ws_msg_header_t pheader;
	pheader.fin = 1;
	pheader.opcode = WS_PONG;
	pheader.plen = len;
	pheader.mask = 0;
	uint8_t data[len];
	if(antd_recv(client,data, len) < 0) return;
	ws_send_frame(client,data,pheader);
	//_send_header(client, pheader);
	//send(client, data, len, 0);
}
/*
* Not tested yet, but should work
*/
void ws_send_close(void* client, unsigned int status, int mask)
{
	//printf("CLOSED\n");
	ws_msg_header_t header;
	header.fin = 1;
	header.opcode = WS_CLOSE;
	header.plen = 2;
	header.mask=mask;
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
	ws_send_frame(client,bytes,header);
	//_send_header(client, header);
	//send(client,bytes,2,0);
}
int ip_from_hostname(const char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;    
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return -1;
    }
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return -1;
}

/*
send a request
*/
int request_socket(const char* ip, int port)
{
	int sockfd;
	struct sockaddr_in dest;
	
	// time out setting
	struct timeval timeout;      
	timeout.tv_sec = CONN_TIME_OUT_S;
	timeout.tv_usec = 0;//3 s
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		return -1;
	}
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
	        perror("setsockopt failed\n");

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
        perror("setsockopt failed\n");
	/*struct linger lingerStruct;
    lingerStruct.l_onoff = 0;  // turn lingering off for sockets
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lingerStruct, sizeof(lingerStruct));*/
	
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if ( inet_aton(ip, &dest.sin_addr) == 0 )
    {
		perror(ip);
		close(sockfd);
		return -1;
    }
	if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
	{
		close(sockfd);
		perror("Connect");
		return -1;
	}
	return sockfd;
}

//TODO: The ping request
/*
this is for the client side, not use for now
int ws_open_hand_shake(const char* host, int port, const char* resource)
{
    char ip[100];
	char buff[MAX_BUFF];
    char* rq = NULL;
    int size;
    // request socket
	ip_from_hostname(host ,ip);
	int sock = request_socket(ip, port);
	if(sock <= 0) return -1;
    // now send ws request handshake 
    rq = __s(CLIENT_RQ,resource,host);
   //  printf("%s\n",rq);
    size = send(sock, rq, strlen(rq),0);
    if(size != strlen(rq))
    {
        printf("Cannot send request \n");
        close(sock);
        return -1;
    }
    // now verify if server accept the socket 
	 size  = read_buf(sock,buff,MAX_BUFF);
	char* token;
    int done = 0;
	while (size > 0 && strcmp("\r\n",buff))
	{
		char* line = strdup(buff);
		//printf("LINE %s\n", line);
		token = strsep(&line,":");
		trim(token,' ');
		if(token != NULL &&strcasecmp(token,"Sec-WebSocket-Accept") == 0)
		{
			token = strsep(&line,":");
			trim(token,' ');
			trim(token,'\n');
			trim(token,'\r');
            //printf("Key found %s \n", token);
            if(strcasecmp(token, SERVER_WS_KEY) == 0)
            {
               // printf("Handshake sucessfull\n");
                done = 1;
            } else
            {
                printf("Wrong key %s vs %s\n",token,SERVER_WS_KEY);
                close(sock);
                return -1;
            }
		} 
		//if(line) free(line);
		size  = read_buf(sock,buff,MAX_BUFF);
	}
    if(done) return sock;
    //printf("No server key found \n");
    return -1;
}*/
char* get_ip_address()
{
	struct ifaddrs* addrs;
	getifaddrs(&addrs);
	struct ifaddrs* tmp = addrs;
	char* ip;
	while (tmp) 
	{
	    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
	    {
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
	        ip = inet_ntoa(pAddr->sin_addr);
			if(strcmp(ip,"127.0.0.1") != 0) 
				return ip;
	    }
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return "127.0.0.1";
}
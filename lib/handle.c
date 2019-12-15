#include "handle.h" 
#define HTML_TPL "<HTML><HEAD><TITLE>%s</TITLE></HEAD><BODY><h2>%s</h2></BODY></HTML>"
#ifdef USE_OPENSSL

static const char* S_100 =  "Continue";
static const char* S_101 =  "Switching Protocols";
static const char* S_102 =  "Processing";
static const char* S_103 =  "Early Hints"; 

static const char* S_200 =  "OK";
static const char* S_201 =  "Created";
static const char* S_202 =  "Accepted";
static const char* S_203 =  "Non-Authoritative Information";
static const char* S_204 =  "No Content";
static const char* S_205 =  "Reset Content";
static const char* S_206 =  "Partial Content";
static const char* S_207 =  "Multi-Status";
static const char* S_208 =  "Already Reported";
static const char* S_226 =  "IM Used"; 

static const char* S_300 =  "Multiple Choices";
static const char* S_301 =  "Moved Permanently";
static const char* S_302 =  "Found";
static const char* S_303 =  "See Other";
static const char* S_304 =  "Not Modified";
static const char* S_305 =  "Use Proxy";
static const char* S_306 =  "Switch Proxy";
static const char* S_307 =  "Temporary Redirect";
static const char* S_308 =  "Permanent Redirect"; 

static const char* S_400 =  "Bad Request";
static const char* S_401 =  "Unauthorized";
static const char* S_402 =  "Payment Required";
static const char* S_403 =  "Forbidden";
static const char* S_404 =  "Not Found";
static const char* S_405 =  "Method Not Allowed";
static const char* S_406 =  "Not Acceptable";
static const char* S_407 =  "Proxy Authentication Required";
static const char* S_408 =  "Request Timeout";
static const char* S_409 =  "Conflict";
static const char* S_410 =  "Gone";
static const char* S_411 =  "Length Required";
static const char* S_412 =  "Precondition Failed";
static const char* S_413 =  "Payload Too Large";
static const char* S_414 =  "URI Too Long";
static const char* S_415 =  "Unsupported Media Type";
static const char* S_416 =  "Range Not Satisfiable";
static const char* S_417 =  "Expectation Failed";
static const char* S_421 =  "Misdirected Request";
static const char* S_422 =  "Unprocessable Entity";
static const char* S_423 =  "Locked";
static const char* S_424 =  "Failed Dependency";
static const char* S_425 =  "Too Early";
static const char* S_426 =  "Upgrade Required";
static const char* S_428 =  "Precondition Required";
static const char* S_429 =  "Too Many Requests";
static const char* S_431 =  "Request Header Fields Too Large";
static const char* S_451 =  "Unavailable For Legal Reasons"; 

static const char* S_500 =  "Internal Server Error";
static const char* S_501 =  "Not Implemented";
static const char* S_502 =  "Bad Gateway";
static const char* S_503 =  "Service Unavailable";
static const char* S_504 =  "Gateway Timeout";
static const char* S_505 =  "HTTP Version Not Supported";
static const char* S_506 =  "Variant Also Negotiates";
static const char* S_507 =  "Insufficient Storage";
static const char* S_508 =  "Loop Detected";
static const char* S_510 =  "Not Extended";
static const char* S_511 =  "Network Authentication Required";
static const char* S_UNOF =  "Unofficial Status";

int usessl()
{
	return 0;
}
#endif

void error_log(const char* fmt, ...)
{
	UNUSED(fmt);
	return;
}
#ifdef DEBUG
void server_log(const char* fmt, ...)
{
	UNUSED(fmt);
	return;
}
#endif

const char* get_status_str(int stat)
{
	switch(stat)
	{
		case 100: return S_100;
		case 101: return S_101;
		case 102: return S_102;
		case 103: return S_103;

		case 200: return S_200;
		case 201: return S_201;
		case 202: return S_202;
		case 203: return S_203;
		case 204: return S_204;
		case 205: return S_205;
		case 206: return S_206;
		case 207: return S_207;
		case 208: return S_208;
		case 226: return S_226;

		case 300: return S_300;
		case 301: return S_301;
		case 302: return S_302;
		case 303: return S_303;
		case 304: return S_304;
		case 305: return S_305;
		case 306: return S_306;
		case 307: return S_307;
		case 308: return S_308;

		case 400: return S_400;
		case 401: return S_401;
		case 402: return S_402;
		case 403: return S_403;
		case 404: return S_404;
		case 405: return S_405;
		case 406: return S_406;
		case 407: return S_407;
		case 408: return S_408;
		case 409: return S_409;
		case 410: return S_410;
		case 411: return S_411;
		case 412: return S_412;
		case 413: return S_413;
		case 414: return S_414;
		case 415: return S_415;
		case 416: return S_416;
		case 417: return S_417;
		case 421: return S_421;
		case 422: return S_422;
		case 423: return S_423;
		case 424: return S_424;
		case 425: return S_425;
		case 426: return S_426;
		case 428: return S_428;
		case 429: return S_429;
		case 431: return S_431;
		case 451: return S_451;

		case 500: return S_500;
		case 501: return S_501;
		case 502: return S_502;
		case 503: return S_503;
		case 504: return S_504;
		case 505: return S_505;
		case 506: return S_506;
		case 507: return S_507;
		case 508: return S_508;
		case 510: return S_510;
		case 511: return S_511;
		default: return S_UNOF;
	}
}

void antd_send_header(void* client, antd_response_header_t* res)
{
	if(!res->header)
		res->header = dict();
	dput(res->header,"Server", strdup(SERVER_NAME));
	const char* stat_str = get_status_str(res->status);
	__t(client, "HTTP/1.1 %d %s", res->status, stat_str);
	chain_t it;
	for_each_assoc(it, res->header)
	{
		__t(client,"%s: %s", it->key, (const char*)it->value);
	}
	// send out cookie
	if(res->cookie)
	{
		int size = list_size(res->cookie);
		for (int i = 0; i < size; i++)
		{
			__t(client,"Set-Cookie: %s", list_at(res->cookie, i)->value.s);
		}
		list_free(&res->cookie);
		res->cookie = NULL;
	}
	__b(client, (unsigned char*)"\r\n", 2);
	freedict(res->header);
	res->header = NULL;
}

/*
void octstream(void* client, char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: application/octet-stream");
	__t(client,"Content-Disposition: attachment; filename=\"%s\"", name);
	response(client,"");
	//Content-Disposition: attachment; filename="fname.ext"
}*/

int antd_send(void *src, const void* data, int len)
{
	if(!src || !data) return -1;
	int written;
	antd_client_t * source = (antd_client_t *) src;
	char* ptr;
	int writelen = 0;
	int  count;
#ifdef USE_OPENSSL
	if(usessl())
	{
		//LOG("SSL WRITE\n");
		//ret = SSL_write((SSL*) source->ssl, data, len);
		ptr = (char* )data;
		writelen = len > BUFFLEN?BUFFLEN:len;
		written = 0;
		fd_set fds;
    	struct timeval timeout;
		while (writelen > 0) //source->attempt < MAX_ATTEMPT
        {
			// clear the error queue
			ERR_clear_error();
            count = SSL_write (source->ssl, ptr+written, writelen);
            if (count > 0)
            {
                written += count;
				writelen = (len - written) > BUFFLEN?BUFFLEN:(len-written);
            }
            else 
            {
                //printf(" received equal to or less than 0\n")
                int err = SSL_get_error(source->ssl, count);
                switch (err)
                {
                    case SSL_ERROR_NONE:
                    {
                        // no real error, just try again...
                        //LOG("SSL_ERROR_NONE \n");
						//source->attempt++;
                        continue;
                    }   

                    case SSL_ERROR_ZERO_RETURN: 
                    {
                        // peer disconnected...
                        ERROR("SSLWRITE: SSL_ERROR_ZERO_RETURN: peer disconected: %d", source->sock);
                        break;
                    }   

                    case SSL_ERROR_WANT_READ: 
                    {
                        // no data available right now, wait a few seconds in case new data arrives...
                        //printf("SSL_ERROR_WANT_READ\n");

                        int sock = SSL_get_rfd(source->ssl);
                        FD_ZERO(&fds);
                        FD_SET(sock, &fds);

                        timeout.tv_sec = 0;
                        timeout.tv_usec = 500;
                        err = select(sock+1, &fds, NULL, NULL, &timeout);
                        if (err == 0 || (err > 0 && FD_ISSET(sock, &fds)))
						{
							//source->attempt++;
                            continue; // more data to read...
						}
						//ERROR("SSL WRITE: want read but select error on the socket %d: %s", source->sock, strerror(errno));
						break;
                    }

                    case SSL_ERROR_WANT_WRITE: 
                    {
                        // socket not writable right now, wait a few seconds and try again...
                        //printf("SSL_ERROR_WANT_WRITE \n");
                        int sock = SSL_get_wfd(source->ssl);
                        FD_ZERO(&fds);
                        FD_SET(sock, &fds);

                        timeout.tv_sec = 0;
                        timeout.tv_usec = 500;

                        err = select(sock+1, NULL, &fds, NULL, &timeout);
                        if (err == 0 || (err > 0 && FD_ISSET(sock, &fds)))
						{
							//source->attempt++;
                            continue; // can write more data now...
						}
						//ERROR("SSL WRITE: want write but select error on the socket %d: %s", source->sock, strerror(errno));
					    break;
                    }

                    default:
                    {
                        // other error
						//ERROR("SSL WRITE: Unknown error on %d: %s", source->sock, ERR_error_string(ERR_get_error(), NULL) );
                        break;
                    }
                }     

                break;
				if(written == 0)
					written = count;
            }
		}
		//source->attempt = 0;
	}
	else
	{
#endif
		ptr = (char* )data;
		writelen = len > BUFFLEN?BUFFLEN:len;
		written = 0;
		while (writelen > 0)
        {
            count = send(source->sock, ptr+written, writelen, 0);
            if (count > 0)
            {
                written += count;
				writelen = (len - written) > BUFFLEN?BUFFLEN:(len-written);
            }
			else if(count == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
			{
				if(written == 0)
					written = count;
				//ERROR("Error while writing: %s", strerror(errno));
				break;
				//return written;
			}
		}
#ifdef USE_OPENSSL
	}
#endif
	/*if(ret <= 0)
	{
		antd_close(src);
	}*/
	if(written > 0)
		time(&source->last_io);
	return written;
}
int antd_recv(void *src,  void* data, int len)
{
	if(!src) return -1;
	int read=0;
	char* ptr = NULL;
	int  received=0;
	int readlen=0;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(usessl())
	{
		ptr = (char* )data;
		readlen = len > BUFFLEN?BUFFLEN:len;
		read = 0;
		fd_set fds;
    	struct timeval timeout;
		while (readlen > 0 )//&& source->attempt < MAX_ATTEMPT
        {
			ERR_clear_error();
            received = SSL_read (source->ssl, ptr+read, readlen);
            if (received > 0)
            {
                read += received;
				readlen = (len - read) > BUFFLEN?BUFFLEN:(len-read);
            }
            else 
            {
                //printf(" received equal to or less than 0\n")
                int err = SSL_get_error(source->ssl, received);
                switch (err)
                {
                    case SSL_ERROR_NONE:
                    {
                        // no real error, just try again...
                        //LOG("SSL_ERROR_NONE \n");
						//source->attempt++;
                        continue;
                    }   

                    case SSL_ERROR_ZERO_RETURN: 
                    {
                        // peer disconnected...
                        //ERROR("SSL READ: SSL_ERROR_ZERO_RETURN, peer disconnected %d", source->sock);
                        break;
                    }   

                    case SSL_ERROR_WANT_READ: 
                    {
                        // no data available right now, wait a few seconds in case new data arrives...
                        //printf("SSL_ERROR_WANT_READ\n");

                        int sock = SSL_get_rfd(source->ssl);
                        FD_ZERO(&fds);
                        FD_SET(sock, &fds);

                        timeout.tv_sec = 0;
                        timeout.tv_usec = 500;
                        err = select(sock+1, &fds, NULL, NULL, &timeout);
                        if (err == 0 || (err > 0 && FD_ISSET(sock, &fds)))
						{
							//source->attempt++;
                            continue; // more data to read...
						}
						//ERROR("SSL READ: want read but select error on the socket %d: %s", source->sock, strerror(errno));
						break;
                    }

                    case SSL_ERROR_WANT_WRITE: 
                    {
                        // socket not writable right now, wait a few seconds and try again...
                        //printf("SSL_ERROR_WANT_WRITE \n");
                        int sock = SSL_get_wfd(source->ssl);
                        FD_ZERO(&fds);
                        FD_SET(sock, &fds);

                        timeout.tv_sec = 0;
                        timeout.tv_usec = 500;

                        err = select(sock+1, NULL, &fds, NULL, &timeout);
                        if (err == 0 || (err > 0 && FD_ISSET(sock, &fds)))
						{
							//source->attempt++;
                            continue; // can write more data now...
						}
						//ERROR("SSL READ: want write but select error on the socket %d: %s", source->sock, strerror(errno));
						break;
                    }

                    default:
                    {
                        // other error 
						//ERROR("SSL READ: unkown error on %d: %s", source->sock, ERR_error_string(ERR_get_error(), NULL));
                        break;
                    }
                }     
				if(read ==0)
					read = received;
                break;
            }
        }
		//source->attempt = 0;
		/*
		int stat, r, st;
		do{
			ret = SSL_read((SSL*) source->ssl, data, len);
			stat = SSL_get_error((SSL*)source->ssl, r);
		} while(ret == -1 && 
			(
			stat == SSL_ERROR_WANT_READ || 
			stat == SSL_ERROR_WANT_WRITE ||  
			stat == SSL_ERROR_NONE ||  
			(stat == SSL_ERROR_SYSCALL && r== 0 && !ERR_get_error()) 
			));
		if(ret == -1)
		{
			LOG("Problem reading %d %d %d\n", ret, stat, r);
		}
		//set_nonblock(source->sock);
		*/
	}
	else
	{
#endif
		ptr = (char* )data;
		readlen = len > BUFFLEN?BUFFLEN:len;
		read = 0;
		while (readlen > 0 )//&& source->attempt < MAX_ATTEMPT
        {
            received = recv(((int) source->sock), ptr+read, readlen, 0);
			//LOG("Read : %c\n", *ptr);
            if (received > 0)
            {
                read += received;
				readlen = (len - read) > BUFFLEN?BUFFLEN:(len-read);
				//LOG("Read len is %d\n", readlen);
            }
			else if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
				ERROR("Error while writing: %s", strerror(errno));
				if(read ==0)
					read = received;
				break;
			}
		}
		//read = recv(((int) source->sock), data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
	//LOG("Received %d bytes\n", read);
	/*if(ret == 0)
	{
		antd_close(src);
	}*/
	if(read > 0)
		time(&source->last_io);
	return read;
}
void set_nonblock(int socket) {
    int flags;
    flags = fcntl(socket,F_GETFL,0);
    //assert(flags != -1);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}
/*void set_block()
{
	int flags;
    flags = fcntl(socket,F_GETFL,0);
    //assert(flags != -1);
    fcntl(socket, F_SETFL, flags & (~O_NONBLOCK));
}*/
int antd_close(void* src)
{
	if(!src) return -1;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(source->ssl && usessl()){
		//printf("SSL:Shutdown ssl\n");
        //SSL_shutdown((SSL*) source->ssl);
		SSL_set_shutdown((SSL*) source->ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
		//printf("SSL:Free ssl\n");
		SSL_free((SSL*) source->ssl);
		
		//EVP_cleanup();
		//ENGINE_cleanup();
		CRYPTO_cleanup_all_ex_data();
		ERR_remove_state(0);
		ERR_free_strings();
		source->ssl = NULL;
		//LOG("Freeing SSL\n");
	}
#endif
	//printf("Close sock %d\n", source->sock);
	int ret = close(source->sock);
	if(source->ip) free(source->ip);	
	free(src);
	src = NULL;
	return ret;
}

int __t(void* client, const char* fstring,...)
{
	int dlen;
	int st;
	va_list arguments;      
    char * data;
    va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments)+1;
    va_end(arguments); 
    if ((data = (char*)malloc(dlen*sizeof(char))) != 0)
    {
        va_start(arguments, fstring);
        vsnprintf(data, dlen, fstring, arguments);
        va_end(arguments);
		st = __b(client, (const unsigned char*)data, strlen(data));
		if(st)
			__b(client, (unsigned char*)"\r\n", 2);
        free(data);
		return st;
    }
	return 0;
	//
}
int __b(void* client, const unsigned char* data, int size)
{
	char buf[BUFFLEN];
	int sent = 0;
	int buflen = 0;
	int nbytes = 0;

	/*if(size <= BUFFLEN)
	{
		nbytes = antd_send(client,data,size);
		return (nbytes==-1?0:1);
	}
	else
	{*/
		while(sent < size)
		{
			if(size - sent > BUFFLEN)
				buflen = BUFFLEN;
			else
				buflen = size - sent;
			memcpy(buf,data+sent,buflen);
			nbytes = antd_send(client,buf,buflen);
			if(nbytes == -1)
			{
				return 0;
			}
			sent += nbytes;
		}	
	//}
	return 1;
}
int __f(void* client, const char* file)
{
	unsigned char buffer[BUFFLEN];
	FILE *ptr;
	ptr = fopen(file,"rb");
	if(!ptr)
	{
		LOG("Cannot read : %s", file);
		return 0;
	}
	size_t size;
	while(!feof(ptr))
	{
		size = fread(buffer,1,BUFFLEN,ptr);
		if(!__b(client,buffer,size)) return 0;
	}
	fclose(ptr);
	return 1;
}

int upload(const char* tmp, const char* path)
{
	return !rename(tmp, path);
}

/*
void set_cookie(void* client,const char* type, dictionary_t dic, const char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	chain_t assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s; Path=/%s",assoc->key, (char*)assoc->value, name);
	}
	response(client,"");
}
void clear_cookie(void* client,  dictionary_t dic)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: text/html; charset=utf-8");
	chain_t assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s;expires=%s",assoc->key, (char*)assoc->value, server_time());
	}
	response(client,"");
}
*/

void antd_error(void* client, int status, const char* msg)
{
	antd_response_header_t rsh;
	rsh.header = dict();
	rsh.cookie = NULL;
	const char* stat_str = get_status_str(status);
	rsh.status = status;
	dput(rsh.header, "Content-Type", strdup("text/html; charset=utf-8"));
	char * res_str = __s(HTML_TPL, stat_str, msg);
	int clen = 0;
	if(res_str)
	{
		clen = strlen(res_str);
	}
	char ibuf[20];
    snprintf (ibuf, sizeof(ibuf), "%d",clen);
	dput(rsh.header, "Content-Length", strdup(ibuf));
	antd_send_header(client, &rsh);
	if(res_str)
	{
		//printf("%s\n", res_str);
		__b(client, (unsigned char*)res_str, clen);
		//__t(client, HTML_TPL, stat_str, msg);
		free(res_str);
	}
}


int ws_enable(dictionary_t dic)
{
	if(!dic) return 0;
	char*v = (char*)dvalue(dic, "__web_socket__");
	if(!v) return 0;
	return atoi(v) == 1;
}
/**
 * read the request as a string line format
 * @param  sock socket
 * @return      a request string
 */
char* read_line(void* sock)
{
	char buf[BUFFLEN];
	read_buf(sock,buf,sizeof(buf));
	return strdup(buf);
}
/**
 * Read the socket request in to a buffer or size
 * The data is read until the buffer is full or
 * there are a carrier return character
 * @param  sock socket
 * @param  buf  buffer
 * @param  size size of buffer
 * @return      number of bytes read
 */
int read_buf(void* sock, char*buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n'))
	{
		n = antd_recv(sock, &c, 1);
		if (n > 0)
		{
			//LOG("Data : %c\n", c);
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}
/*
	We put it here since we want the plugin is able
	to destroy the request if it want to
	in this case, the plugin should return an empty
	with no data
*/
void destroy_request(void *data)
{
	if (!data)
		return;
	antd_request_t *rq = (antd_request_t *)data;
	LOG("Close request %d", rq->client->sock);
	// free all other thing
	if (rq->request)
	{
		dictionary_t tmp = dvalue(rq->request, "COOKIE");
		if (tmp)
			freedict(tmp);
		tmp = dvalue(rq->request, "REQUEST_HEADER");
		if (tmp)
			freedict(tmp);
		tmp = dvalue(rq->request, "REQUEST_DATA");
		if (tmp)
			freedict(tmp);
		dput(rq->request, "REQUEST_HEADER", NULL);
		dput(rq->request, "REQUEST_DATA", NULL);
		dput(rq->request, "COOKIE", NULL);
		freedict(rq->request);
	}
	antd_close(rq->client);
	free(rq);
}
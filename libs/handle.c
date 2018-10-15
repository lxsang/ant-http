#include "handle.h" 
#ifdef USE_OPENSSL
int usessl()
{
	return 0;
}
#endif

void set_status(void* client,int code,const char* msg)
{
	char *s = __s("HTTP/1.1 %d %s", code, msg);
	response(client, s);
	free(s);
	s = __s("Server: %s ", SERVER_NAME);
	response(client, s);
	free(s);
}
void redirect(void* client,const char*path)
{
	__t(client,"<html><head><meta http-equiv=\"refresh\" content=\"0; url=%s\"></head><body></body></html>",path);
}

void html(void* client)
{
	ctype(client,"text/html; charset=utf-8");
}
void text(void* client)
{
	ctype(client,"text/plain; charset=utf-8");
}
void json(void* client)
{
	ctype(client,"application/json");
}
void textstream(void* client)
{
	ctype(client, "text/event-stream");
}
void octstream(void* client, char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: application/octet-stream");
	__t(client,"Content-Disposition: attachment; filename=\"%s\"", name);
	response(client,"");
	//Content-Disposition: attachment; filename="fname.ext"
}
void jpeg(void* client)
{
	ctype(client,"image/jpeg");
}
void ctype(void* client, const char* type)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	response(client,"");
}

int response(void* client, const char* data)
{
	char buf[BUFFLEN+3];
	strcpy(buf, data);
	int nbytes;
	int size = strlen(data);
	buf[size] = '\r';
	buf[size+1] = '\n';
	buf[size+2] = '\0'; 
	
	nbytes = antd_send(client, buf, strlen(buf));
	return (nbytes ==-1?0:1);
}
int antd_send(void *src, const void* data, int len)
{
	if(!src || !data) return -1;
	int written;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(usessl())
	{
		//LOG("SSL WRITE\n");
		//ret = SSL_write((SSL*) source->ssl, data, len);
		int  count;
		char* ptr = (char* )data;
		int writelen = len > BUFFLEN?BUFFLEN:len;
		written = 0;
		fd_set fds;
    	struct timeval timeout;
		while (writelen > 0) //source->attempt < MAX_ATTEMPT
        {
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
                        //printf("SSL_ERROR_ZERO_RETURN \n");
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
					    break;
                    }

                    default:
                    {
                        // other error 
                        break;
                    }
                }     

                break;
            }
		}
		//source->attempt = 0;
	}
	else
	{
#endif
		written = send(source->sock, data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
	/*if(ret <= 0)
	{
		antd_close(src);
	}*/
	return written;
}
int antd_recv(void *src,  void* data, int len)
{
	if(!src) return -1;
	int read;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(usessl())
	{
		int  received;
		char* ptr = (char* )data;
		int readlen = len > BUFFLEN?BUFFLEN:len;
		read = 0;
		fd_set fds;
    	struct timeval timeout;
		while (readlen > 0 )//&& source->attempt < MAX_ATTEMPT
        {
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
                        //printf("SSL_ERROR_ZERO_RETURN \n");
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
						break;
                    }

                    default:
                    {
                        // other error 
                        break;
                    }
                }     

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
		read = recv(((int) source->sock), data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
	/*if(ret == 0)
	{
		antd_close(src);
	}*/
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
int __ti(void* client,int data)
{
	char str[15];
	sprintf(str, "%d", data);
	return response(client,str);
}

int __t(void* client, const char* fstring,...)
{
	int nbytes;
	int dlen;
	int sent = 0;
	int buflen = 0;
	va_list arguments;      
    char * data;
    char* chunk;
    va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments) + 1;
    va_end(arguments); 
    if ((data = (char*)malloc(dlen*sizeof(char))) != 0)
    {
        va_start(arguments, fstring);
        vsnprintf(data, dlen, fstring, arguments);
        va_end(arguments);
        
        if(dlen < BUFFLEN) 
		{
			int ret = response(client,data);
			free(data);
			return ret;
		}
		else
		{
			while(sent < dlen - 1)
			{
				if(dlen - sent > BUFFLEN)
					buflen = BUFFLEN;
				else
					buflen = dlen - sent - 1;
				//LOG("BUFFLEN %d\n",buflen);
				chunk = (char*) malloc((buflen)*sizeof(char));
				memcpy(chunk,data+sent,buflen);
				//chunk[buflen-1] = '\0';
				//response(client,chunk);
				sent += buflen;
				nbytes = antd_send(client, chunk, buflen);
				free(chunk);	
				if(nbytes == -1)
				{
					//free(data);
					//return 0;
					break;
				}
			}
			chunk = "\r\n";
			antd_send(client, chunk, strlen(chunk));
		}
        free(data);
    }
	return 1;
	//
}
int __b(void* client, const unsigned char* data, int size)
{
	char buf[BUFFLEN];
	int sent = 0;
	int buflen = 0;
	int nbytes;

	if(size <= BUFFLEN)
	{
		nbytes = antd_send(client,data,size);
		return (nbytes==-1?0:1);
	}
	else
	{
		while(sent < size)
		{
			if(size - sent > BUFFLEN)
				buflen = BUFFLEN;
			else
				buflen = size - sent;
			memcpy(buf,data+sent,buflen);
			nbytes = antd_send(client,buf,buflen);
			sent += buflen;
			if(nbytes == -1) return 0;
		}	
	}
	return 1;
}
int __fb(void* client, const char* file)
{
	printf("Open file %s\n",file );
	unsigned char buffer[BUFFLEN];
	FILE *ptr;
	ptr = fopen(file,"rb");
	if(!ptr)
	{
		LOG("Cannot read : %s\n", file);
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
int __f(void* client, const char* file)
{
	char buf[BUFFLEN];
	FILE *ptr;
	int nbytes;
	ptr = fopen(file,"r");
	if(!ptr)
	{
		LOG("Cannot read : %s\n", file);
		return 0;
	}
	while(fgets(buf, sizeof(buf), ptr) != NULL)
	{
		nbytes = antd_send(client, buf, strlen(buf));
		if(nbytes == -1) return 0;
		//LOG("READ : %s\n", buf);
		//fgets(buf, sizeof(buf), ptr);
	}
	fclose(ptr);
	return 1;
}

int upload(const char* tmp, const char* path)
{
	return !rename(tmp, path);
}
// __plugin__.name
void set_cookie(void* client,const char* type, dictionary dic, const char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s; Path=/%s",assoc->key, (char*)assoc->value, name);
	}
	response(client,"");
}
void clear_cookie(void* client,  dictionary dic)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: text/html; charset=utf-8");
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s;expires=",assoc->key, (char*)assoc->value, server_time());
	}
	response(client,"");
}
void unknow(void* client)
{
	set_status(client,520,"Unknown Error");
	__t(client,"Content-Type: text/html; charset=utf-8");
	response(client,"");
	__t(client,"520 Unknow request");
}
void notfound(void* client)
{
	set_status(client,404,"Not found");
	__t(client,"Content-Type: text/html; charset=utf-8");
	response(client,"");
	__t(client,"Resource not found");
}
void badrequest(void* client)
{
	set_status(client,400,"Bad request");
	__t(client,"Content-Type: text/html; charset=utf-8");
	response(client,"");
	__t(client,"400 Bad request");
}
void unimplemented(void* client)
{
	set_status(client,501,"Method Not Implemented");
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client, "<HTML><HEAD><TITLE>Method Not Implemented");
	__t(client, "</TITLE></HEAD>");
	__t(client, "<BODY><P>HTTP request method not supported.");
	__t(client, "</BODY></HTML>");
}
void cannot_execute(void* client)
{
	set_status(client,500,"Internal Server Error");
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client, "<P>Error prohibited CGI execution.");
}
int ws_enable(dictionary dic)
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
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}
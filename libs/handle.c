#include "handle.h"
config_t server_config; 
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
	int ret;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(usessl())
	{
		//LOG("SSL WRITE\n");
		ret = SSL_write((SSL*) source->ssl, data, len);
	}
	else
	{
#endif
		ret = send(source->sock, data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
	/*if(ret <= 0)
	{
		antd_close(src);
	}*/
	return ret;
}
int antd_recv(void *src,  void* data, int len)
{
	if(!src) return -1;
	int ret;
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(usessl())
	{
		//LOG("SSL READ\n");
		ret = SSL_read((SSL*) source->ssl, data, len);
	}
	else
	{
#endif
		ret = recv(((int) source->sock), data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
	/*if(ret == 0)
	{
		antd_close(src);
	}*/
	return ret;
}
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
		//LOG("Freeing SSL\n");
	}
#endif
	//printf("Close sock %d\n", source->sock);
	int ret = close(source->sock);
	if(source->ip) free(source->ip);
	// TODO remove this when using nonblocking
	server_config.connection--; 
	LOG("Remaining connection %d\n", server_config.connection);
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
	unsigned char buf[BUFFLEN];
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
	html(client);
	__t(client,"404 API not found");
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
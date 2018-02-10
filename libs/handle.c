#include "handle.h"

#ifdef USE_OPENSSL
int usessl()
{
	return 0;
}
#endif

void set_status(void* client,int code,const char* msg)
{
	response(client, __s("HTTP/1.1 %d %s", code, msg));
	response(client, __s("Server: %s ", SERVER_NAME));
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
	int _ssl = 0;
#ifdef USE_OPENSSL
	_ssl = usessl();
#endif
	nbytes = antd_send(client, buf, strlen(buf), _ssl);
	return (nbytes ==-1?0:1);
}
int antd_send(const void *src, const void* data, int len, int _ssl)
{
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(_ssl)
	{
		return SSL_write((SSL*) source->ssl, data, len);
	}
	else
	{
#endif
		return send(source->sock, data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
}
int antd_recv(const void *src,  void* data, int len, int _ssl)
{
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(_ssl)
	{
		return SSL_read((SSL*) source->ssl, data, len);
	}
	else
	{
#endif
		return recv(((int) source->sock), data, len, 0);
#ifdef USE_OPENSSL
	}
#endif
}
int antd_close(void* src)
{
	antd_client_t * source = (antd_client_t *) src;
#ifdef USE_OPENSSL
	if(source->ssl && usessl()){
		SSL_free((SSL*) source->ssl);
		LOG("Freeing SSL\n");
	}
#endif
	printf("Close sock %d\n", source->sock);
	close(source->sock);
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
int _ssl = 0;
#ifdef USE_OPENSSL
	_ssl = usessl();
#endif
    if ((data = (char*)malloc(dlen*sizeof(char))) != 0)
    {
        va_start(arguments, fstring);
        vsnprintf(data, dlen, fstring, arguments);
        va_end(arguments);
        
        if(dlen < BUFFLEN) 
			return response(client,data);
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
				nbytes = antd_send(client, chunk, buflen, _ssl);
				free(chunk);	
				if(nbytes == -1) return 0;
			}
			chunk = "\r\n";
			antd_send(client, chunk, strlen(chunk), _ssl);
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
int _ssl = 0;
#ifdef USE_OPENSSL
	_ssl = usessl();
#endif
	if(size <= BUFFLEN)
	{
		nbytes = antd_send(client,data,size,_ssl);
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
			nbytes = antd_send(client,buf,buflen,_ssl);
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
	int _ssl = 0;
#ifdef USE_OPENSSL
	_ssl = usessl();
#endif
	while(fgets(buf, sizeof(buf), ptr) != NULL)
	{
		nbytes = antd_send(client, buf, strlen(buf), _ssl);
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
	return (dic != NULL && R_INT(dic,"__web_socket__") == 1);
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
int _ssl = 0;
#ifdef USE_OPENSSL
	_ssl = usessl();
#endif
	while ((i < size - 1) && (c != '\n'))
	{
		n = antd_recv(sock, &c, 1, _ssl);
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
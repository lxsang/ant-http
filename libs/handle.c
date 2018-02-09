#include "handle.h"


void set_status(int client,int code,const char* msg)
{
	response(client, __s("HTTP/1.1 %d %s", code, msg));
	response(client, __s("Server: %s ", SERVER_NAME));
}
void redirect(int client,const char*path)
{
	__t(client,"<html><head><meta http-equiv=\"refresh\" content=\"0; url=%s\"></head><body></body></html>",path);
}

void html(int client)
{
	ctype(client,"text/html; charset=utf-8");
}
void text(int client)
{
	ctype(client,"text/plain; charset=utf-8");
}
void json(int client)
{
	ctype(client,"application/json");
}
void textstream(int client)
{
	ctype(client, "text/event-stream");
}
void octstream(int client, char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: application/octet-stream");
	__t(client,"Content-Disposition: attachment; filename=\"%s\"", name);
	response(client,"");
	//Content-Disposition: attachment; filename="fname.ext"
}
void jpeg(int client)
{
	ctype(client,"image/jpeg");
}
void ctype(int client, const char* type)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	response(client,"");
}

int response(int client, const char* data)
{
	char buf[BUFFLEN+3];
	strcpy(buf, data);
	int nbytes;
	int size = strlen(data);
	buf[size] = '\r';
	buf[size+1] = '\n';
	buf[size+2] = '\0';
	nbytes = send(client, buf, strlen(buf), 0);
	return (nbytes ==-1?0:1);
}
int __ti(int client,int data)
{
	char str[15];
	sprintf(str, "%d", data);
	return response(client,str);
}

int __t(int client, const char* fstring,...)
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
				nbytes = send(client, chunk, buflen, 0);
				free(chunk);	
				if(nbytes == -1) return 0;
			}
			chunk = "\r\n";
			send(client, chunk, strlen(chunk), 0);
		}
        free(data);
    }
	return 1;
	//
}
int __b(int client, const unsigned char* data, int size)
{
	char buf[BUFFLEN];
	int sent = 0;
	int buflen = 0;
	int nbytes;
	if(size <= BUFFLEN)
	{
		nbytes = send(client,data,size,0);
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
			nbytes = send(client,buf,buflen,0);
			sent += buflen;
			if(nbytes == -1) return 0;
		}	
	}
	return 1;
}
int __fb(int client, const char* file)
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
int __f(int client, const char* file)
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
	;
	while(fgets(buf, sizeof(buf), ptr) != NULL)
	{
		nbytes = send(client, buf, strlen(buf), 0);
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
void set_cookie(int client,const char* type, dictionary dic, const char* name)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s; Path=/%s",assoc->key, (char*)assoc->value, name);
	}
	response(client,"");
}
void clear_cookie(int client,  dictionary dic)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: text/html; charset=utf-8");
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s;expires=",assoc->key, (char*)assoc->value, server_time());
	}
	response(client,"");
}
void unknow(int client)
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
char* read_line(int sock)
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
int read_buf(int sock, char*buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
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
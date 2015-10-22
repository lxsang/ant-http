#include "plugin.h"

plugin_header __plugin__;
call __init__;
// private function

void __init_plugin__(const char* pl,const char*ph,const char* htdocs, const char* pdir){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= strdup(ph);
	__plugin__.htdocs = strdup(htdocs);
	__plugin__.pdir = strdup(pdir);
	if(__init__ != NULL) __init__();
}; 
sqldb getdb()
{
	int plen = strlen(__plugin__.name)+strlen(__plugin__.dbpath)+4;
	char* path = (char*) malloc(plen*sizeof(char));
	strcpy(path,__plugin__.dbpath);
	strcat(path,__plugin__.name);
	strcat(path,".db");
	sqldb ret = (sqldb)database(path);
	free(path);
	return ret;
}
void header_base(int client)
{

	response(client, "HTTP/1.0 200 OK");
	response(client, "RPI CAR SERVER ");

}
void redirect(int client,const char*path)
{
	__t(client,"<html><head><meta http-equiv=\"refresh\" content=\"0; url=%s\"></head><body></body></html>",path);
}

void html(int client)
{
	header(client,"text/html; charset=utf-8");
}
void text(int client)
{
	header(client,"text/plain; charset=utf-8");
}
void json(int client)
{
	header(client,"application/json");
}
void jpeg(int client)
{
	header(client,"image/jpeg");
}
void header(int client, const char* type)
{
	header_base(client);
	__t(client,"Content-Type: %s",type);
	response(client,"");
}

void response(int client, const char* data)
{
	char buf[BUFFLEN+3];
	strcpy(buf, data);
	int size = strlen(data);
	buf[size] = '\r';
	buf[size+1] = '\n';
	buf[size+2] = '\0';
	send(client, buf, strlen(buf), 0);
}
void __ti(int client,int data)
{
	char str[15];
	sprintf(str, "%d", data);
	response(client,str);
}

void __t(int client, const char* fstring,...)
{

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
			response(client,data);
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
				send(client, chunk, buflen, 0);
				free(chunk);	
			}
			chunk = "\r\n";
			send(client, chunk, strlen(chunk), 0);
		}
        free(data);
    }
	//
}
void __b(int client, const unsigned char* data, int size)
{
	char buf[BUFFLEN];
	int sent = 0;
	int buflen = 0;
	if(size <= BUFFLEN)
		send(client,data,size,0);
	else
	{
		while(sent < size)
		{
			if(size - sent > BUFFLEN)
				buflen = BUFFLEN;
			else
				buflen = size - sent;
			memcpy(buf,data+sent,buflen);
			send(client,buf,buflen,0);
			sent += buflen;
		}	
	}
}
void __fb(int client, const char* file)
{
	printf("Open file %s\n",file );
	unsigned char buffer[BUFFLEN];
	FILE *ptr;
	ptr = fopen(file,"rb");
	if(!ptr)
	{
		LOG("Cannot read : %s\n", file);
		return;
	}
	while(!feof(ptr))
	{
		fread(buffer,BUFFLEN,1,ptr);
		__b(client,buffer,BUFFLEN);
	}
	fclose(ptr);
}
void __f(int client, const char* file)
{
	unsigned char buf[BUFFLEN];
	FILE *ptr;
	ptr = fopen(file,"r");
	if(!ptr)
	{
		LOG("Cannot read : %s\n", file);
		return;
	}
	fgets(buf, sizeof(buf), ptr);
	while(!feof(ptr))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), ptr);
	}
	fclose(ptr);
}

char* route(const char* repath)
{
	int len = strlen(__plugin__.name) + 2;
	if(repath != NULL)
		len += strlen(repath)+1;
	char * path = (char*) malloc(len*sizeof(char));
	strcpy(path,"/");
	strcat(path,__plugin__.name);
	if(repath != NULL)
	{
		strcat(path,"/");
		strcat(path,repath);
	}
	return path;
}
char* htdocs(const char* repath)
{
	if(repath != NULL)
		return __s("%s/%s",__plugin__.htdocs,repath);
	else
		return __s("%s",__plugin__.htdocs);
}
int upload(const char* tmp, const char* path)
{
	return !rename(tmp, path);
}
void set_cookie(int client,dictionary dic)
{
	header_base(client);
	__t(client,"Content-Type: text/html; charset=utf-8");
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s",assoc->key, assoc->value.s);
	}
	response(client,"");
}
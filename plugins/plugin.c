#include "plugin.h"

plugin_header __plugin__;
// private function
call __init__;

void __init_plugin__(const char* pl,const char*ph,const char* htdocs, const char* pdir,int port){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= strdup(ph);
	__plugin__.htdocs = strdup(htdocs);
	__plugin__.pdir = strdup(pdir);
	__plugin__.sport = port;
	if(__init__ != NULL) __init__();
}; 

#ifdef USE_DB
sqldb __getdb(char *name)
{
	int plen = strlen(name)+strlen(__plugin__.dbpath)+4;
	char* path = (char*) malloc(plen*sizeof(char));
	strcpy(path,__plugin__.dbpath);
	strcat(path,__plugin__.name);
	strcat(path,".db");
	sqldb ret = (sqldb)database(path);
	free(path);
	return ret;
}
sqldb getdb()
{
	return getdb(__plugin__.name);
}
#endif
void set_status(int client,int code,const char* msg)
{
	response(client, __s("HTTP/1.0 %d %s", code, msg));
	response(client, __s("Server: %s ", SERVER_NAME));
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
void textstream(int client)
{
	header(client, "text/event-stream");
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
	header(client,"image/jpeg");
}
void header(int client, const char* type)
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
void set_cookie(int client,const char* type, dictionary dic)
{
	set_status(client,200,"OK");
	__t(client,"Content-Type: %s",type);
	association assoc;
	for_each_assoc(assoc,dic){
		__t(client,"Set-Cookie: %s=%s; Path=/%s",assoc->key, (char*)assoc->value, __plugin__.name);
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
char* config_dir()
{
	struct stat st;
	char* path = __s("%s%s%s", __plugin__.pdir,DIR_SEP, __plugin__.name);
	if (stat(path, &st) == -1)
		mkdir(path, 0755);
	return path;
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
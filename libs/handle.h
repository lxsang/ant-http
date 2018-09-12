#ifndef HANDLE_H
#define HANDLE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//open ssl
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "dictionary.h"
#include "list.h"
#include "ini.h"

#define SERVER_NAME "antd"
#define IS_POST(method) (strcmp(method,"POST")== 0)
#define IS_GET(method) (strcmp(method,"GET")== 0)
#define R_STR(d,k) ((char*)dvalue(d,k))
#define R_INT(d,k) (atoi(dvalue(d,k)))
#define R_FLOAT(d,k) ((double)atof(dvalue(d,k)))
#define R_PTR(d,k) (dvalue(d,k))
#define __RESULT__ "{\"result\":%d,\"msg\":\"%s\"}"


#ifdef USE_OPENSSL
int __attribute__((weak)) usessl();
#endif

typedef struct  { 
	int port;
    char *plugins_dir; 
    char *plugins_ext;
    char *db_path;
    char* htdocs;
    char* tmpdir;
    list rules;
    dictionary handlers;
    int backlog;
    int maxcon;
    int connection;
#ifdef USE_OPENSSL
    int usessl;
    char* sslcert;
    char* sslkey;
#endif
}config_t;
//extern config_t server_config;
typedef struct{
    int sock;
    void* ssl;
    char* ip;
} antd_client_t;

int response(void*, const char*);
void ctype(void*,const char*);
void redirect(void*,const char*);
void html(void*);
void text(void*);
void json(void*);
void jpeg(void*);
void octstream(void*, char*);
void textstream(void*);
int __ti(void*,int);
int __t(void*, const char*,...);
int __b(void*, const unsigned char*, int);
int __f(void*, const char*);
int __fb(void*, const char*);
int upload(const char*, const char*);

void set_cookie(void*, const char*,dictionary,const char*);
void set_status(void*,int,const char*);
void clear_cookie(void*, dictionary);
/*Default function for plugin*/
void unknow(void*);
int ws_enable(dictionary);
char* read_line(void* sock);
int read_buf(void* sock,char* buf,int i);
int antd_send( void *source, const void* data, int len);
int antd_recv( void *source,  void* data, int len);
int antd_close(void* source);
#endif

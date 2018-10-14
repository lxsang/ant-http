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
#include <fcntl.h>
#include "dictionary.h"
#include "list.h"
#include "ini.h"

#define SERVER_NAME "Antd"
#define IS_POST(method) (strcmp(method,"POST")== 0)
#define IS_GET(method) (strcmp(method,"GET")== 0)
#define R_STR(d,k) ((char*)dvalue(d,k))
#define R_INT(d,k) (atoi(dvalue(d,k)))
#define R_FLOAT(d,k) ((double)atof(dvalue(d,k)))
#define R_PTR(d,k) (dvalue(d,k))
#define __RESULT__ "{\"result\":%d,\"msg\":\"%s\"}"
#define FORM_URL_ENCODE  "application/x-www-form-urlencoded"
#define FORM_MULTI_PART  "multipart/form-data"
#define MAX_ATTEMPT 1000
#ifdef USE_OPENSSL
int __attribute__((weak)) usessl();
#endif
//extern config_t server_config;
typedef struct{
    int sock;
    void* ssl;
    char* ip;
#ifdef USE_OPENSSL
    int status;
#endif
    int attempt;
} antd_client_t;

typedef struct {
    antd_client_t* client;
    dictionary request;
} antd_request_t;


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
    int n_workers;
#ifdef USE_OPENSSL
    int usessl;
    char* sslcert;
    char* sslkey;
#endif
}config_t;

typedef struct  { 
    char *name; 
    char *dbpath;
    char * htdocs;
    char*pdir;
	int sport;
    int raw_body;
#ifdef USE_OPENSSL
    int usessl;
#endif
} plugin_header_t;

void set_nonblock(int socket);
//void set_block(int socket);
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
void badrequest(void* client);
void unimplemented(void* client);
void notfound(void* client);
int ws_enable(dictionary);
char* read_line(void* sock);
int read_buf(void* sock,char* buf,int i);
int antd_send( void *source, const void* data, int len);
int antd_recv( void *source,  void* data, int len);
int antd_close(void* source);
#endif

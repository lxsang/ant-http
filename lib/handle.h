#ifndef HANDLE_H
#define HANDLE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
//open ssl
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#ifdef USE_DB
#include "dbhelper.h"
#endif
#include <fcntl.h>
#include <stdlib.h>
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
#define MAX_WAIT_S 2 // 1/3 minute

//extern config_t server_config;

typedef struct {
    unsigned int port;
    int usessl;
    char* htdocs;
    int sock;
    list_t rules;
} port_config_t;

typedef struct{
    int sock;
    void* ssl;
    char* ip;
    int port;
//#ifdef USE_OPENSSL
    int status;
//#endif
    time_t last_io;
    port_config_t* port_config;
} antd_client_t;

typedef struct {
    antd_client_t* client;
    dictionary_t request;
} antd_request_t;

typedef struct
{
    dictionary_t header;
    list_t cookie;
    int status;

} antd_response_header_t;



typedef struct  { 
	//int port;
    char *plugins_dir; 
    char *plugins_ext;
    char *db_path;
    //char* htdocs;
    char* tmpdir;
    dictionary_t handlers;
    int backlog;
    int maxcon;
    int connection;
    int n_workers;
    FILE* errorfp;
// #ifdef DEBUG
    FILE* logfp;
// #endif
// #ifdef USE_OPENSSL
    int enable_ssl;
    char* sslcert;
    char* sslkey;
    char* ssl_cipher;
    dictionary_t mimes;
    dictionary_t ports;
// #endif
}config_t;

typedef struct  { 
    char *name; 
    char *dbpath;
    char *tmpdir;
    char*pdir;
    int raw_body;
} plugin_header_t;

void set_nonblock(int socket);
//void set_block(int socket);

void antd_send_header(void*,antd_response_header_t*);
const char* get_status_str(int stat);
int __t(void*, const char*,...);
int __b(void*, const unsigned char*, int);
int __f(void*, const char*);

int upload(const char*, const char*);

/*Default function for plugin*/
void antd_error(void* client, int status, const char* msg);

int ws_enable(dictionary_t);
char* read_line(void* sock);
int read_buf(void* sock,char* buf,int i);
int antd_send( void *source, const void* data, int len);
int antd_recv( void *source,  void* data, int len);
int antd_close(void* source);
void destroy_request(void *data);
#endif

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
#ifdef USE_ZLIB
#include <zlib.h>
#endif
#ifdef USE_DB
#include "dbhelper.h"
#endif

#include <fcntl.h>
#include <stdlib.h>
#include "dictionary.h"
#include "list.h"
#include "ini.h"
#include "scheduler.h"

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
#define MAX_IO_WAIT_TIME 5 // second


typedef enum {ANTD_CGZ, ANTD_CDEFL, ANTD_CNONE} antd_compress_t;

// define the client flag
#define CLIENT_FL_ACCEPTED          0x01
#define CLIENT_FL_COMPRESSION_END   0x02
#define CLIENT_FL_HTTP_1_1          0x04
#define CLIENT_FL_READABLE          0x08
#define CLIENT_FL_WRITABLE          0x10

typedef struct {
    unsigned int port;
    int usessl;
    char* htdocs;
    int sock;
    dictionary_t rules;
} port_config_t;

typedef struct{
    int sock;
    void* ssl;
    uint8_t flags;
    time_t last_io;
    // compress option
    antd_compress_t z_level;
    void* zstream;
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
    int max_upload_size;
    // ssl
    int enable_ssl;
    char* sslcert;
    char* sslkey;
    char* ssl_cipher;
    int gzip_enable;
    list_t gzip_types;
    dictionary_t mimes;
    dictionary_t ports;
// #endif
}config_t;

typedef struct  { 
    char name[128]; 
    char dbpath[512];
    char tmpdir[512];
    char pdir[512];
    int raw_body;
} plugin_header_t;

// some functions that allows access to server
// private data
int __attribute__((weak))  require_plugin(const char*);
void __attribute__((weak)) htdocs(antd_request_t* rq, char* dest);
void __attribute__((weak)) dbdir(char* dest);
void __attribute__((weak)) tmpdir(char* dest);
void __attribute__((weak)) plugindir(char* dest);
int __attribute__((weak))  compressable(char* ctype);
void __attribute__((weak)) schedule_task(antd_task_t* task);

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
int read_buf(void* sock,char* buf,int i);
int antd_send( void *source, const void* data, int len);
int antd_recv( void *source,  void* data, int len);
int antd_close(void* source);
void destroy_request(void *data);
#endif

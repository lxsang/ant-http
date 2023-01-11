#ifndef HANDLE_H
#define HANDLE_H

#include <time.h>

#include "list.h"
#include "dictionary.h"

#define SERVER_NAME "Antd"
#define IS_POST(method) (strcmp(method, "POST") == 0)
#define IS_GET(method) (strcmp(method, "GET") == 0)
#define R_STR(d, k) ((char *)dvalue(d, k))
#define R_INT(d, k) (atoi(dvalue(d, k)))
#define R_FLOAT(d, k) ((double)atof(dvalue(d, k)))
#define R_PTR(d, k) (dvalue(d, k))
#define __RESULT__ "{\"result\":%d,\"msg\":\"%s\"}"
#define FORM_URL_ENCODE "application/x-www-form-urlencoded"
#define FORM_MULTI_PART "multipart/form-data"
#define MAX_IO_WAIT_TIME 5 // second

#define ANTD_CLIENT_ACCEPT 0x0
#define ANTD_CLIENT_HANDSHAKE 0x1
#define ANTD_CLIENT_HEADER_DECODE 0x2
#define ANTD_CLIENT_PLUGIN_EXEC 0x3
#define ANTD_CLIENT_PROTO_CHECK 0x4
#define ANTD_CLIENT_RESOLVE_REQUEST 0x5
#define ANTD_CLIENT_SERVE_FILE 0x6
#define ANTD_CLIENT_RQ_DATA_DECODE 0x7
#define ANTD_CLIENT_PROXY_MONITOR 0x8

#define ANTD_PLUGIN_READY 0x0
#define ANTD_PLUGIN_PANNIC 0x1

typedef enum
{
    ANTD_CGZ,
    ANTD_CDEFL,
    ANTD_CNONE
} antd_compress_t;

//extern config_t server_config;

typedef struct
{
    unsigned int port;
    int usessl;
    char *htdocs;
    char* plugins;
    int sock;
    dictionary_t rules;
} port_config_t;

typedef struct
{
    int sock;
    void *ssl;
    int state;
    time_t last_io;
    // compress
    antd_compress_t z_level;
    void *zstream;
    int z_status;
} antd_client_t;

typedef struct
{
    antd_client_t *client;
    dictionary_t request;
} antd_request_t;

typedef struct
{
    dictionary_t header;
    list_t cookie;
    int status;

} antd_response_header_t;

typedef struct
{
    //int port;
    char *plugins_dir;
    char *plugins_ext;
    char *db_path;
    //char* htdocs;
    char *tmpdir;
    char *stat_fifo_path;
    dictionary_t handlers;
    int backlog;
    int maxcon;
    int connection;
    int n_workers;
    int scheduler_timeout;
    int max_upload_size;
    // ssl
    int enable_ssl;
    char *sslcert;
    char *sslkey;
    char *ssl_cipher;
    int gzip_enable;
    int debug_enable;
    list_t gzip_types;
    dictionary_t mimes;
    dictionary_t ports;
    dictionary_t plugins;
    // #endif
} config_t;

typedef struct
{
    char name[128];
    char* dbpath;
    char* tmpdir;
    char* pdir;
    dictionary_t config;
    int raw_body;
    int status;
} plugin_header_t;

int __attribute__((weak)) require_plugin(const char *);
void __attribute__((weak)) htdocs(antd_request_t *rq, char *dest);
void __attribute__((weak)) dbdir(char **dest);
void __attribute__((weak)) tmpdir(char **dest);
void __attribute__((weak)) plugindir(char **dest);

int __attribute__((weak)) compressable(char *ctype);

void set_nonblock(int socket);
//void set_block(int socket);

void antd_send_header(void *, antd_response_header_t *);
const char *get_status_str(int stat);
int __t(void *, const char *, ...);
int __b(void *, const unsigned char *, int);
int __f(void *, const char *);

int upload(const char *, const char *);

/*Default function for plugin*/
void antd_error(void *client, int status, const char *msg);

int ws_enable(dictionary_t);
int read_buf(void *sock, char *buf, int i);
int antd_send(void *source, const void *data, int len);
int antd_recv(void *source, void *data, int len);
int antd_recv_upto(void* src, void* data, int len);
int antd_close(void *source);
void destroy_request(void *data);
#endif

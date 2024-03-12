#ifndef CONFIG_H
#define CONFIG_H

#include "lib/handle.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE "antd-config.ini"
#endif

typedef struct
{
    unsigned int port;
    int usessl;
    char *htdocs;
    char* plugins;
    int sock;
    antd_proto_t type;
    dictionary_t rules;
} port_config_t;

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
void load_config(const char *file);
void destroy_config();
#endif
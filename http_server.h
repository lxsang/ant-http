#ifndef HTTP_SERVER
#define HTTP_SERVER
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include "libs/handle.h"
#include "plugin_manager.h"

#define FORM_URL_ENCODE  "application/x-www-form-urlencoded"
#define FORM_MULTI_PART  "multipart/form-data"
#define APP_JSON		 "application/json"
#define PLUGIN_HANDLER	 "handler"
#define WS_MAGIC_STRING	 "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: ant-httpd"

#define CONFIG "config.ini"
extern config_t server_config;

void accept_request(void*);
void cat(void*, FILE *);
void cannot_execute(void*);
void error_die(const char *);
//int get_line(int, char *, int);
void not_found(void*);
void serve_file(void*, const char *);
int startup(unsigned *);
void unimplemented(void*);
void badrequest(void*);
void rule_check(association, const char* , const char* , const char* , char*);
void ws_confirm_request(void*, const char*);
char* post_url_decode(void* client,int len);
dictionary decode_url_request(const char* query);
dictionary decode_request(void* client,const char* method, char* url);
dictionary decode_multi_part_request(void*,const char*);
dictionary decode_cookie(const char*);
char* json_data_decode(void*,int);

int execute_plugin(void* client, const char *path,
  const char *method, dictionary rq);

#endif
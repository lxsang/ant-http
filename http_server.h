#ifndef HTTP_SERVER
#define HTTP_SERVER
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include "plugins/handle.h"
#include "plugin_manager.h"

#define FORM_URL_ENCODE  "application/x-www-form-urlencoded"
#define FORM_MULTI_PART  "multipart/form-data"
#define APP_JSON		 "application/json"
#define PLUGIN_HANDLER	 "handler"
#define WS_MAGIC_STRING	 "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: ant-httpd"

void accept_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
int get_line(int, char *, int);
void not_found(int);
void serve_file(int, const char *);
int startup(unsigned *);
void unimplemented(int);
void badrequest(int);
void ws_confirm_request(int, const char*);
char* post_url_decode(int client,int len);
dictionary decode_url_request(const char* query);
dictionary decode_request(int client,const char* method,const char* query);
dictionary decode_multi_part_request(int,const char*);
dictionary decode_cookie(const char*);
char* json_data_decode(int,int);

int execute_plugin(int client, const char *path,
  const char *method, const char *query_string);

#endif
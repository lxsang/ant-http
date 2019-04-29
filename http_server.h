#ifndef HTTP_SERVER
#define HTTP_SERVER
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "libs/handle.h"
#include "libs/scheduler.h"
#include "plugin_manager.h"

#define PLUGIN_HANDLER	 "handle"
#define WS_MAGIC_STRING	 "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define CONFIG "config.ini"

// define all task status here
// for debug purpose
#define TASK_ACCEPT             0x01
#define TASK_ACCEPT_PEND        0xA0
#define TASK_ACCEPT_SSL_CONT    0xA1
#define TASK_ACCEPT_HS_DONE     0xA2
#define TASK_ACCEPT_READWAIT    0xA3
#define TASK_DECODE_HEADER      0x02
#define TASK_DECODE_RQ          0x03
#define TASK_RESOLVE_RQ         0x04
#define TASK_EXEC_PLUGIN_RAW    0x05 // with raw data
#define TASK_EXEC_PLUGIN_COOK   0x06 // with decoded post request data
#define TASK_SERVE_FILE         0x07
#define TASK_DECODE_MP_DATA     0x08

config_t* config(); 
void destroy_config();
void load_config(const char* file);
void* accept_request(void*);
void* finish_request(void*);
void cat(void*, FILE *);
void cannot_execute(void*);
//int get_line(int, char *, int);
void not_found(void*);
void* serve_file(void*);
int startup(unsigned *);
void unimplemented(void*);
void badrequest(void*);
int rule_check(const char*, const char*, const char* , const char* , const char* , char*);
void ws_confirm_request(void*, const char*);
char* post_url_decode(void* client,int len);
void decode_url_request(const char* query, dictionary);
void* decode_request_header(void* data);
void* decode_request(void* data);
void* decode_post_request(void* data);
void* resolve_request(void* data);
void* decode_multi_part_request(void*,const char*);
void* decode_multi_part_request_data(void* data);
dictionary decode_cookie(const char*);
char* post_data_decode(void*,int);
void set_nonblock(int);
void* execute_plugin(void* data, const char *path);

#endif
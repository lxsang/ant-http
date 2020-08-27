#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <stdio.h>
#include <string.h>
#include "lib/handle.h"

#define PLUGIN_HANDLER "handle"
#define WS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#ifndef CONFIG_FILE
#define CONFIG_FILE "antd-config.ini"
#endif

config_t *config();
void destroy_config();
void load_config(const char *file);
void *accept_request(void *);
void *finish_request(void *);
void cat(void *, FILE *);
void cannot_execute(void *);
//int get_line(int, char *, int);
void *serve_file(void *);
int startup(unsigned *);
int rule_check(const char *, const char *, const char *, const char *, const char *, char *);
void ws_confirm_request(void *, const char *);
char *post_url_decode(void *client, int len);
void decode_url_request(const char *query, dictionary_t);
void *decode_request_header(void *data);
void *decode_request(void *data);
void *decode_post_request(void *data);
void *resolve_request(void *data);
void *decode_multi_part_request(void *, const char *);
void *decode_multi_part_request_data(void *data);
void decode_cookie(const char *, dictionary_t d);
char *post_data_decode(void *, int);
void set_nonblock(int);
void *execute_plugin(void *data, const char *path);

#endif
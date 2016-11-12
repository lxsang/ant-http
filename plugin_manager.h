#include <dlfcn.h>
#include <sys/socket.h>
#include "plugins/dictionary.h"
#include "config.h"

#define FORM_URL_ENCODE  "application/x-www-form-urlencoded"
#define FORM_MULTI_PART  "multipart/form-data"
#define APP_JSON		 "application/json"
#define PLUGIN_HANDLER	 "handler"
#define WS_MAGIC_STRING	 "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
struct plugin_entry { 
    struct plugin_entry *next; 
    char *pname; 
    void *handle;
};

/* lookup: look for s in hashtab */
struct plugin_entry *plugin_lookup(char *s);
/* install: put (name, defn) in hashtab */
struct plugin_entry *plugin_load(char *name);
void unload_all_plugin();
void unload_plugin(struct plugin_entry*);
void unload_plugin_by_name(const char*);
void * plugin_from_file(char* name);
void ws_confirm_request(int, const char*);
char* post_url_decode(int client,int len);
dictionary decode_url_request(const char* query);
dictionary decode_request(int client,const char* method,const char* query);
dictionary decode_multi_part_request(int,const char*);
dictionary decode_cookie(const char*);
char* json_data_decode(int,int);
char* read_line(int sock);
int read_buf(int sock,char* buf,int i);
int execute_plugin(int client, const char *path,
  const char *method, const char *query_string);
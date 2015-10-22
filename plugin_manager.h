#include <dlfcn.h>
#include <sys/socket.h>
#include "plugins/dictionary.h"
#include "config.h"

#define FORM_URL_ENCODE  "application/x-www-form-urlencoded"
#define FORM_MULTI_PART  "multipart/form-data"


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
void * plugin_from_file(char* name);
char* post_url_decode(int client,int len);
dictionary decode_url_request(const char* query);
dictionary decode_request(int client,const char* method,const char* query);
dictionary decode_multi_part_request(int,const char*);
dictionary decode_cookie(const char*);
char* read_line(int sock);
int read_buf(int sock,char* buf,int i);
int execute_plugin(int client, const char *path,
  const char *method, const char *query_string);
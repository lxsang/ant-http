#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H
#include <dlfcn.h>
#include "libs/utils.h"
#include "libs/handle.h"

struct plugin_entry { 
    struct plugin_entry *next; 
    char *pname; 
    void *handle;
};
extern config_t server_config;
/* lookup: look for s in hashtab */
struct plugin_entry *plugin_lookup(char *s);
/* install: put (name, defn) in hashtab */
struct plugin_entry *plugin_load(char *name);
void unload_all_plugin();
void unload_plugin(struct plugin_entry*);
void unload_plugin_by_name(const char*);
void * plugin_from_file(char* name);

#endif
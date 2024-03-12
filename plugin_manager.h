#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "lib/dictionary.h"

#define PLUGIN_HANDLER "handle"

struct plugin_entry { 
    struct plugin_entry *next; 
    char *name; 
    void *handle;
    dictionary_t instances;
};
/* lookup: look for s in hashtable */
struct plugin_entry *plugin_lookup(char *s);
/* install: put (name, defn) in hashtable */
struct plugin_entry *plugin_load(char *name, dictionary_t config);
void unload_all_plugin();
void unload_plugin(struct plugin_entry*);
#endif
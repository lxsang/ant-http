#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "lib/dictionary.h"

struct plugin_entry { 
    struct plugin_entry *next; 
    char *pname; 
    void *handle;
};
/* lookup: look for s in hashtab */
struct plugin_entry *plugin_lookup(char *s);
/* install: put (name, defn) in hashtab */
struct plugin_entry *plugin_load(char *name, dictionary_t config);
void unload_all_plugin();
void unload_plugin(struct plugin_entry*);
#endif
#ifndef PLUGIN_CTX_H
#define PLUGIN_CTX_H
#include "handle.h"
struct _plugin_ctx_t
{
    char * name;
    const char * tmpdir;
    const char * basedir;
    char * confdir;
    int raw_body;
    int status;
    dictionary_t config;
    void * data;
    void *(*handle)(void *);
    void *(*create)(struct _plugin_ctx_t *);
    void (*drop)(struct _plugin_ctx_t *);
} ;
#endif
#ifndef PLUGIN_H
#define PLUGIN_H

#include <sys/stat.h>

#define ANTD_PLUGIN_READY 0x0
#define ANTD_PLUGIN_PANNIC 0x1
#define ANTD_PLUGIN_INIT 0x2

#define PLUGIN_INIT "create"
#define PLUGIN_HANDLE "handle"
#define PLUGIN_DROP "drop"

#define DEF_PLUGIN_INTERFACE(name, param, ret) ret##name(param)

#include "utils.h"
#include "dictionary.h"

#define PLUGIN_PANIC(ctx, a, ...)                          \
    ERROR("%s: " a, antd_plugin_name(ctx), ##__VA_ARGS__); \
    antd_plugin_set_status(ctx, ANTD_PLUGIN_PANNIC);

typedef struct _plugin_ctx_t antd_plugin_ctx_t;

const char *antd_plugin_basedir(antd_plugin_ctx_t *);
const char *antd_plugin_tmpdir(antd_plugin_ctx_t *);
const char *antd_plugin_confdir(antd_plugin_ctx_t *);
const char *antd_plugin_name(antd_plugin_ctx_t *);
void antd_plugin_set_status(antd_plugin_ctx_t *, int);
int antd_plugin_status(antd_plugin_ctx_t *);
void antd_plugin_use_raw_body(antd_plugin_ctx_t *);
int antd_plugin_is_raw_body(antd_plugin_ctx_t *);
void *antd_plugin_data(antd_plugin_ctx_t *);
dictionary_t antd_plugin_config(antd_plugin_ctx_t*);

/*Default interfaces shall be implemented by plugin*/
void *create(antd_plugin_ctx_t *);
void drop(antd_plugin_ctx_t *);
void *handle(void *);

#endif

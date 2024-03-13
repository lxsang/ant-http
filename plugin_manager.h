#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "lib/dictionary.h"
#include "lib/plugin.h"
#include "lib/scheduler.h"

typedef void*(*antd_plugin_handle_t)(void *);

antd_plugin_ctx_t *antd_plugin_load(const char *name);
void antd_unload_all_plugin();
antd_plugin_handle_t antd_get_ctx_handle(antd_plugin_ctx_t *);
#endif
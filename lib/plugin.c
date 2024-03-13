#include "plugin_ctx.h"
#include "plugin.h"


const char * antd_plugin_basedir(antd_plugin_ctx_t * ctx)
{
    return ctx->basedir;
}
const char * antd_plugin_tmpdir(antd_plugin_ctx_t * ctx)
{
    return ctx->tmpdir;
}
const char * antd_plugin_confdir(antd_plugin_ctx_t *ctx)
{
    if(ctx->confdir == NULL)
    {
        struct stat st;
        ctx->confdir = __s("%s%s%s", ctx->basedir,DIR_SEP, ctx->name);
        if (stat(ctx->confdir, &st) == -1)
            mkdir(ctx->confdir, 0755);
    }
    return ctx->confdir;
}
const char * antd_plugin_name(antd_plugin_ctx_t *ctx)
{
    return ctx->name;
}
void antd_plugin_set_status(antd_plugin_ctx_t * ctx, int stat)
{
    ctx->status = stat;
}
int antd_plugin_status(antd_plugin_ctx_t * ctx)
{
    return ctx->status;
}
void antd_plugin_use_raw_body(antd_plugin_ctx_t * ctx)
{
    ctx->raw_body = 1;
}
int antd_plugin_is_raw_body(antd_plugin_ctx_t *ctx)
{
    return ctx->raw_body == 1;
}
void* antd_plugin_data(antd_plugin_ctx_t* ctx)
{
    return ctx->data;
}
dictionary_t antd_plugin_config(antd_plugin_ctx_t* ctx)
{
    return ctx->config;
}

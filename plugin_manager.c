#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <stdio.h>
#include "lib/utils.h"
#include "lib/handle.h"
#include "config.h"
#include "lib/plugin_ctx.h"
#include "plugin_manager.h"

struct plugin_entry { 
    struct plugin_entry *next; 
    char *name; 
    void *handle;
    dictionary_t instances;
};

extern config_t g_server_config;

/**
 * Plugin table to store the loaded plugin
 */
static struct plugin_entry *plugin_table[HASHSIZE];

/**
 * Locate a plugin in the plugin table
 * @param  s plugin name
 * @return   a plugin entry in the plugin table
 */
static struct plugin_entry *plugin_lookup(const char *s)
{
    if(!s)
    {
        return NULL;
    }
    struct plugin_entry *np;
    for (np = plugin_table[hash(s, HASHSIZE)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np; /* found */
    return NULL;       /* not found */
}

static void antd_plugin_ctx_drop(struct plugin_entry* np, antd_plugin_ctx_t* ctx)
{
    if(!ctx)
    {
        return;
    }
    if(ctx->drop)
    {
        if(ctx->name)
            LOG("Release user resource for context: %s", ctx->name);
        ctx->drop((void*)ctx);
    }
    if(ctx->name)
    {
        LOG("Dropping plugin context: %s", ctx->name);
        if(np->instances)
        {
            dput(np->instances, ctx->name, NULL);
        }
        free(ctx->name);
    }
    if(ctx->confdir)
    {
        free(ctx->confdir);
    }
    free(ctx);
}


static antd_plugin_ctx_t* antd_plugin_ctx_lookup(struct plugin_entry* np, const char* name)
{
    if(!np || !np->instances)
    {
        return NULL;
    }
    LOG("Looking for plugin context: %s", name);
    antd_plugin_ctx_t* ctx = dvalue(np->instances, name);
    if(ctx == NULL)
    {
        char *error;
        LOG("Create new plugin context: %s", name);
        ctx = (antd_plugin_ctx_t *)malloc(sizeof(antd_plugin_ctx_t));
        if(!ctx)
        {
            ERROR("Unable to allocate memory for plugin context `%s`: %s", name, strerror(errno));
            return NULL;
        }
        // init the context
        ctx->basedir = g_server_config.plugins_dir;
        ctx->tmpdir = g_server_config.tmpdir;
        ctx->name = strdup(name);
        ctx->confdir = NULL;
        ctx->raw_body = 0;
        ctx->status = ANTD_PLUGIN_INIT;
        ctx->config=dvalue(g_server_config.plugins, name);
        ctx->data = NULL;
        ctx->handle = NULL;
        ctx->create = NULL;
        ctx->drop = NULL;
        // look for handle function
        ctx->handle = (void* (*)(void *))dlsym(np->handle, PLUGIN_HANDLE);
        if ((error = dlerror()) != NULL)
        {
            ERROR("Problem when finding plugin handle function for %s : %s", name, error);
            ctx->handle = NULL;
            antd_plugin_ctx_drop(np, ctx);
            return NULL;
        }
        // look for drop function
        ctx->drop = (void (*)(antd_plugin_ctx_t *))dlsym(np->handle, PLUGIN_DROP);
        if ((error = dlerror()) != NULL)
        {
            ERROR("Problem when finding plugin drop function for %s : %s", name, error);
            ctx->drop = NULL;
            antd_plugin_ctx_drop(np, ctx);
            return NULL;
        }
        // look for init function
        ctx->create = (void* (*)(antd_plugin_ctx_t *))dlsym(np->handle, PLUGIN_INIT);
        if ((error = dlerror()) != NULL)
        {
            ERROR("Problem when finding plugin init function for %s : %s.", name, error);
            ctx->create = NULL;
            antd_plugin_ctx_drop(np, ctx);
            return NULL;
        }
        else
        {
            // run the init function
            ctx->data = ctx->create(ctx);
            if(ctx->status == ANTD_PLUGIN_PANNIC)
            {
                ERROR("PANIC happens when init plugin context %s. drop it", name);
                antd_plugin_ctx_drop(np, ctx);
                return NULL;
            }
            ctx->status = ANTD_PLUGIN_READY;
        }
        dput(np->instances, name, (void*)ctx);
    }
    return ctx;
}

static void antd_plugin_entry_drop(struct plugin_entry* np)
{
    if(!np)
    {
        return;
    }
    if(np->name)
    {
        LOG("Unloaded %s", np->name);
        free(np->name);
    }
    if(np->instances)
    {
        chain_t it;
        for_each_assoc(it, np->instances)
        {
            antd_plugin_ctx_drop(np,(antd_plugin_ctx_t*)it->value);
        }
        freedict(np->instances);
    }
    if(np->handle)
    {
        dlclose(np->handle);
    }
    free(np);
}

/**
 * Load a plugin to the plugin table
 * Only load when not available in the plugin table
 * @param  name plugin name
 * @param config: plugin configuration
 * @return      pointer to the loaded plugin
 */
antd_plugin_ctx_t* antd_plugin_load(const char *name)
{
    const char *plugin_file_name = NULL;
    char path[BUFFLEN];
    struct plugin_entry *np;
    unsigned hashval;
    antd_plugin_ctx_t *ctx;
    dictionary_t pconf = dvalue(g_server_config.plugins, name);
    if (pconf)
    {
        plugin_file_name = dvalue(pconf, "name");
    }
    if(plugin_file_name == NULL)
    {
        plugin_file_name = name;
    }
    if(plugin_file_name == NULL)
    {
        return NULL;
    }
    if ((np = plugin_lookup(plugin_file_name)) == NULL)
    { /* not found */
        LOG("Loading plugin: %s...", plugin_file_name);
        np = (struct plugin_entry *)malloc(sizeof(struct plugin_entry));
        np->instances = NULL;
        if (np == NULL)
        {
            return NULL;
        }

        (void)snprintf(path, sizeof(path), "%s/%s%s", g_server_config.plugins_dir, plugin_file_name, g_server_config.plugins_ext);
        np->name = strdup(plugin_file_name);
        // now load it from file
        np->handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL /*| RTLD_NODELETE*/);
        if (!np->handle)
        {
            ERROR("Cannot load plugin '%s' : '%s'", plugin_file_name, dlerror());
            antd_plugin_entry_drop(np);
            return NULL;
        }
        np->instances = dict();
        hashval = hash(name, HASHSIZE);
        np->next = plugin_table[hashval];
        plugin_table[hashval] = np;
    }
    else /* already there */
    {
        LOG("The plugin %s id already loaded", plugin_file_name);
    }
    // now look for the plugin context
    ctx = antd_plugin_ctx_lookup(np, name);
    // check if plugin is ready
    if (ctx == NULL)
    {
        ERROR("Unable to fetch plugin context for: [%s] %s", plugin_file_name, name);
        return NULL;
    }
    LOG("PLugin instance status: [%s] %d", name, ctx->status);
    if (ctx->status != ANTD_PLUGIN_READY)
    {
        ERROR("Plugin instance is not ready or error: [%s].", name);
        antd_plugin_ctx_drop(np, ctx);
        return NULL;
    }
    return ctx;
}

/*
    Unload a plugin by its name

void unload_plugin_by_name(const char *name)
{
    struct plugin_entry *np;
    int hasval = hash(name, HASHSIZE);
    np = plugin_table[hasval];
    if (strcmp(np->name, name) == 0)
    {
        antd_plugin_entry_drop(np);
        plugin_table[hasval] = np->next;
    }
    else
    {
        for (np = plugin_table[hasval]; np != NULL; np = np->next)
        {
            if (np->next != NULL && strcmp(name, np->next->name) == 0)
            {
                break;
            }
        }
        if (np == NULL)
            return; // the plugin is is not loaded
        antd_plugin_entry_drop(np->next);
        np->next = np->next->next;
    }
}
*/

/**
 * Unload all the plugin loaded on the plugin table
 */
void antd_unload_all_plugin()
{
    LOG("Unload all plugins");
    for (int i = 0; i < HASHSIZE; i++)
    {
        struct plugin_entry **np, *curr;
        np = &plugin_table[i];

        while ((curr = *np) != NULL)
        {
            (*np) = (*np)->next;
            antd_plugin_entry_drop(curr);
            //free(curr);
        }
        plugin_table[i] = NULL;
    }
}

antd_plugin_handle_t antd_get_ctx_handle(antd_plugin_ctx_t *ctx)
{
    return ctx->handle;
}

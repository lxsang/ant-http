#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <stdio.h>
#include "plugin_manager.h"
#include "lib/utils.h"
#include "lib/handle.h"
#include "config.h"

extern config_t g_server_config;

static void unload_plugin_by_name(const char *);
static void *plugin_from_file(char *name, char *path, dictionary_t conf);

/**
 * Plugin table to store the loaded plugin
 */
static struct plugin_entry *plugin_table[HASHSIZE];

/**
 * Locate a plugin in the plugin table
 * @param  s plugin name
 * @return   a plugin entry in the plugin table
 */
struct plugin_entry *plugin_lookup(char *s)
{
    struct plugin_entry *np;
    for (np = plugin_table[hash(s, HASHSIZE)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np; /* found */
    return NULL;       /* not found */
}

/**
 * Load a plugin to the plugin table
 * Only load when not available in the plugin table
 * @param  name plugin name
 * @param config: plugin configuration
 * @return      pointer to the loaded plugin
 */
struct plugin_entry *plugin_load(char *name, dictionary_t pconf)
{
    char *pname = NULL;
    char path[BUFFLEN];
    struct plugin_entry *np;
    unsigned hashval;
    plugin_header_t *(*metafn)();
    plugin_header_t *meta = NULL;
    int fromfd, tofd;
    char *error;
    struct stat st;
    int is_tmp = 0;
    if (pconf)
    {
        pname = dvalue(pconf, "name");
    }
    if ((np = plugin_lookup(name)) == NULL)
    { /* not found */
        LOG("Loading plugin: %s...", name);
        np = (struct plugin_entry *)malloc(sizeof(*np));
        if (np == NULL || name == NULL)
        {
            if (np)
                free(np);
            return NULL;
        }

        (void)snprintf(path, sizeof(path), "%s/%s%s", g_server_config.plugins_dir, name, g_server_config.plugins_ext);
        if (pname && strcmp(name, pname) != 0)
        {
            // copy plugin file to tmpdir
            (void)snprintf(path, sizeof(path), "%s/%s%s", g_server_config.plugins_dir, pname, g_server_config.plugins_ext);
            LOG("Original plugin file: %s", path);
            if ((fromfd = open(path, O_RDONLY)) < 0)
            {
                ERROR("Unable to open file for reading %s: %s", path, strerror(errno));
                return NULL;
            }
            if (stat(path, &st) != 0)
            {
                close(fromfd);
                ERROR("Unable to get file stat %s: %s", path, strerror(errno));
                return NULL;
            }
            (void)snprintf(path, sizeof(path), "%s/%s%s", g_server_config.tmpdir, name, g_server_config.plugins_ext);
            LOG("TMP plugin file: %s", path);
            if ((tofd = open(path, O_WRONLY | O_CREAT, 0600)) < 0)
            {
                close(fromfd);
                ERROR("Unable open file for reading %s: %s", path, strerror(errno));
                return NULL;
            }
            if (sendfile(tofd, fromfd, NULL, st.st_size) != st.st_size)
            {
                close(fromfd);
                close(tofd);
                ERROR("Unable to copy file: %s", strerror(errno));
                return NULL;
            }
            is_tmp = 1;
        }

        np->name = strdup(name);
        np->handle = plugin_from_file(name, path, pconf);
        if (is_tmp)
        {
            //TODO change this
            (void)remove(path);
        }
        if (np->handle == NULL)
        {
            if (np->name)
                free(np->name);
            if (np)
                free(np);
            return NULL;
        }
        hashval = hash(name, HASHSIZE);
        np->next = plugin_table[hashval];
        plugin_table[hashval] = np;
    }
    else /* already there */
    {
        LOG("The plugin %s id already loaded", name);
    }

    // check if plugin is ready
    metafn = (plugin_header_t * (*)()) dlsym(np->handle, "meta");
    if ((error = dlerror()) != NULL)
    {
        ERROR("Unable to fetch plugin meta-data: [%s] %s", name, error);
        unload_plugin_by_name(name);
        free(np);
        return NULL;
    }
    meta = metafn();
    LOG("PLugin status: [%s] %d", name, meta->status);
    if (!meta || meta->status != ANTD_PLUGIN_READY)
    {
        ERROR("Plugin is not ready or error: [%s].", name);
        unload_plugin_by_name(name);
        free(np);
        return NULL;
    }
    return np;
}
/**
 * Find a plugin in a file, and load it in to the plugin table
 * @param  name Name of the plugin
 * @return
 */
static void *plugin_from_file(char *name, char *path, dictionary_t conf)
{
    void *lib_handle;
    char *error;
    void (*fn)(plugin_header_t *, dictionary_t);
    lib_handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL /*| RTLD_NODELETE*/);
    if (!lib_handle)
    {
        ERROR("Cannot load plugin '%s' : '%s'", name, dlerror());
        return NULL;
    }
    fn = (void (*)(plugin_header_t *, dictionary_t))dlsym(lib_handle, "__init_plugin__");
    if ((error = dlerror()) != NULL)
        ERROR("Problem when finding plugin init function for %s : %s", name, error);
    else
    {
        plugin_header_t header;
        strncpy(header.name, name, MAX_PATH_LEN - 1);
        strncpy(header.dbpath, g_server_config.db_path, MAX_PATH_LEN - 1);
        strncpy(header.tmpdir, g_server_config.tmpdir, MAX_PATH_LEN - 1);
        strncpy(header.pdir, g_server_config.plugins_dir, MAX_PATH_LEN - 1);
        header.config = conf;
        header.raw_body = 0;
        header.status = ANTD_PLUGIN_INIT;
        (*fn)(&header, conf);
    }
    // trick libc that we close this lib, but it is not realy deleted
    return lib_handle;
}

void unload_plugin(struct plugin_entry *np)
{
    char *error;
    void (*fn)() = NULL;
    // find and execute the exit function
    fn = (void (*)())dlsym(np->handle, "__release__");
    if ((error = dlerror()) != NULL)
    {
        ERROR("Cant not release plugin %s : %s", np->name, error);
    }
    if (fn)
    {
        (*fn)();
    }
    dlclose(np->handle);
    LOG("Unloaded %s", np->name);
    // free((void *) np->handle);
    if (np->name)
        free((void *)np->name);
}
/*
    Unload a plugin by its name
*/
void unload_plugin_by_name(const char *name)
{
    struct plugin_entry *np;
    int hasval = hash(name, HASHSIZE);
    np = plugin_table[hasval];
    if (strcmp(np->name, name) == 0)
    {
        unload_plugin(np);
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
        unload_plugin(np->next);
        np->next = np->next->next;
    }
}
/**
 * Unload all the plugin loaded on the plugin table
 */
void unload_all_plugin()
{
    LOG("Unload all plugins");
    for (int i = 0; i < HASHSIZE; i++)
    {
        struct plugin_entry **np, *curr;
        np = &plugin_table[i];

        while ((curr = *np) != NULL)
        {
            (*np) = (*np)->next;
            unload_plugin(curr);
            free(curr);
        }
        plugin_table[i] = NULL;
    }
    exit(0);
}

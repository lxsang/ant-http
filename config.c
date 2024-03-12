#include <string.h>
#include <errno.h>
#include <unistd.h> 
#include "lib/ini.h"
#include "lib/utils.h"
#include "config.h"
#include "plugin_manager.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

extern config_t g_server_config;

// define all basic mime here
static mime_t _mimes[] = {
    {"image/bmp", "bmp"},
    {"image/jpeg", "jpg,jpeg"},
    {"text/css", "css"},
    {"text/markdown", "md"},
    {"text/csv", "csv"},
    {"application/pdf", "pdf"},
    {"image/gif", "gif"},
    {"text/html", "html"},
    {"application/json", "json"},
    {"application/javascript", "js"},
    {"image/png", "png"},
    {"text/plain", "txt"},
    {"application/xhtml+xml", "xhtml"},
    {"application/xml", "xml"},
    {"image/svg+xml", "svg"},
    {NULL, NULL}};

static void init_plugins()
{
    chain_t it, it2;
    dictionary_t config;
    const char *value;
    for_each_assoc(it, g_server_config.plugins)
    {
        config = (dictionary_t)it->value;
        if (config)
        {
            for_each_assoc(it2, config)
            {
                LOG("Plugin %s: [%s] -> [%s]", it->key, it2->key, (char *)it2->value);
                if (strncmp(it2->key, "file_type", 9) == 0 && it2->value)
                {
                    char *file_type = strdup((char *)it2->value);
                    char *token;
                    char *stringp = file_type;
                    while ((token = strsep(&stringp, ",")))
                    {
                        trim(token, ' ');
                        if (strlen(token) > 0)
                        {
                            dput(g_server_config.handlers, token, strdup((char *)it->key));
                            LOG("Plugin %s: support %s file", it->key, token);
                        }
                    }
                    free(file_type);
                }
            }
            value = (char *)dvalue(config, "autoload");
            if (value && (strncmp(value, "1", 1) == 0 || strncmp(value, "true", 3) == 0))
            {
                // load the plugin
                LOG("Plugin %s: autoloading...", it->key);
                plugin_load(it->key, config);
            }
        }
    }
}

static int config_handler(void *conf, const char *section, const char *name,
                          const char *value)
{
    config_t *pconfig = (config_t *)conf;
    regmatch_t regex_matches[2];
    char buf[255];
    char *tmp;
    struct stat st;
    // trim(section, ' ');
    // trim(value,' ');
    // trim(name,' ');
    // char * ppath = NULL;
    if (MATCH("SERVER", "plugins"))
    {
        if (stat(value, &st) == -1)
            mkdirp(value, 0755);
        tmp = realpath(value, NULL);
        if (!tmp)
        {
            ERROR("Unable to query real path for %s: %s", value, strerror(errno));
        }
        else
        {
            if (pconfig->plugins_dir)
                free(pconfig->plugins_dir);
            pconfig->plugins_dir = tmp;
            LOG("Plugin root is %s", pconfig->plugins_dir);
        }
    }
    else if (MATCH("SERVER", "plugins_ext"))
    {
        if (pconfig->plugins_ext)
            free(pconfig->plugins_ext);
        pconfig->plugins_ext = strdup(value);
    }
    else if (MATCH("SERVER", "database"))
    {
        if (stat(value, &st) == -1)
            mkdirp(value, 0700);
        tmp = realpath(value, NULL);
        if (!tmp)
        {
            ERROR("Unable to query real path for %s: %s", value, strerror(errno));
        }
        else
        {
            if (pconfig->db_path)
                free(pconfig->db_path);
            pconfig->db_path = tmp;
            LOG("Database root is %s", pconfig->db_path);
        }
    }
    else if (MATCH("SERVER", "tmpdir"))
    {
        if (stat(value, &st) == -1)
        {
            mkdirp(value, 0755);
        }
        else
        {
            removeAll(value, 0);
        }
        tmp = realpath(value, NULL);
        if (!tmp)
        {
            ERROR("Unable to query real path for %s: %s", value, strerror(errno));
        }
        else
        {
            if (pconfig->tmpdir)
                free(pconfig->tmpdir);
            pconfig->tmpdir = tmp;
            LOG("TMP root is %s", pconfig->tmpdir);
        }
    }
    else if (MATCH("SERVER", "statistic_fifo"))
    {
        if (pconfig->stat_fifo_path)
            free(pconfig->stat_fifo_path);
        pconfig->stat_fifo_path = strdup(value);
    }
    else if (MATCH("SERVER", "max_upload_size"))
    {
        pconfig->max_upload_size = atoi(value);
    }
    else if (MATCH("SERVER", "maxcon"))
    {
        pconfig->maxcon = atoi(value);
    }
    else if (MATCH("SERVER", "backlog"))
    {
        pconfig->backlog = atoi(value);
    }
    else if (MATCH("SERVER", "workers"))
    {
        pconfig->n_workers = atoi(value);
    }
    else if (MATCH("SERVER", "debug_enable"))
    {
        (void)setenv("ANTD_DEBUG", value, 1);
        pconfig->debug_enable = atoi(value);
    }
    else if (MATCH("SERVER", "scheduler_timeout"))
    {
        pconfig->scheduler_timeout = atoi(value);
    }
#ifdef USE_ZLIB
    else if (MATCH("SERVER", "gzip_enable"))
    {
        pconfig->gzip_enable = atoi(value);
    }
    else if (MATCH("SERVER", "gzip_types"))
    {
        pconfig->gzip_types = split(value, ",");
    }
#endif
#ifdef USE_OPENSSL
    else if (MATCH("SERVER", "ssl.cert"))
    {
        if (pconfig->sslcert)
            free(pconfig->sslcert);
        pconfig->sslcert = strdup(value);
    }
    else if (MATCH("SERVER", "ssl.key"))
    {
        if (pconfig->sslkey)
            free(pconfig->sslkey);
        pconfig->sslkey = strdup(value);
    }
    else if (MATCH("SERVER", "ssl.cipher"))
    {
        if (pconfig->ssl_cipher)
            free(pconfig->ssl_cipher);
        pconfig->ssl_cipher = strdup(value);
    }
#endif
    else if (strcmp(section, "MIMES") == 0)
    {
        dput(pconfig->mimes, name, strdup(value));
    }
    else if (regex_match("PORT:\\s*([0-9]+)", section, 2, regex_matches))
    {
        memset(buf, '\0', sizeof(buf));
        memcpy(buf, section + regex_matches[1].rm_so, regex_matches[1].rm_eo - regex_matches[1].rm_so);
        port_config_t *p = dvalue(pconfig->ports, buf);
        if (!p)
        {
            p = (port_config_t *)malloc(sizeof(port_config_t));
            p->htdocs = NULL;
            p->plugins = NULL;
            p->sock = -1;
            p->type = ANTD_PROTO_ALL;
            p->rules = dict_n(1);
            dput(pconfig->ports, buf, p);
            p->port = atoi(buf);
        }
        if (strcmp(name, "htdocs") == 0)
        {
            if (stat(value, &st) == -1)
            {
                mkdirp(value, 0755);
            }
            p->htdocs = realpath(value, NULL);
            if (!p->htdocs)
            {
                ERROR("Unable to query real path for %s: %s", value, strerror(errno));
                p->htdocs = strdup(value);
            }
            else
            {
                LOG("Server root is %s", p->htdocs);
            }
        }
        else if (strcmp(name, "plugins") == 0)
        {
            p->plugins = strdup(value);
        }
        else if (strcmp(name, "ssl.enable") == 0)
        {
            p->usessl = atoi(value);
            if (p->usessl)
                pconfig->enable_ssl = 1;
        }
        else if (strcmp(name, "protocol") == 0)
        {
            if (strcmp(value, "ipv4") == 0)
            {
                p->type = ANTD_PROTO_IP_4;
            }
            else if (strcmp(value, "ipv6") == 0)
            {
                p->type = ANTD_PROTO_IP_6;
            }
            else
            {
                ERROR("Unknown IP protocol setting %s. Enable both.", value);
            }
        }
        else
        {
            // other thing should be rules
            dput(p->rules, name, strdup(value));
        }
    }
    // plugin configuration
    else if (regex_match("PLUGIN:\\s*(.*)", section, 2, regex_matches))
    {
        memset(buf, '\0', sizeof(buf));
        memcpy(buf, section + regex_matches[1].rm_so, regex_matches[1].rm_eo - regex_matches[1].rm_so);
        dictionary_t p = dvalue(pconfig->plugins, buf);
        if (!p)
        {
            p = dict();
            dput(pconfig->plugins, buf, p);
        }
        dput(p, name, strdup(value));
    }
    else
    {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

void load_config(const char *file)
{
    g_server_config.ports = dict();
    g_server_config.plugins = dict();
    g_server_config.plugins_dir = strdup("plugins/");
    g_server_config.plugins_ext = strdup(".so");
    g_server_config.db_path = strdup("databases/");
    // g_server_config.htdocs = "htdocs/";
    g_server_config.tmpdir = strdup("/tmp/");
    g_server_config.stat_fifo_path = strdup("");
    g_server_config.n_workers = 4;
    g_server_config.backlog = 1000;
    g_server_config.handlers = dict();
    g_server_config.maxcon = 100;
    g_server_config.max_upload_size = 10000000; // 10Mb
    g_server_config.connection = 0;
    g_server_config.mimes = dict();
    g_server_config.enable_ssl = 0;
    g_server_config.sslcert = strdup("cert.pem");
    g_server_config.sslkey = strdup("key.pem");
    g_server_config.ssl_cipher = NULL;
    g_server_config.gzip_enable = 0;
    g_server_config.gzip_types = NULL;
    g_server_config.debug_enable = 0;
    g_server_config.scheduler_timeout = 30; // 30 s
    // put it default mimes
    for (int i = 0; _mimes[i].type != NULL; i++)
    {
        dput(g_server_config.mimes, _mimes[i].type, strdup(_mimes[i].ext));
    }
    if (ini_parse(file, config_handler, &g_server_config) < 0)
    {
        ERROR("Can't load '%s'. Used defaut configuration", file);
    }
    else
    {
        LOG("Using configuration : %s", file);
#ifdef USE_OPENSSL
        LOG("SSL enable %d", g_server_config.enable_ssl);
        LOG("SSL cert %s", g_server_config.sslcert);
        LOG("SSL key %s", g_server_config.sslkey);
        /*if(!g_server_config.ssl_cipher)
            LOG("SSL Cipher suite: %s", "HIGH");
        else
            LOG("SSL Cipher suite: %s", g_server_config.ssl_cipher);*/
#endif
    }
    LOG("%d mimes entries found", g_server_config.mimes->size);
    // Init plugins if necessary
    init_plugins();
}

void destroy_config()
{
    chain_t it;
    freedict(g_server_config.handlers);
    if (g_server_config.plugins_dir)
        free(g_server_config.plugins_dir);
    if (g_server_config.plugins_ext)
        free(g_server_config.plugins_ext);
    if (g_server_config.db_path)
        free(g_server_config.db_path);
    if (g_server_config.tmpdir)
        free(g_server_config.tmpdir);
    if (g_server_config.ssl_cipher)
        free(g_server_config.ssl_cipher);
    if (g_server_config.gzip_types)
        list_free(&g_server_config.gzip_types);
    if (g_server_config.mimes)
        freedict(g_server_config.mimes);
    if (g_server_config.stat_fifo_path)
        free(g_server_config.stat_fifo_path);
    if (g_server_config.plugins)
    {
        for_each_assoc(it, g_server_config.plugins)
        {
            freedict((dictionary_t)it->value);
        }
        freedict(g_server_config.plugins);
    }
    if (g_server_config.ports)
    {
        port_config_t *cnf;
        for_each_assoc(it, g_server_config.ports)
        {
            cnf = (port_config_t *)it->value;
            if (cnf != NULL)
            {
                if (cnf->htdocs != NULL)
                    free(cnf->htdocs);
                if (cnf->plugins)
                    free(cnf->plugins);
                if (cnf->sock > 0)
                {
                    close(cnf->sock);
                }
                freedict(cnf->rules);
            }
        }
        freedict(g_server_config.ports);
    }
    LOG("Unclosed connection: %d", g_server_config.connection);
}
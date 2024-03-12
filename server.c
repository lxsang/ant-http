#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/socket.h>
#include <poll.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
//#include <limits.h>
//#include <stdlib.h>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "server.h"
#include "lib/handle.h"
#include "plugin_manager.h"
#include "lib/scheduler.h"
#include "lib/utils.h"
#include "decode.h"
#include "config.h"


static pthread_mutex_t server_mux = PTHREAD_MUTEX_INITIALIZER;
config_t g_server_config;

void *execute_plugin(void *data, const char *pname);
void *serve_file(void *data);

void *accept_request(void *data)
{
    char buf[BUFFLEN];
    char *token = NULL;
    char *line = NULL;
    antd_task_t *task;
    antd_request_t *rq = (antd_request_t *)data;

    task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    // first verify if the socket is ready
    antd_client_t *client = (antd_client_t *)rq->client;

    struct pollfd pfd[1];

    pfd[0].fd = client->sock;
    pfd[0].events = POLLIN | POLLOUT;

    int sel = poll(pfd, 1, POLL_EVENT_TO);
    if (sel == -1)
    {
        antd_error(rq->client, 400, "Bad request");
        return task;
    }
    if (pfd[0].revents & POLLERR || pfd[0].revents & POLLHUP)
    {
        antd_error(rq->client, 400, "Bad request");
        return task;
    }
    if (sel == 0 || (!(pfd[0].revents & POLLIN) && !(pfd[0].revents & POLLOUT)))
    {
        task->handle = accept_request;
        return task;
    }
    // perform the ssl handshake if enabled
#ifdef USE_OPENSSL
    int ret = -1, stat;
    if (client->ssl && client->state == ANTD_CLIENT_ACCEPT)
    {
        // LOG("Atttempt %d\n", client->attempt);
        if (SSL_accept((SSL *)client->ssl) == -1)
        {
            stat = SSL_get_error((SSL *)client->ssl, ret);
            switch (stat)
            {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_NONE:
                task->handle = accept_request;
                return task;
            default:
                ERROR("Error performing SSL handshake %d %d %s", stat, ret, ERR_error_string(ERR_get_error(), NULL));
                antd_error(rq->client, 400, "Invalid SSL request");
                // server_config.connection++;
                ERR_print_errors_fp(stderr);
                return task;
            }
        }
        client->state = ANTD_CLIENT_HANDSHAKE;
        task->handle = accept_request;
        // LOG("Handshake finish for %d\n", client->sock);
        return task;
    }
    else
    {
        if (!((pfd[0].revents & POLLIN)))
        {
            task->handle = accept_request;
            return task;
        }
    }
#endif
    // LOG("Ready for reading %d\n", client->sock);
    // server_config.connection++;
    client->state = ANTD_CLIENT_PROTO_CHECK;
    read_buf(rq->client, buf, sizeof(buf));
    line = buf;
    LOG("Request (%d): %s", rq->client->sock, line);
    // get the method string
    token = strsep(&line, " ");
    if (!line)
    {
        // LOG("No method found");
        antd_error(rq->client, 405, "No method found");
        return task;
    }
    trim(token, ' ');
    trim(line, ' ');
    dput(rq->request, "METHOD", strdup(token));
    // get the request
    token = strsep(&line, " ");
    if (!line)
    {
        // LOG("No request found");
        antd_error(rq->client, 400, "Bad request");
        return task;
    }
    trim(token, ' ');
    trim(line, ' ');
    trim(line, '\n');
    trim(line, '\r');
    dput(rq->request, "PROTOCOL", strdup(line));
    dput(rq->request, "REQUEST_QUERY", strdup(token));
    line = token;
    token = strsep(&line, "?");
    dput(rq->request, "REQUEST_PATH", url_decode(token));
    // decode request
    // now return the task
    task->handle = decode_request_header;
    return task;
}

void *resolve_request(void *data)
{
    struct stat st;
    char path[2 * BUFFLEN];
    antd_request_t *rq = (antd_request_t *)data;
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    char *url = (char *)dvalue(rq->request, "REQUEST_URI");
    char *newurl = NULL;
    char *rqp = NULL;
    char *oldrqp = NULL;
    rq->client->state = ANTD_CLIENT_RESOLVE_REQUEST;
    char * root = (char *)dvalue(rq->request, "SERVER_WWW_ROOT");
    snprintf(path, sizeof(path), "%s/%s", root, url);
    LOG("URL is : %s", url);
    LOG("Resource Path is : %s", path);
    // if (path[strlen(path) - 1] == '/')
    //     strcat(path, "index.html");
    if (stat(path, &st) == -1)
    {
        free(task);
        rqp = strdup(url);
        oldrqp = rqp;
        trim(rqp, '/');
        newurl = strsep(&rqp, "/");
        if (!rqp)
            rqp = strdup("/");
        else
            rqp = strdup(rqp);
        dput(rq->request, "REQUEST_URI", rqp);
        dput(rq->request, "RESOURCE_PATH", __s("%s/%s", root,rqp));
        LOG("Execute plugin %s", newurl);
        task = execute_plugin(rq, newurl);
        free(oldrqp);
        return task;
    }
    else
    {
        if (S_ISDIR(st.st_mode))
        {
            strcat(path, "/index.html");
            if (stat(path, &st) == -1)
            {
                chain_t it;
                newurl = NULL;
                for_each_assoc(it, g_server_config.handlers)
                {
                    memset(path, 0, sizeof(path));
                    snprintf(path, sizeof(path), "%s/%s/index.%s", root, url, it->key);
                    if (stat(path, &st) == 0)
                    {
                        i = g_server_config.handlers->cap;
                        newurl = path;
                        break;
                    }
                }
                if (!newurl)
                {
                    antd_error(rq->client, 404, "Resource Not Found");
                    return task;
                }
            }
        }
        dput(rq->request, "RESOURCE_PATH", strdup(path));
        // check if the mime is supported
        // if the mime is not supported
        // find an handler plugin to process it
        // if the plugin is not found, forbidden access to the file should be sent
        char *mime_type = mime(path);
        dput(rq->request, "RESOURCE_MIME", strdup(mime_type));
        if (strcmp(mime_type, "application/octet-stream") == 0)
        {
            char *ex = ext(path);
            char *h = NULL;
            if (ex)
            {
                h = dvalue(g_server_config.handlers, ex);
                free(ex);
            }
            if (h)
            {
                // sprintf(path,"/%s%s",h,url);
                // LOG("WARNING::::Access octetstream via handle %s", h);
                // if(execute_plugin(client,buf,method,rq) < 0)
                //     cannot_execute(client);
                free(task);
                return execute_plugin(rq, h);
            }
            else
                antd_error(rq->client, 403, "Access forbidden");
        }
        else
        {
            // discard all request data
            dictionary_t headers = (dictionary_t)dvalue(rq->request, "REQUEST_HEADER");
            if (headers)
            {
                char *sclen = (char *)dvalue(headers, "Content-Length");
                unsigned clen = 0;
                unsigned read = 0;
                int count;
                if (sclen)
                {
                    clen = atoi(sclen);
                    while (read < clen)
                    {
                        count = antd_recv(rq->client, path, sizeof(path) < clen ? sizeof(path) : clen);
                        if (count <= 0)
                            break;
                        read += count;
                    }
                }
            }
            antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
            task->handle = serve_file;
        }
        return task;
    }
}

void *finish_request(void *data)
{
    if (!data)
        return NULL;
    destroy_request(data);
    g_server_config.connection--;
    LOG("Remaining connection %d", g_server_config.connection);
    return NULL;
}



void *serve_file(void *data)
{
    antd_request_t *rq = (antd_request_t *)data;
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    char *path = (char *)dvalue(rq->request, "RESOURCE_PATH");
    char *mime_type = (char *)dvalue(rq->request, "RESOURCE_MIME");
    rq->client->state = ANTD_CLIENT_SERVE_FILE;
    struct stat st;
    int s = stat(path, &st);

    if (s == -1)
    {
        antd_error(rq->client, 404, "File not found");
    }
    else
    {
        // check if it is modified
        dictionary_t header = (dictionary_t)dvalue(rq->request, "REQUEST_HEADER");
        char *last_modif_since = (char *)dvalue(header, "If-Modified-Since");
        time_t t = st.st_ctime;
        struct tm tm;
        if (last_modif_since)
        {
            strptime(last_modif_since, "%a, %d %b %Y %H:%M:%S GMT", &tm);
            t = timegm(&tm);
            // t = mktime(localtime(&t));
        }

        if (last_modif_since && st.st_ctime == t)
        {
            // return the not changed
            antd_error(rq->client, 304, "");
        }
        else
        {
            int size = (int)st.st_size;
            char ibuf[64];
            snprintf(ibuf, sizeof(ibuf), "%d", size);
            antd_response_header_t rhd;
            rhd.cookie = NULL;
            rhd.status = 200;
            rhd.header = dict();
            dput(rhd.header, "Content-Type", strdup(mime_type));
#ifdef USE_ZLIB
            if (!compressable(mime_type) || rq->client->z_level == ANTD_CNONE)
#endif
                dput(rhd.header, "Content-Length", strdup(ibuf));
            gmtime_r(&st.st_ctime, &tm);
            strftime(ibuf, 64, "%a, %d %b %Y %H:%M:%S GMT", &tm);
            dput(rhd.header, "Last-Modified", strdup(ibuf));
            dput(rhd.header, "Cache-Control", strdup("no-cache"));
            antd_send_header(rq->client, &rhd);

            __f(rq->client, path);
        }
    }

    return task;
}

static void *proxy_monitor(void *data)
{
    antd_request_t *rq = (antd_request_t *)data;
    rq->client->state = ANTD_CLIENT_PROXY_MONITOR;
    antd_client_t *proxy = (antd_client_t *)dvalue(rq->request, "PROXY_HANDLE");
    antd_task_t *task = antd_create_task(NULL, data, NULL, rq->client->last_io);
    int pret, ret, sz1 = 0, sz2 = 0;
    char *buf = NULL;
    buf = (char *)malloc(BUFFLEN);
    struct pollfd pfd[1];
    memset(pfd, 0, sizeof(pfd));
    pfd[0].fd = proxy->sock;
    pfd[0].events = POLLIN;
    ret = 1;

    do
    {
        sz1 = antd_recv_upto(rq->client, buf, BUFFLEN);

        if ((sz1 < 0) || (sz1 > 0 && antd_send(proxy, buf, sz1) != sz1))
        {
            ret = 0;
            break;
        }
        pret = poll(pfd, 1, 0);
        if (pret < 0)
        {
            (void)close(proxy->sock);
            return task;
        }
        sz2 = 0;
        if (pret > 0 && (pfd[0].revents & POLLIN))
        {
            sz2 = antd_recv_upto(proxy, buf, BUFFLEN);
            if (sz2 <= 0 || (sz2 > 0 && antd_send(rq->client, buf, sz2) != sz2))
            {
                ret = 0;
                break;
            }
        }
        if ((pret > 0) && (pfd[0].revents & POLLERR ||
                           pfd[0].revents & POLLRDHUP ||
                           pfd[0].revents & POLLHUP ||
                           pfd[0].revents & POLLNVAL))
        {
            ret = 0;
            break;
        }
    } while (sz1 > 0 || sz2 > 0);
    free(buf);

    if (ret == 0)
    {
        (void)close(proxy->sock);
        return task;
    }

    if (pfd[0].revents & POLLIN)
    {
        antd_task_bind_event(task, proxy->sock, 0, TASK_EVT_ON_READABLE);
    }
    else
    {
        antd_task_bind_event(task, proxy->sock, 50u, TASK_EVT_ON_TIMEOUT);
    }
    task->handle = proxy_monitor;
    task->access_time = rq->client->last_io;

    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
    return task;
}

void *proxify(void *data)
{
    int sock_fd, size, ret;
    char *str = NULL;
    chain_t it;
    antd_request_t *rq = (antd_request_t *)data;
    antd_client_t *proxy = NULL;
    rq->client->state = ANTD_CLIENT_RESOLVE_REQUEST;
    char *host = dvalue(rq->request, "PROXY_HOST");
    int port = atoi(dvalue(rq->request, "PROXY_PORT"));
    char *path = dvalue(rq->request, "PROXY_PATH");
    char *query = dvalue(rq->request, "PROXY_QUERY");
    char *ptr, *ip;
    dictionary_t xheader = dvalue(rq->request, "REQUEST_HEADER");
    antd_task_t *task = antd_create_task(NULL, data, NULL, rq->client->last_io);
    if (!xheader)
    {
        antd_error(rq->client, 400, "Badd Request");
        return task;
    }
    pthread_mutex_lock(&server_mux);
    ip = NULL;
    // ip_from_host is not threadsafe, need to lock it
    ptr = ip_from_hostname(host);
    if (ptr)
    {
        ip = strdup(ptr);
    }
    pthread_mutex_unlock(&server_mux);

    if (!ip)
    {
        antd_error(rq->client, 502, "Badd address");
        return task;
    }
    // TODO support ipv6
    sock_fd = antd_request_socket(ip, port);
    free(ip);
    if (sock_fd == -1)
    {
        antd_error(rq->client, 503, "Service Unavailable");
        return task;
    }
    set_nonblock(sock_fd);
    /*struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0; //POLL_EVENT_TO*1000;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        ERROR("setsockopt failed:%s", strerror(errno));
        antd_error(rq->client, 500, "Internal proxy error");
        (void)close(sock_fd);
        return task;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        ERROR("setsockopt failed:%s", strerror(errno));
        antd_error(rq->client, 500, "Internal proxy error");
        (void)close(sock_fd);
        return task;
    }*/

    proxy = (antd_client_t *)malloc(sizeof(antd_client_t));
    proxy->sock = sock_fd;
    proxy->ssl = NULL;
    proxy->zstream = NULL;
    proxy->z_level = ANTD_CNONE;
    time(&proxy->last_io);

    // store content length here
    dput(rq->request, "PROXY_HANDLE", proxy);

    str = __s("%s %s?%s HTTP/1.1\r\n", (char *)dvalue(rq->request, "METHOD"), path, query);
    size = strlen(str);
    ret = antd_send(proxy, str, size);
    free(str);
    if (ret != size)
    {
        antd_error(rq->client, 500, "");
        (void)close(sock_fd);
        return task;
    }
    for_each_assoc(it, xheader)
    {
        str = __s("%s: %s\r\n", it->key, (char *)it->value);
        size = strlen(str);
        ret = antd_send(proxy, str, size);
        free(str);
        if (ret != size)
        {
            antd_error(rq->client, 500, "");
            (void)close(sock_fd);
            return task;
        }
    }
    (void)antd_send(proxy, "\r\n", 2);
    // now monitor the proxy
    task->handle = proxy_monitor;
    task->access_time = rq->client->last_io;
    // register event
    antd_task_bind_event(task, proxy->sock, 0, TASK_EVT_ON_READABLE | TASK_EVT_ON_WRITABLE);
    return task;
}







/**
 * Execute a plugin based on the http requeset
 * First decode the http request header to find the correct plugin
 * and the correct function on the plugin
 * Second, decode all parameters necessary of the request and pass it
 * to the callback function.
 * Execute the callback function if sucess
 * @param  client       soket client
 * @param  path         request path
 * @param  method       request method
 * @param  query_string GET query string
 * @return              -1 if failure
 *                      1 if sucess
 */
void *execute_plugin(void *data, const char *pname)
{
    void *(*fn)(void *);
    plugin_header_t *(*metafn)();
    plugin_header_t *meta = NULL;
    struct plugin_entry *plugin;
    char *error;
    char pattern[256];

    antd_request_t *rq = (antd_request_t *)data;
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);

    snprintf(pattern, sizeof(pattern), "\\b%s\\b", pname);
    char *port_s = (char *)dvalue(rq->request, "SERVER_PORT");
    port_config_t *pcnf = (port_config_t *)dvalue(g_server_config.ports, port_s);

    // check if plugin is enabled on this port
    if (!pcnf->plugins || !regex_match(pattern, pcnf->plugins, 0, NULL))
    {
        LOG("No plugin matched in [%s] using pattern [%s]", pcnf->plugins, pattern);
        antd_error(rq->client, 403, "Access forbidden");
        return task;
    }

    // LOG("Plugin name '%s'", pname);
    rq->client->state = ANTD_CLIENT_PLUGIN_EXEC;
    // load the plugin
    pthread_mutex_lock(&server_mux);
    plugin = plugin_load((char *)pname, dvalue(g_server_config.plugins, pname));
    pthread_mutex_unlock(&server_mux);
    if (plugin == NULL)
    {
        antd_error(rq->client, 503, "Requested service not found");
        return task;
    }
    // check if the plugin want rawbody or decoded body
    metafn = (plugin_header_t * (*)()) dlsym(plugin->handle, "meta");
    if ((error = dlerror()) == NULL)
    {
        meta = metafn();
    }
    // load the function
    fn = (void *(*)(void *))dlsym(plugin->handle, PLUGIN_HANDLER);
    if ((error = dlerror()) != NULL)
    {
        ERROR("Problem when finding %s method from %s : %s", PLUGIN_HANDLER, pname, error);
        antd_error(rq->client, 503, "Requested service not found");
        return task;
    }
    // check if we need the raw data or not
    if (meta && meta->raw_body == 1)
    {
        task->handle = fn;
    }
    else
    {
        free(task);
        task = antd_create_task(decode_post_request, (void *)rq, fn, rq->client->last_io);
    }
    return task;
}

dictionary_t mimes_list()
{
    return g_server_config.mimes;
}

#ifdef USE_ZLIB
int compressable(char *ctype)
{
    if (!g_server_config.gzip_enable || g_server_config.gzip_types == NULL)
        return 0;
    item_t it;
    list_for_each(it, g_server_config.gzip_types)
    {
        if (it->type == LIST_TYPE_POINTER && it->value.ptr && regex_match((const char *)it->value.ptr, ctype, 0, NULL))
        {
            return 1;
        }
    }
    return 0;
}
#endif

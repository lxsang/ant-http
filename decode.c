#ifdef USE_OPENSSL
#include <openssl/sha.h>
#else
#include "lib/sha1.h"
#endif
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#include "decode.h"
#include "lib/handle.h"
#include "lib/utils.h"
#include "lib/scheduler.h"
#include "server.h"
#include "lib/base64.h"
#include "config.h"

#define WS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define HEADER_MAX_SIZE 8192

extern config_t g_server_config;

static int rule_check(const char *k, const char *v, const char *host, const char *_url, const char *_query, char *buf)
{
    // first perfom rule check on host, if not success, perform on url
    regmatch_t key_matches[10];
    regmatch_t val_matches[2];
    char *query = strdup(_query);
    char *url = strdup(_url);
    int ret;
    char *target;
    char *tmp, rep[10];
    int idx = 0;
    memset(rep, 0, 10);
    LOG("Verify %s=%s on %s or %s", k, v, url, host);
    // 1 group
    if (!host || !(ret = regex_match(k, host, 10, key_matches)))
    {
        target = url;
        ret = regex_match(k, url, 10, key_matches);
    }
    else
        target = (char *)host;

    if (!ret)
    {
        free(url);
        free(query);
        return 0;
    }
    LOG("Match found on %s", target);
    tmp = (char *)v;
    char *search = "<([a-zA-Z0-9]+)>";
    while ((ret = regex_match(search, tmp, 2, val_matches)))
    {
        memcpy(buf + idx, tmp, val_matches[1].rm_so - 1);
        idx += val_matches[1].rm_so - 1;
        memcpy(rep, tmp + val_matches[1].rm_so, val_matches[1].rm_eo - val_matches[1].rm_so);
        if (strcasecmp(rep, "url") == 0)
        {
            memcpy(buf + idx, url, strlen(url));
            idx += strlen(url);
        }
        else if (strcasecmp(rep, "query") == 0)
        {
            memcpy(buf + idx, query, strlen(query));
            idx += strlen(query);
        }
        else if (match_int(rep))
        {
            int i = atoi(rep);
            memcpy(buf + idx, target + key_matches[i].rm_so, key_matches[i].rm_eo - key_matches[i].rm_so);
            idx += key_matches[i].rm_eo - key_matches[i].rm_so;
        }
        else if (strcasecmp(rep, "break") == 0)
        {
            // ignore it
            LOG("Found break command, will break after this rule");
        }
        else
        { // just keep it
            memcpy(buf + idx, tmp + val_matches[1].rm_so - 1, val_matches[1].rm_eo + 2 - val_matches[1].rm_so);
            idx += val_matches[1].rm_eo + 2 - val_matches[1].rm_so;
        }
        tmp += val_matches[1].rm_eo + 1;
        // break;
    }
    // now modify the match 2 group
    if (idx > 0)
    {
        if (tmp)
        {
            // copy the remainning of tmp
            memcpy(buf + idx, tmp, strlen(tmp));
            idx += strlen(tmp);
        }
        buf[idx] = '\0';
    }
    free(url);
    free(query);
    LOG("New URI is %s", buf);
    return 1;
}

static char *apply_rules(dictionary_t rules, const char *host, char *url)
{
    // rule check
    char *query_string = url;
    while ((*query_string != '?') && (*query_string != '\0'))
        query_string++;
    if (*query_string == '?')
    {
        *query_string = '\0';
        query_string++;
    }
    // char* oldurl = strdup(url);
    chain_t it;
    char *k;
    char *v;
    int should_break = 0;
    for_each_assoc(it, rules)
    {
        k = it->key;
        if (it->value)
        {
            v = (char *)it->value;
            // 1 group
            if (regex_match("<break>$", v, 0, NULL))
            {
                should_break = 1;
            }
            if (rule_check(k, v, host, url, query_string, url))
            {
                query_string = url;

                while ((*query_string != '?') && (*query_string != '\0'))
                    query_string++;
                if (*query_string == '?')
                {
                    *query_string = '\0';
                    query_string++;
                }
                if (should_break)
                {
                    i = rules->cap;
                    LOG("Break rule check as matched found at %s -> %s", k, v);
                    break;
                }
            }
        }
    }

    return strdup(query_string);
}

static void ws_confirm_request(void *client, const char *key)
{
    char buf[256];
    char rkey[128];
    char sha_d[20];
    char base64[64];
    strncpy(rkey, key, sizeof(rkey) - 1);
    int n = (int)sizeof(rkey) - (int)strlen(key);
    if (n < 0)
        n = 0;
    strncat(rkey, WS_MAGIC_STRING, n);
#ifdef USE_OPENSSL
    SHA_CTX context;
#else
    SHA1_CTX context;
#endif

    SHA1_Init(&context);
    SHA1_Update(&context, rkey, strlen(rkey));
    SHA1_Final((uint8_t *)sha_d, &context);
    Base64encode(base64, sha_d, 20);
    // send accept to client
    sprintf(buf, "HTTP/1.1 101 Switching Protocols\r\n");
    antd_send(client, buf, strlen(buf));
    sprintf(buf, "Upgrade: websocket\r\n");
    antd_send(client, buf, strlen(buf));
    sprintf(buf, "Connection: Upgrade\r\n");
    antd_send(client, buf, strlen(buf));
    sprintf(buf, "Sec-WebSocket-Accept: %s\r\n", base64);
    antd_send(client, buf, strlen(buf));
    sprintf(buf, "\r\n");
    antd_send(client, buf, strlen(buf));

    LOG("%s", "Websocket is now enabled for plugin");
}

static void *decode_request(void *data)
{
    antd_request_t *rq = (antd_request_t *)data;
    dictionary_t headers = dvalue(rq->request, "REQUEST_HEADER");
    int ws = 0;
    char *ws_key = NULL;
    char *method = NULL;
    char *tmp;
    antd_task_t *task = NULL;
    ws_key = (char *)dvalue(headers, "Sec-WebSocket-Key");
    tmp = (char *)dvalue(headers, "Upgrade");
    if (tmp && strcasecmp(tmp, "websocket") == 0)
        ws = 1;
    method = (char *)dvalue(rq->request, "METHOD");
    task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    if (EQU(method, "GET"))
    {
        // if(ctype) free(ctype);
        if (ws && ws_key != NULL)
        {
            ws_confirm_request(rq->client, ws_key);
            // insert wsocket flag to request
            // plugin should handle this ugraded connection
            // not the server
            dput(rq->request, "__web_socket__", strdup("1"));
        }
        // resolve task
        task->handle = resolve_request;
        return task;
    }
    else if (EQU(method, "HEAD") || EQU(method, "OPTIONS") || EQU(method, "DELETE"))
    {
        task->handle = resolve_request;
        return task;
    }
    else if (EQU(method, "POST") || EQU(method, "PUT") || EQU(method, "PATCH"))
    {
        task->handle = resolve_request;
        return task;
    }
    else
    {
        antd_error(rq->client, 501, "Request Method Not Implemented");
        return task;
    }
}

/**
 * Check if the current request is e reverse proxy
 * return a proxy task if this is the case
 */
static void *check_proxy(antd_request_t *rq, const char *path, const char *query)
{
    char *pattern = "^(https?)://([^:]+):([0-9]+)(.*)$";
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    char buff[256];
    regmatch_t matches[5];
    int ret, size;
    ret = regex_match(pattern, path, 5, matches);

    if (!ret)
    {
        free(task);
        return NULL;
    }

    if (matches[1].rm_eo - matches[1].rm_so == 5)
    {
        // https is not supported for now
        // TODO add https support
        antd_error(rq->client, 503, "Service Unavailable");
        return task;
    }
    // http proxy request
    size = matches[2].rm_eo - matches[2].rm_so < (int)sizeof(buff) ? matches[2].rm_eo - matches[2].rm_so : (int)sizeof(buff);
    (void)memcpy(buff, path + matches[2].rm_so, size);
    buff[size] = '\0';
    dput(rq->request, "PROXY_HOST", strdup(buff));

    size = matches[3].rm_eo - matches[3].rm_so < (int)sizeof(buff) ? matches[3].rm_eo - matches[3].rm_so : (int)sizeof(buff);
    (void)memcpy(buff, path + matches[3].rm_so, size);
    buff[size] = '\0';
    dput(rq->request, "PROXY_PORT", strdup(buff));

    dput(rq->request, "PROXY_PATH", strdup(path + matches[4].rm_so));
    dput(rq->request, "PROXY_QUERY", strdup(query));

    task->handle = proxify;
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE | TASK_EVT_ON_WRITABLE);
    return task;
}

/**
 * Decode the cookie header to a dictionary
 * @param  client The client socket
 * @return        The Dictionary socket or NULL
 */
static void decode_cookie(const char *line, dictionary_t dic)
{
    char *token, *token1;
    char *cpstr = strdup(line);
    char *orgcpy = cpstr;
    trim(cpstr, ' ');
    trim(cpstr, '\n');
    trim(cpstr, '\r');

    while ((token = strsep(&cpstr, ";")))
    {
        trim(token, ' ');
        token1 = strsep(&token, "=");
        if (token1 && token && strlen(token) > 0)
        {
            dput(dic, token1, strdup(token));
        }
    }
    free(orgcpy);
}

/**
 * Decode a query string (GET request or POST URL encoded) to
 * a dictionary of key-value
 * @param  query : the query string
 * @return       a dictionary of key-value
 */
static void decode_url_request(const char *query, dictionary_t dic)
{
    if (query == NULL)
        return;
    // str_copy = ;
    char *token;
    if (strlen(query) == 0)
        return;
    char *str_copy = strdup(query);
    char *org_copy = str_copy;
    // dictionary dic = dict();
    while ((token = strsep(&str_copy, "&")))
    {
        char *key;
        char *val = NULL;
        if (strlen(token) > 0)
        {
            key = strsep(&token, "=");
            if (key && strlen(key) > 0)
            {
                val = strsep(&token, "=");
                if (!val)
                    val = "";
                dput(dic, key, url_decode(val));
            }
        }
    }
    free(org_copy);
    // return dic;
}

/**
 * Decode the HTTP request header
 */

void *decode_request_header(void *data)
{
    antd_request_t *rq = (antd_request_t *)data;
    rq->client->state = ANTD_CLIENT_HEADER_DECODE;
    dictionary_t cookie = NULL;
    char *line;
    char *token;
    char *query = NULL;
    char *host = NULL;
    char buf[2 * BUFFLEN];
    int header_size = 0;
    int ret;
    char *url = (char *)dvalue(rq->request, "REQUEST_QUERY");
    dictionary_t xheader = dvalue(rq->request, "REQUEST_HEADER");
    dictionary_t request = dvalue(rq->request, "REQUEST_DATA");
    char *port_s = (char *)dvalue(rq->request, "SERVER_PORT");
    port_config_t *pcnf = (port_config_t *)dvalue(g_server_config.ports, port_s);
    antd_task_t *task = NULL;
    // first real all header
    // this for check if web socket is enabled

    while (((ret = read_buf(rq->client, buf, sizeof(buf))) > 0) && strcmp("\r\n", buf))
    {
        header_size += ret;
        line = buf;
        trim(line, '\n');
        trim(line, '\r');
        token = strsep(&line, ":");
        trim(token, ' ');
        trim(line, ' ');
        if (token && line && strlen(line) > 0)
        {
            verify_header(token);
            dput(xheader, token, strdup(line));
        }
        if (token != NULL && strcasecmp(token, "Cookie") == 0)
        {
            if (!cookie)
            {
                cookie = dict();
            }
            decode_cookie(line, cookie);
        }
        else if (token != NULL && strcasecmp(token, "Host") == 0)
        {
            host = strdup(line);
        }
        if (header_size > HEADER_MAX_SIZE)
        {
            antd_error(rq->client, 413, "Payload Too Large");
            ERROR("Header size too large (%d): %d vs %d", rq->client->sock, header_size, HEADER_MAX_SIZE);
            task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
            return task;
        }
    }
    if (ret == 0)
    {
        // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
        task = antd_create_task(decode_request_header, (void *)rq, NULL, rq->client->last_io);
        antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
        return task;
    }
    // check for content length size
    line = (char *)dvalue(xheader, "Content-Length");
    if (line)
    {
        int clen = atoi(line);
        if (clen > g_server_config.max_upload_size)
        {
            antd_error(rq->client, 413, "Request body data is too large");
            // dirty fix, wait for message to be sent
            // 100 ms sleep
            usleep(100000);
            task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
            return task;
        }
    }

#ifdef USE_ZLIB
    // check for gzip
    line = (char *)dvalue(xheader, "Accept-Encoding");
    if (line)
    {
        if (regex_match("gzip", line, 0, NULL))
        {
            rq->client->z_level = ANTD_CGZ;
        }
        else if (regex_match("deflate", line, 0, NULL))
        {
            rq->client->z_level = ANTD_CDEFL;
        }
        else
        {
            rq->client->z_level = ANTD_CNONE;
        }
    }
    else
    {
        rq->client->z_level = ANTD_CNONE;
    }
#endif
    // if(line) free(line);
    memset(buf, 0, sizeof(buf));
    strncat(buf, url, sizeof(buf) - 1);
    LOG("Original query (%d): %s", rq->client->sock, url);
    query = apply_rules(pcnf->rules, host, buf);
    LOG("Processed query: %s", query);
    if (cookie)
        dput(rq->request, "COOKIE", cookie);
    if (host)
        free(host);

    // check if this is a reverse proxy ?
    task = check_proxy(rq, buf, query);
    if (task)
    {
        if (query)
            free(query);
        return task;
    }
    LOG("REQUEST_URI:%s", buf);
    dput(rq->request, "REQUEST_URI", url_decode(buf));
    if (query)
    {
        decode_url_request(query, request);
        dput(rq->request, "REQUEST_QUERY", query);
        //if(url)
        //    free(url);
    }
    // header ok, now checkmethod
    task = antd_create_task(decode_request, (void *)rq, NULL, rq->client->last_io);
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE); //
    return task;
}




 /**
 * Decode post query string to string
 */
static char *post_data_decode(void *client, int len)
{
    char *query = (char *)malloc((len + 1) * sizeof(char));
    char *ptr = query;
    int readlen = len > BUFFLEN ? BUFFLEN : len;
    int read = 0, stat = 1;
    while (readlen > 0 && stat >= 0)
    {
        stat = antd_recv_upto(client, ptr + read, readlen);
        if (stat > 0)
        {
            read += stat;
            readlen = (len - read) > BUFFLEN ? BUFFLEN : (len - read);
        }
        if (stat == 0)
        {
            if (difftime(time(NULL), ((antd_client_t *)client)->last_io) > MAX_IO_WAIT_TIME)
            {
                stat = -1;
            }
            else
            {
                usleep(POLL_EVENT_TO * 1000);
            }
        }
    }

    if (read > 0)
        query[read] = '\0';
    else
    {
        free(query);
        query = NULL;
    }
    return query;
}

static void *decode_multi_part_request_data(void *data)
{
    // loop through each part separated by the boundary
    char *line;
    char *part_name = NULL;
    char *part_file = NULL;
    char *file_path;
    char buf[BUFFLEN];
    char *field;
    int len;
    // dictionary dic = NULL;
    int fd = -1;
    char *token, *keytoken, *valtoken;
    antd_request_t *rq = (antd_request_t *)data;
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    char *boundary = (char *)dvalue(rq->request, "MULTI_PART_BOUNDARY");
    dictionary_t dic = (dictionary_t)dvalue(rq->request, "REQUEST_DATA");
    // search for content disposition:
    while (((len = read_buf(rq->client, buf, sizeof(buf))) > 0) && !strstr(buf, "Content-Disposition:"))
        ;
    ;
    if (len <= 0 || !strstr(buf, "Content-Disposition:"))
    {
        return task;
    }
    char *boundend = __s("%s--", boundary);
    line = buf;
    // extract parameters from header
    while ((token = strsep(&line, ";")))
    {
        keytoken = strsep(&token, "=");
        if (keytoken && strlen(keytoken) > 0)
        {
            trim(keytoken, ' ');
            valtoken = strsep(&token, "=");
            if (valtoken)
            {
                trim(valtoken, ' ');
                trim(valtoken, '\n');
                trim(valtoken, '\r');
                trim(valtoken, '\"');
                if (strcmp(keytoken, "name") == 0)
                {
                    part_name = strdup(valtoken);
                }
                else if (strcmp(keytoken, "filename") == 0)
                {
                    part_file = strdup(valtoken);
                }
            }
        }
    }
    line = NULL;
    // get the binary data
    LOG("Part file: %s part name: %s", part_file, part_name);
    if (part_name != NULL)
    {
        // go to the beginning of data bock
        while ((len = read_buf(rq->client, buf, sizeof(buf))) > 0 && strncmp(buf, "\r\n", 2) != 0)
            ;
        ;

        if (part_file == NULL)
        {
            /**
             *  WARNING:
             * This allow only 1024 bytes of data (max),
             * out of this range, the data is cut out.
             * Need an efficient way to handle this
             */
            len = read_buf(rq->client, buf, sizeof(buf));
            if (len > 0)
            {
                line = buf;
                trim(line, '\n');
                trim(line, '\r');
                trim(line, ' ');
                dput(dic, part_name, strdup(line));
            }
            // find the next boundary
            while ((len = read_buf(rq->client, buf, sizeof(buf))) > 0 && !strstr(buf, boundary))
            {
                line = buf;
            }
        }
        else
        {
            file_path = __s("%s/%s.%u", g_server_config.tmpdir, part_file, (unsigned)time(NULL));
            fd = open(file_path, O_WRONLY | O_CREAT, 0600);
            if (fd > 0)
            {
                int totalsize = 0, len = 0;
                // read until the next boundary
                //  TODO: this is not efficient for big file
                //  need a solution
                while ((len = read_buf(rq->client, buf, sizeof(buf))) > 0 && !strstr(buf, boundary))
                {
                    len = guard_write(fd, buf, len);
                    totalsize += len;
                }

                // remove \r\n at the end
                lseek(fd, 0, SEEK_SET);
                // fseek(fp,-2, SEEK_CUR);
                totalsize -= 2;
                int stat = ftruncate(fd, totalsize);
                LOG("Write %d bytes to %s", totalsize, file_path);
                UNUSED(stat);
                close(fd);
                line = buf;

                field = __s("%s.file", part_name);
                dput(dic, field, strdup(part_file));
                free(field);
                field = __s("%s.tmp", part_name);
                dput(dic, field, strdup(file_path));
                free(field);
                field = __s("%s.size", part_name);
                dput(dic, field, __s("%d", totalsize));
                free(field);
                field = __s("%s.ext", part_name);
                dput(dic, field, ext(part_file));
                free(field);
            }
            else
            {
                ERROR("Cannot write file to :%s", file_path);
            }
            free(file_path);
            free(part_file);
        }
        free(part_name);
    }
    /**
     * The upload procedure may take time, the task access time should be updated
     * after the procedure finish
     */
    task->access_time = rq->client->last_io;
    // check if end of request
    if (line && strstr(line, boundend))
    {
        // LOG("End request %s", boundend);
        free(boundend);
        // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
        return task;
    }
    free(boundend);
    if (line && strstr(line, boundary))
    {
        // continue upload
        // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
        task->handle = decode_multi_part_request_data;
        return task;
    }
    // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
    return task;
}
/**
 * Decode the multi-part form data from the POST request
 * If it is a file upload, copy the file to tmp dir
 */
static void *decode_multi_part_request(void *data, const char *ctype)
{
    char *boundary;
    char line[BUFFLEN];
    char *str_copy = (char *)ctype;
    int len;
    antd_request_t *rq = (antd_request_t *)data;
    antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    // antd_task_bind_event(task, rq->client->sock, 0, );
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    // dictionary dic = NULL;
    boundary = strsep(&str_copy, "="); // discard first part
    boundary = str_copy;
    if (boundary && strlen(boundary) > 0)
    {
        // dic = dict();
        trim(boundary, ' ');
        dput(rq->request, "MULTI_PART_BOUNDARY", strdup(boundary));
        // find first boundary
        while (((len = read_buf(rq->client, line, sizeof(line))) > 0) && !strstr(line, boundary))
            ;
        if (len > 0)
        {
            task->handle = decode_multi_part_request_data;
        }
    }
    return task;
}


void *decode_post_request(void *data)
{
    antd_request_t *rq = (antd_request_t *)data;
    rq->client->state = ANTD_CLIENT_RQ_DATA_DECODE;
    dictionary_t request = dvalue(rq->request, "REQUEST_DATA");
    dictionary_t headers = dvalue(rq->request, "REQUEST_HEADER");
    char *ctype = NULL;
    int clen = -1;
    char *tmp;
    antd_task_t *task = NULL;
    ctype = (char *)dvalue(headers, "Content-Type");
    tmp = (char *)dvalue(headers, "Content-Length");
    if (tmp)
        clen = atoi(tmp);
    char *method = (char *)dvalue(rq->request, "METHOD");
    task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    if (!method || (!EQU(method, "POST") && !EQU(method, "PUT") && !EQU(method, "PATCH")))
        return task;
    if (ctype == NULL || clen == -1)
    {
        antd_error(rq->client, 400, "Bad Request, missing content description");
        return task;
    }
    // decide what to do with the data
    if (strstr(ctype, FORM_URL_ENCODE))
    {
        char *pquery = post_data_decode(rq->client, clen);
        if (pquery)
        {
            decode_url_request(pquery, request);
            free(pquery);
        }
        else if (clen > 0)
        {
            // WARN: this may not work on ssl socket
            // antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE | TASK_EVT_ON_WRITABLE);
            antd_error(rq->client, 400, "Bad Request, missing content data");
            return task;
        }
    }
    else if (strstr(ctype, FORM_MULTI_PART))
    {
        free(task);
        return decode_multi_part_request(rq, ctype);
    }
    else
    {
        /*let plugin hande this data as we dont known how to deal with it*/
        dput(request, "HAS_RAW_BODY", strdup("true"));
    }
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
    return task;
}
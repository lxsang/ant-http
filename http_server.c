#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#else
#include "lib/sha1.h"
#endif

#include "http_server.h"
#include "lib/handle.h"
#include "plugin_manager.h"
#include "lib/scheduler.h"
#include "lib/utils.h"
#include "lib/ini.h"
#include "lib/base64.h"

#define HEADER_MAX_SIZE 8192

//define all basic mime here
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

static pthread_mutex_t server_mux = PTHREAD_MUTEX_INITIALIZER;
config_t server_config;
config_t *config()
{
	return &server_config;
}
void destroy_config()
{
	freedict(server_config.handlers);
	if (server_config.plugins_dir)
		free(server_config.plugins_dir);
	if (server_config.plugins_ext)
		free(server_config.plugins_ext);
	if (server_config.db_path)
		free(server_config.db_path);
	if (server_config.tmpdir)
		free(server_config.tmpdir);
	if (server_config.ssl_cipher)
		free(server_config.ssl_cipher);
	if (server_config.gzip_types)
		list_free(&server_config.gzip_types);
	if (server_config.mimes)
		freedict(server_config.mimes);
	if (server_config.stat_fifo_path)
		free(server_config.stat_fifo_path);
	if (server_config.ports)
	{
		chain_t it;
		port_config_t *cnf;
		for_each_assoc(it, server_config.ports)
		{
			cnf = (port_config_t *)it->value;
			if (cnf != NULL)
			{
				if (cnf->htdocs != NULL)
					free(cnf->htdocs);
				if (cnf->sock > 0)
				{
					close(cnf->sock);
				}
				freedict(cnf->rules);
			}
		}
		freedict(server_config.ports);
	}
	LOG("Unclosed connection: %d", server_config.connection);
}

static int config_handler(void *conf, const char *section, const char *name,
						  const char *value)
{
	config_t *pconfig = (config_t *)conf;
	regmatch_t port_matches[2];
	struct stat st;
	//trim(section, ' ');
	//trim(value,' ');
	//trim(name,' ');
	//char * ppath = NULL;
	if (MATCH("SERVER", "plugins"))
	{
		if (pconfig->plugins_dir)
			free(pconfig->plugins_dir);
		pconfig->plugins_dir = strdup(value);
		if (stat(pconfig->plugins_dir, &st) == -1)
			mkdirp(pconfig->plugins_dir, 0755);
	}
	else if (MATCH("SERVER", "plugins_ext"))
	{
		if (pconfig->plugins_ext)
			free(pconfig->plugins_ext);
		pconfig->plugins_ext = strdup(value);
	}
	else if (MATCH("SERVER", "database"))
	{
		if (pconfig->db_path)
			free(pconfig->db_path);
		pconfig->db_path = strdup(value);
		if (stat(pconfig->db_path, &st) == -1)
			mkdirp(pconfig->db_path, 0755);
	}
	else if (MATCH("SERVER", "tmpdir"))
	{
		if (pconfig->tmpdir)
			free(pconfig->tmpdir);
		pconfig->tmpdir = strdup(value);
		if (stat(pconfig->tmpdir, &st) == -1)
			mkdirp(pconfig->tmpdir, 0755);
		else
		{
			removeAll(pconfig->tmpdir, 0);
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
	else if (strcmp(section, "FILEHANDLER") == 0)
	{
		dput(pconfig->handlers, name, strdup(value));
	}
	else if (strcmp(section, "AUTOSTART") == 0 || strcmp(section, "AUTOLOAD") == 0)
	{
		// The server section must be added before the autostart section
		// auto start plugin
		plugin_load((char *)value);
	}
	else if (strcmp(section, "MIMES") == 0)
	{
		dput(pconfig->mimes, name, strdup(value));
	}
	else if (regex_match("PORT:\\s*([0-9]+)", section, 2, port_matches))
	{
		char buf[20];
		memset(buf, '\0', sizeof(buf));
		memcpy(buf, section + port_matches[1].rm_so, port_matches[1].rm_eo - port_matches[1].rm_so);
		port_config_t *p = dvalue(pconfig->ports, buf);
		if (!p)
		{
			p = (port_config_t *)malloc(sizeof(port_config_t));
			p->htdocs = NULL;
			p->sock = -1;
			p->rules = dict_n(1);
			dput(pconfig->ports, buf, p);
			p->port = atoi(buf);
		}
		if (strcmp(name, "htdocs") == 0)
		{
			p->htdocs = strdup(value);
			if (stat(p->htdocs, &st) == -1)
			{
				mkdirp(p->htdocs, 0755);
			}
		}
		else if (strcmp(name, "ssl.enable") == 0)
		{
			p->usessl = atoi(value);
			if (p->usessl)
				pconfig->enable_ssl = 1;
		}
		else
		{
			// other thing should be rules
			dput(p->rules, name, strdup(value));
		}
	}
	else
	{
		return 0; /* unknown section/name, error */
	}
	return 1;
}

void load_config(const char *file)
{
	server_config.ports = dict();
	server_config.plugins_dir = strdup("plugins/");
	server_config.plugins_ext = strdup(".dylib");
	server_config.db_path = strdup("databases/");
	//server_config.htdocs = "htdocs/";
	server_config.tmpdir = strdup("tmp/");
	server_config.stat_fifo_path = strdup("/var/run/antd_stat");
	server_config.n_workers = 4;
	server_config.backlog = 1000;
	server_config.handlers = dict();
	server_config.maxcon = 100;
	server_config.max_upload_size = 10000000; //10Mb
	server_config.connection = 0;
	server_config.mimes = dict();
	server_config.enable_ssl = 0;
	server_config.sslcert = strdup("cert.pem");
	server_config.sslkey = strdup("key.pem");
	server_config.ssl_cipher = NULL;
	server_config.gzip_enable = 0;
	server_config.gzip_types = NULL;
	// put it default mimes
	for (int i = 0; _mimes[i].type != NULL; i++)
	{
		dput(server_config.mimes, _mimes[i].type, strdup(_mimes[i].ext));
	}
	if (ini_parse(file, config_handler, &server_config) < 0)
	{
		ERROR("Can't load '%s'. Used defaut configuration", file);
	}
	else
	{
		LOG("Using configuration : %s", file);
#ifdef USE_OPENSSL
		LOG("SSL enable %d", server_config.enable_ssl);
		LOG("SSL cert %s", server_config.sslcert);
		LOG("SSL key %s", server_config.sslkey);
		/*if(!server_config.ssl_cipher)
			LOG("SSL Cipher suite: %s", "HIGH");
		else
			LOG("SSL Cipher suite: %s", server_config.ssl_cipher);*/
#endif
	}
	LOG("%d mimes entries found", server_config.mimes->size);
}

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
		//LOG("Atttempt %d\n", client->attempt);
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
				//server_config.connection++;
				ERR_print_errors_fp(stderr);
				return task;
			}
		}
		client->state = ANTD_CLIENT_HANDSHAKE;
		task->handle = accept_request;
		//LOG("Handshake finish for %d\n", client->sock);
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
	//LOG("Ready for reading %d\n", client->sock);
	//server_config.connection++;
	client->state = ANTD_CLIENT_PROTO_CHECK;
	read_buf(rq->client, buf, sizeof(buf));
	line = buf;
	LOG("Request (%d): %s", rq->client->sock, line);
	// get the method string
	token = strsep(&line, " ");
	if (!line)
	{
		//LOG("No method found");
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
		//LOG("No request found");
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
	char *url = (char *)dvalue(rq->request, "RESOURCE_PATH");
	char *newurl = NULL;
	char *rqp = NULL;
	char *oldrqp = NULL;
	rq->client->state = ANTD_CLIENT_RESOLVE_REQUEST;
	htdocs(rq, path);
	strcat(path, url);
	//LOG("Path is : %s", path);
	//if (path[strlen(path) - 1] == '/')
	//	strcat(path, "index.html");
	if (stat(path, &st) == -1)
	{
		free(task);
		rqp = strdup((char *)dvalue(rq->request, "REQUEST_PATH"));
		oldrqp = rqp;
		trim(rqp, '/');
		newurl = strsep(&rqp, "/");
		if (!rqp)
			rqp = strdup("/");
		else
			rqp = strdup(rqp);
		dput(rq->request, "RESOURCE_PATH", rqp);
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
				for_each_assoc(it, server_config.handlers)
				{
					newurl = __s("%s/index.%s", url, it->key);
					memset(path, 0, sizeof(path));
					htdocs(rq, path);
					strcat(path, newurl);
					if (stat(path, &st) != 0)
					{
						free(newurl);
						newurl = NULL;
					}
					else
					{
						i = server_config.handlers->cap;
						break;
					}
				}
				if (!newurl)
				{
					antd_error(rq->client, 404, "Resource Not Found");
					return task;
				}
				//if(url) free(url); this is freed in the dput function
				url = newurl;
				dput(rq->request, "RESOURCE_PATH", url);
			}
		}
		dput(rq->request, "ABS_RESOURCE_PATH", strdup(path));
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
				h = dvalue(server_config.handlers, ex);
				free(ex);
			}
			if (h)
			{
				//sprintf(path,"/%s%s",h,url);
				//LOG("WARNING::::Access octetstream via handle %s", h);
				//if(execute_plugin(client,buf,method,rq) < 0)
				//	cannot_execute(client);
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
	server_config.connection--;
	LOG("Remaining connection %d", server_config.connection);
	return NULL;
}

int rule_check(const char *k, const char *v, const char *host, const char *_url, const char *_query, char *buf)
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
		else
		{ // just keep it
			memcpy(buf + idx, tmp + val_matches[1].rm_so - 1, val_matches[1].rm_eo + 2 - val_matches[1].rm_so);
			idx += val_matches[1].rm_eo + 2 - val_matches[1].rm_so;
		}
		tmp += val_matches[1].rm_eo + 1;
		//break;
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
	return 1;
}

void *serve_file(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
	char *path = (char *)dvalue(rq->request, "ABS_RESOURCE_PATH");
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
			//t = mktime(localtime(&t));
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

int startup(unsigned *port)
{
	int httpd = 0;
	struct sockaddr_in name;
	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
	{
		ERROR("Port %d - socket: %s", *port, strerror(errno));
		return -1;
	}

	if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
	{
		ERROR("Unable to set reuse address on port %d - setsockopt: %s", *port, strerror(errno));
	}

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
	{
		ERROR("Port %d -bind: %s", *port, strerror(errno));
		return -1;
	}
	if (*port == 0) /* if dynamically allocating a port */
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
		{
			ERROR("Port %d - getsockname: %s", *port, strerror(errno));
			return -1;
		}
		*port = ntohs(name.sin_port);
	}

	LOG("back log is %d", server_config.backlog);
	if (listen(httpd, server_config.backlog) < 0)
	{
		ERROR("Port %d - listen: %s", *port, strerror(errno));
		return -1;
	}
	return (httpd);
}

char *apply_rules(dictionary_t rules, const char *host, char *url)
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
	//char* oldurl = strdup(url);
	chain_t it;
	char *k;
	char *v;
	for_each_assoc(it, rules)
	{
		k = it->key;
		v = (char *)it->value;
		// 1 group
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
		}
	}

	return strdup(query_string);
}

static void *proxy_monitor(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	rq->client->state = ANTD_CLIENT_PROXY_MONITOR;
	antd_client_t *proxy = (antd_client_t *)dvalue(rq->request, "PROXY_HANDLE");
	antd_task_t *task = antd_create_task(NULL, data, NULL, rq->client->last_io);
	int ret, sz1 = 0, sz2 = 0;
	char *buf = NULL;
	buf = (char *)malloc(BUFFLEN);
	struct pollfd pfd[1];
	memset(pfd, 0, sizeof(pfd));
	pfd[0].fd = proxy->sock;
	pfd[0].events = POLLIN;
	ret = 1;

	if (poll(pfd, 1, 0) < 0)
	{
		(void)close(proxy->sock);
		return task;
	}
	do
	{
		sz1 = antd_recv_upto(rq->client, buf, BUFFLEN);

		if ((sz1 < 0) || (sz1 > 0 && antd_send(proxy, buf, sz1) != sz1))
		{
			ret = 0;
			break;
		}
		sz2 = antd_recv_upto(proxy, buf, BUFFLEN);
		if (sz2 < 0 || (sz2 > 0 && antd_send(rq->client, buf, sz2) != sz2))
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
	if (sz2 == 0)
	{
		if (
			pfd[0].revents & POLLERR ||
			pfd[0].revents & POLLRDHUP ||
			pfd[0].revents & POLLHUP ||
			pfd[0].revents & POLLNVAL) //||
			//pfd[0].revents & POLLIN)
		{
			(void)close(proxy->sock);
			return task;
		}
	}
	if(pfd[0].revents & POLLIN)
	{
		antd_task_bind_event(task, proxy->sock, 0, TASK_EVT_ON_READABLE);
	}
	else
	{
		antd_task_bind_event(task, proxy->sock, 100u, TASK_EVT_ON_TIMEOUT);
	}
	task->handle = proxy_monitor;
	task->access_time = rq->client->last_io;
	
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
	return task;
}

static void *proxify(void *data)
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
	dictionary_t xheader = dvalue(rq->request, "REQUEST_HEADER");
	antd_task_t *task = antd_create_task(NULL, data, NULL, rq->client->last_io);
	if (!xheader)
	{
		antd_error(rq->client, 400, "Badd Request");
		return task;
	}
	sock_fd = request_socket(ip_from_hostname(host), port);
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
	port_config_t *pcnf = (port_config_t *)dvalue(server_config.ports, port_s);
	antd_task_t *task;
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
		//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
		task = antd_create_task(decode_request_header, (void *)rq, NULL, rq->client->last_io);
		antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
		return task;
	}
	// check for content length size
	line = (char *)dvalue(xheader, "Content-Length");
	if (line)
	{
		int clen = atoi(line);
		if (clen > server_config.max_upload_size)
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
	//if(line) free(line);
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

	dput(rq->request, "RESOURCE_PATH", url_decode(buf));
	if (query)
	{
		decode_url_request(query, request);
		free(query);
	}
	// header ok, now checkmethod
	task = antd_create_task(decode_request, (void *)rq, NULL, rq->client->last_io);
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE); //
	return task;
}

void *decode_request(void *data)
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
	//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
	if (EQU(method, "GET"))
	{
		//if(ctype) free(ctype);
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
	//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
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
			// task->handle = decode_post_request;
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
		char *pquery = post_data_decode(rq->client, clen);
		char *key = strstr(ctype, "/");
		if (key)
			key++;
		else
			key = ctype;
		if (pquery)
		{
			dput(request, key, strdup(pquery));
			free(pquery);
		}
		else if (clen > 0)
		{
			//task->handle = decode_post_request;
			//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE | TASK_EVT_ON_WRITABLE);
			antd_error(rq->client, 400, "Bad Request, missing content data");
			return task;
		}
	}
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
	return task;
}

/**
* Send header to the client to confirm 
* that the websocket is accepted by
* our server
*/
void ws_confirm_request(void *client, const char *key)
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
/**
 * Decode the cookie header to a dictionary
 * @param  client The client socket
 * @return        The Dictionary socket or NULL
 */
void decode_cookie(const char *line, dictionary_t dic)
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
 * Decode the multi-part form data from the POST request
 * If it is a file upload, copy the file to tmp dir
 */
void *decode_multi_part_request(void *data, const char *ctype)
{
	char *boundary;
	char line[BUFFLEN];
	char *str_copy = (char *)ctype;
	int len;
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
	//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
	//dictionary dic = NULL;
	boundary = strsep(&str_copy, "="); //discard first part
	boundary = str_copy;
	if (boundary && strlen(boundary) > 0)
	{
		//dic = dict();
		trim(boundary, ' ');
		dput(rq->request, "MULTI_PART_BOUNDARY", strdup(boundary));
		//find first boundary
		while (((len = read_buf(rq->client, line, sizeof(line))) > 0) && !strstr(line, boundary))
			;
		if (len > 0)
		{
			task->handle = decode_multi_part_request_data;
		}
	}
	return task;
}
void *decode_multi_part_request_data(void *data)
{
	// loop through each part separated by the boundary
	char *line;
	char *part_name = NULL;
	char *part_file = NULL;
	char *file_path;
	char buf[BUFFLEN];
	char *field;
	int len;
	//dictionary dic = NULL;
	FILE *fp = NULL;
	char *token, *keytoken, *valtoken;
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
	//antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
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
	if (part_name != NULL)
	{
		// go to the beginning of data bock
		while ((len = read_buf(rq->client, buf, sizeof(buf))) > 0 && strcmp(buf, "\r\n") != 0)
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
			file_path = __s("%s%s.%u", server_config.tmpdir, part_file, (unsigned)time(NULL));
			fp = fopen(file_path, "wb");
			if (fp)
			{
				int totalsize = 0, len = 0;
				//read until the next boundary
				// TODO: this is not efficient for big file
				// need a solution
				while ((len = read_buf(rq->client, buf, sizeof(buf))) > 0 && !strstr(buf, boundary))
				{
					fwrite(buf, len, 1, fp);
					totalsize += len;
				}
				//remove \r\n at the end
				fseek(fp, 0, SEEK_SET);
				//fseek(fp,-2, SEEK_CUR);
				totalsize -= 2;
				int stat = ftruncate(fileno(fp), totalsize);
				UNUSED(stat);
				fclose(fp);
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
	// check if end of request
	if (line && strstr(line, boundend))
	{
		//LOG("End request %s", boundend);
		free(boundend);
		antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
		return task;
	}
	free(boundend);
	if (line && strstr(line, boundary))
	{
		// continue upload
		antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_READABLE);
		task->handle = decode_multi_part_request_data;
		return task;
	}
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE);
	return task;
}
/**
 * Decode a query string (GET request or POST URL encoded) to  
 * a dictionary of key-value
 * @param  query : the query string
 * @return       a dictionary of key-value
 */
void decode_url_request(const char *query, dictionary_t dic)
{
	if (query == NULL)
		return;
	//str_copy = ;
	char *token;
	if (strlen(query) == 0)
		return;
	char *str_copy = strdup(query);
	char *org_copy = str_copy;
	//dictionary dic = dict();
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
	//return dic;
}
/**
* Decode post query string to string
*/
char *post_data_decode(void *client, int len)
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
			usleep(POLL_EVENT_TO*1000);
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
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
	antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
	//LOG("Plugin name '%s'", pname);
	rq->client->state = ANTD_CLIENT_PLUGIN_EXEC;
	//load the plugin
	if ((plugin = plugin_lookup((char *)pname)) == NULL)
	{
		pthread_mutex_lock(&server_mux);
		plugin = plugin_load((char *)pname);
		pthread_mutex_unlock(&server_mux);
		if (plugin == NULL)
		{
			antd_error(rq->client, 503, "Requested service not found");
			return task;
		}
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
	return server_config.mimes;
}

void dbdir(char *dest)
{
	strncpy(dest, server_config.db_path, 512);
}
void tmpdir(char *dest)
{
	strncpy(dest, server_config.tmpdir, 512);
}
void plugindir(char *dest)
{
	strncpy(dest, server_config.plugins_dir, 512);
}

#ifdef USE_ZLIB
int compressable(char *ctype)
{
	if (!server_config.gzip_enable || server_config.gzip_types == NULL)
		return 0;
	item_t it;
	list_for_each(it, server_config.gzip_types)
	{
		if (it->type == LIST_TYPE_POINTER && it->value.ptr && regex_match((const char *)it->value.ptr, ctype, 0, NULL))
		{
			return 1;
		}
	}
	return 0;
}
#endif

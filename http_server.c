#include "http_server.h"
static pthread_mutex_t server_mux = PTHREAD_MUTEX_INITIALIZER;
config_t server_config;
config_t *config()
{
	return &server_config;
}

void destroy_config()
{
	list_free(&(server_config.rules));
	freedict(server_config.handlers);
	if (server_config.plugins_dir)
		free(server_config.plugins_dir);
	if (server_config.plugins_ext)
		free(server_config.plugins_ext);
	if (server_config.db_path)
		free(server_config.db_path);
	if (server_config.htdocs)
		free(server_config.htdocs);
	if (server_config.tmpdir)
		free(server_config.tmpdir);

	LOG("Unclosed connection: %d\n", server_config.connection);
}

static int config_handler(void *conf, const char *section, const char *name,
						  const char *value)
{
	config_t *pconfig = (config_t *)conf;
	//char * ppath = NULL;
	if (MATCH("SERVER", "port"))
	{
		pconfig->port = atoi(value);
	}
	else if (MATCH("SERVER", "plugins"))
	{
		pconfig->plugins_dir = strdup(value);
	}
	else if (MATCH("SERVER", "plugins_ext"))
	{
		pconfig->plugins_ext = strdup(value);
	}
	else if (MATCH("SERVER", "database"))
	{
		pconfig->db_path = strdup(value);
	}
	else if (MATCH("SERVER", "htdocs"))
	{
		pconfig->htdocs = strdup(value);
	}
	else if (MATCH("SERVER", "tmpdir"))
	{
		pconfig->tmpdir = strdup(value);
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
#ifdef USE_OPENSSL
	else if (MATCH("SERVER", "ssl.enable"))
	{
		pconfig->usessl = atoi(value);
	}
	else if (MATCH("SERVER", "ssl.cert"))
	{
		pconfig->sslcert = strdup(value);
	}
	else if (MATCH("SERVER", "ssl.key"))
	{
		pconfig->sslkey = strdup(value);
	}
#endif
	else if (strcmp(section, "RULES") == 0)
	{
		list_put_s(&pconfig->rules, name);
		list_put_s(&pconfig->rules, value);
	}
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
	else
	{
		return 0; /* unknown section/name, error */
	}
	return 1;
}
void init_file_system()
{
	struct stat st;
	if (stat(server_config.plugins_dir, &st) == -1)
		mkdir(server_config.plugins_dir, 0755);
	if (stat(server_config.db_path, &st) == -1)
		mkdir(server_config.db_path, 0755);
	if (stat(server_config.htdocs, &st) == -1)
		mkdir(server_config.htdocs, 0755);
	if (stat(server_config.tmpdir, &st) == -1)
		mkdir(server_config.tmpdir, 0755);
	else
	{
		removeAll(server_config.tmpdir, 0);
	}
}
void load_config(const char *file)
{
	server_config.port = 8888;
	server_config.plugins_dir = "plugins/";
	server_config.plugins_ext = ".dylib";
	server_config.db_path = "databases/";
	server_config.htdocs = "htdocs/";
	server_config.tmpdir = "tmp/";
	server_config.n_workers = 4;
	server_config.backlog = 100;
	server_config.rules = list_init();
	server_config.handlers = dict();
	server_config.maxcon = 1000;
	server_config.connection = 0;
#ifdef USE_OPENSSL
	server_config.usessl = 0;
	server_config.sslcert = "cert.pem";
	server_config.sslkey = "key.pem";
#endif
	if (ini_parse(file, config_handler, &server_config) < 0)
	{
		LOG("Can't load '%s'\n. Used defaut configuration", file);
	}
	else
	{
		LOG("Using configuration : %s\n", file);
#ifdef USE_OPENSSL
		LOG("SSL enable %d\n", server_config.usessl);
		LOG("SSL cert %s\n", server_config.sslcert);
		LOG("SSL key %s\n", server_config.sslkey);
#endif
	}
	init_file_system();
}

void *accept_request(void *data)
{
	char buf[BUFFLEN];
	char *token = NULL;
	char *line = NULL;
	antd_task_t *task;
	antd_request_t *rq = (antd_request_t *)data;

	task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	fd_set read_flags, write_flags;
	// first verify if the socket is ready
	antd_client_t *client = (antd_client_t *)rq->client;
	FD_ZERO(&read_flags);
	FD_SET(rq->client->sock, &read_flags);
	FD_ZERO(&write_flags);
	FD_SET(rq->client->sock, &write_flags);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;
	// select
	int sel = select(client->sock + 1, &read_flags, &write_flags, (fd_set *)0, &timeout);
	if (sel == -1)
	{
		unknow(rq->client);
		return task;
	}
	if (sel == 0 || (!FD_ISSET(client->sock, &read_flags) && !FD_ISSET(client->sock, &write_flags)))
	{
		// retry it later
		task->handle = accept_request;
		return task;
	}
	// perform the ssl handshake if enabled
#ifdef USE_OPENSSL
	int ret = -1, stat;
	if (server_config.usessl == 1 && client->status == 0)
	{
		//if(client->attempt > MAX_ATTEMPT) return task;
		//LOG("Atttempt %d\n", client->attempt);
		if (SSL_accept((SSL *)client->ssl) == -1)
		{
			client->attempt++;
			stat = SSL_get_error((SSL *)client->ssl, ret);
			switch (stat)
			{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_NONE:
				//LOG("RECALL %d\n", stat);
				task->handle = accept_request;
				task->priority = HIGH_PRIORITY;
				task->type = LIGHT;
				return task;
			default:
				LOG("Error performing SSL handshake %d %d %lu\n", stat, ret, ERR_get_error());
				server_config.connection++;
				ERR_print_errors_fp(stderr);
				return task;
			}
		}
		client->attempt = 0;
		client->status = 1;
		task->handle = accept_request;
		return task;
	}
	else
	{
		if (!FD_ISSET(client->sock, &read_flags))
		{
			task->handle = accept_request;
			return task;
		}
	}
#endif
	server_config.connection++;
	read_buf(rq->client, buf, sizeof(buf));
	line = buf;
	// get the method string
	token = strsep(&line, " ");
	if (!line)
	{
		LOG("No method found\n");
		unknow(rq->client);
		return task;
	}
	trim(token, ' ');
	trim(line, ' ');
	dput(rq->request, "METHOD", strdup(token));
	// get the request
	token = strsep(&line, " ");
	if (!line)
	{
		LOG("No request found\n");
		unknow(rq->client);
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
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	char *url = (char *)dvalue(rq->request, "RESOURCE_PATH");
	char *newurl = NULL;
	char *rqp = NULL;
	char *oldrqp = NULL;
	strcpy(path, server_config.htdocs);
	strcat(path, url);
	LOG("Path is : %s \n", path);
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
				association it;
				for_each_assoc(it, server_config.handlers)
				{
					newurl = __s("%s/index.%s", url, it->key);
					memset(path, 0, sizeof(path));
					strcat(path, server_config.htdocs);
					strcat(path, newurl);
					if (stat(path, &st) != 0)
					{
						free(newurl);
						newurl = NULL;
					}
					else
					{
						break;
					}
				}
				if (!newurl)
				{
					notfound(rq->client);
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
			char *h = dvalue(server_config.handlers, ex);
			if (ex)
				free(ex);
			if (h)
			{
				//sprintf(path,"/%s%s",h,url);
				LOG("WARNING::::Access octetstream via handle %s\n", h);
				//if(execute_plugin(client,buf,method,rq) < 0)
				//	cannot_execute(client);
				free(task);
				return execute_plugin(rq, h);
			}
			else
				unknow(rq->client);
		}
		else
		{
			task->type = HEAVY;
			task->handle = serve_file;
		}
		return task;
	}
}

void *finish_request(void *data)
{
	if (!data)
		return NULL;
	LOG("Close request\n");
	antd_request_t *rq = (antd_request_t *)data;
	// free all other thing
	if (rq->request)
	{
		dictionary tmp = dvalue(rq->request, "COOKIE");
		if (tmp)
			freedict(tmp);
		tmp = dvalue(rq->request, "REQUEST_HEADER");
		if (tmp)
			freedict(tmp);
		tmp = dvalue(rq->request, "REQUEST_DATA");
		if (tmp)
			freedict(tmp);
		dput(rq->request, "REQUEST_HEADER", NULL);
		dput(rq->request, "REQUEST_DATA", NULL);
		dput(rq->request, "COOKIE", NULL);
		freedict(rq->request);
	}
	antd_close(rq->client);
	free(rq);
	server_config.connection--;
	LOG("Remaining connection %d\n", server_config.connection);
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
	//printf("match again %s\n",tmp);
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

static void error_die(const char *sc)
{
	perror(sc);
	exit(1);
}
void *serve_file(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	char *path = (char *)dvalue(rq->request, "ABS_RESOURCE_PATH");
	char *mime_type = (char *)dvalue(rq->request, "RESOURCE_MIME");
	ctype(rq->client, mime_type);
	if (is_bin(path))
		__fb(rq->client, path);
	else
		__f(rq->client, path);
	return task;
}

int startup(unsigned *port)
{
	int httpd = 0;
	struct sockaddr_in name;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
		error_die("socket");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		error_die("bind");
	if (*port == 0) /* if dynamically allocating a port */
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	printf("back log is %d\n", server_config.backlog);
	if (listen(httpd, server_config.backlog) < 0)
		error_die("listen");
	return (httpd);
}

char *apply_rules(const char *host, char *url)
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
	int size = list_size(server_config.rules);
	for (int i = 0; i < size; i += 2)
	{
		char *k, *v;
		k = list_at(server_config.rules, i)->value.s;
		v = list_at(server_config.rules, i + 1)->value.s;
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
/**
 * Decode the HTTP request header
 */

void *decode_request_header(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	dictionary cookie = NULL;
	char *line;
	char *token;
	char *query = NULL;
	char *host = NULL;
	char buf[2 * BUFFLEN];
	char *url = (char *)dvalue(rq->request, "REQUEST_QUERY");
	dictionary xheader = dict();
	dictionary request = dict();
	dput(rq->request, "REQUEST_HEADER", xheader);
	dput(rq->request, "REQUEST_DATA", request);
	// first real all header
	// this for check if web socket is enabled
	// ip address
	dput(xheader, "REMOTE_ADDR", (void *)strdup(((antd_client_t *)rq->client)->ip));
	dput(xheader, "SERVER_PORT", (void *)__s("%d", server_config.port));
	//while((line = read_line(client)) && strcmp("\r\n",line))
	while ((read_buf(rq->client, buf, sizeof(buf))) && strcmp("\r\n", buf))
	{
		line = buf;
		trim(line, '\n');
		trim(line, '\r');
		token = strsep(&line, ":");
		trim(token, ' ');
		trim(line, ' ');
		if (token && line && strlen(line) > 0)
			dput(xheader, token, strdup(line));
		if (token != NULL && strcasecmp(token, "Cookie") == 0)
		{
			if (!cookie)
				cookie = decode_cookie(line);
		}
		else if (token != NULL && strcasecmp(token, "Host") == 0)
		{
			host = strdup(line);
		}
	}
	//if(line) free(line);
	memset(buf, 0, sizeof(buf));
	strcat(buf, url);
	query = apply_rules(host, buf);
	dput(rq->request, "RESOURCE_PATH", url_decode(buf));
	if (query)
	{
		decode_url_request(query, request);
		free(query);
	}
	if (cookie)
		dput(rq->request, "COOKIE", cookie);
	if (host)
		free(host);
	// header ok, now checkmethod
	antd_task_t *task = antd_create_task(decode_request, (void *)rq, NULL);
	task->priority++;
	return task;
}

void *decode_request(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	dictionary headers = dvalue(rq->request, "REQUEST_HEADER");
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
	task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0)
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
	else if (strcmp(method, "POST") == 0)
	{
		task->handle = resolve_request;
		//task->type = HEAVY;
		return task;
	}
	else
	{
		unimplemented(rq->client);
		return task;
	}
}

void *decode_post_request(void *data)
{
	antd_request_t *rq = (antd_request_t *)data;
	dictionary request = dvalue(rq->request, "REQUEST_DATA");
	dictionary headers = dvalue(rq->request, "REQUEST_HEADER");
	char *ctype = NULL;
	int clen = -1;
	char *tmp;
	antd_task_t *task = NULL;
	ctype = (char *)dvalue(headers, "Content-Type");
	tmp = (char *)dvalue(headers, "Content-Length");
	if (tmp)
		clen = atoi(tmp);
	char *method = (char *)dvalue(rq->request, "METHOD");
	task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	task->type = HEAVY;
	if (!method || strcmp(method, "POST") != 0)
		return task;
	if (ctype == NULL || clen == -1)
	{
		LOG("Bad request\n");
		badrequest(rq->client);
		return task;
	}
	// decide what to do with the data
	if (strstr(ctype, FORM_URL_ENCODE))
	{
		char *pquery = post_data_decode(rq->client, clen);
		decode_url_request(pquery, request);
		free(pquery);
	}
	else if (strstr(ctype, FORM_MULTI_PART))
	{
		//printf("Multi part form : %s\n", ctype);
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
		dput(request, key, strdup(pquery));
		free(pquery);
	}
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
	strcpy(rkey, key);
	strcat(rkey, WS_MAGIC_STRING);
	//printf("RESPONDKEY '%s'\n", rkey);
#ifdef USE_OPENSSL
	SHA_CTX context;
#else
	SHA1_CTX context;
#endif

	SHA1_Init(&context);
	SHA1_Update(&context, rkey, strlen(rkey));
	SHA1_Final((uint8_t *)sha_d, &context);
	Base64encode(base64, sha_d, 20);
	//printf("Base 64 '%s'\n", base64);
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

	LOG("%s\n", "Websocket is now enabled for plugin");
}
/**
 * Decode the cookie header to a dictionary
 * @param  client The client socket
 * @return        The Dictionary socket or NULL
 */
dictionary decode_cookie(const char *line)
{
	char *token, *token1;
	char *cpstr = strdup(line);
	char *orgcpy = cpstr;
	trim(cpstr, ' ');
	trim(cpstr, '\n');
	trim(cpstr, '\r');

	dictionary dic = NULL;
	while ((token = strsep(&cpstr, ";")))
	{
		trim(token, ' ');
		token1 = strsep(&token, "=");
		if (token1 && token && strlen(token) > 0)
		{
			if (dic == NULL)
				dic = dict();
			dput(dic, token1, strdup(token));
		}
	}
	free(orgcpy);
	return dic;
}
/**
 * Decode the multi-part form data from the POST request
 * If it is a file upload, copy the file to tmp dir
 */
void *decode_multi_part_request(void *data, const char *ctype)
{
	char *boundary;
	char *line;
	char *str_copy = strdup(ctype);
	char *orgcpy = str_copy;
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	//dictionary dic = NULL;
	boundary = strsep(&str_copy, "="); //discard first part
	boundary = str_copy;
	if (boundary && strlen(boundary) > 0)
	{
		//dic = dict();
		trim(boundary, ' ');
		dput(rq->request, "MULTI_PART_BOUNDARY", strdup(boundary));
		//find first boundary
		while ((line = read_line(rq->client)) && !strstr(line, boundary))
		{
			if (line)
				free(line);
		}
		if (line)
		{
			task->handle = decode_multi_part_request_data;
			free(line);
		}
	}
	free(orgcpy);
	task->type = HEAVY;
	return task;
}
void *decode_multi_part_request_data(void *data)
{
	// loop through each part separated by the boundary
	char *line;
	char *orgline;
	char *part_name = NULL;
	char *part_file = NULL;
	char *file_path;
	char buf[BUFFLEN];
	char *field;
	//dictionary dic = NULL;
	FILE *fp = NULL;
	char *token, *keytoken, *valtoken;
	antd_request_t *rq = (antd_request_t *)data;
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	char *boundary = (char *)dvalue(rq->request, "MULTI_PART_BOUNDARY");
	dictionary dic = (dictionary)dvalue(rq->request, "REQUEST_DATA");
	char *boundend = __s("%s--", boundary);
	// search for content disposition:
	while ((line = read_line(rq->client)) &&
		   !strstr(line, "Content-Disposition:"))
	{
		free(line);
		line = NULL;
	}
	if (!line || !strstr(line, "Content-Disposition:"))
	{
		if (line)
			free(line);
		free(boundend);
		return task;
	}
	orgline = line;
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
	free(orgline);
	line = NULL;
	// get the binary data
	if (part_name != NULL)
	{
		// go to the beginning of data bock
		while ((line = read_line(rq->client)) && strcmp(line, "\r\n") != 0)
		{
			free(line);
			line = NULL;
		}
		if (line)
		{
			free(line);
			line = NULL;
		}
		if (part_file == NULL)
		{
			/**
			 * This allow only 1024 bytes of data (max),
			 * out of this range, the data is cut out.
			 * Need an efficient way to handle this
			 */
			line = read_line(rq->client);
			trim(line, '\n');
			trim(line, '\r');
			trim(line, ' ');
			dput(dic, part_name, line);
			// find the next boundary
			while ((line = read_line(rq->client)) && !strstr(line, boundary))
			{
				free(line);
				line = NULL;
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
				ftruncate(fileno(fp), totalsize);
				fclose(fp);
				line = strdup(buf);

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
				LOG("Cannot write file to :%s\n", file_path);
			}
			free(file_path);
			free(part_file);
		}
		free(part_name);
	}
	//printf("[Lines]:%s\n",line);
	// check if end of request
	if (line && strstr(line, boundend))
	{
		LOG("End request %s\n", boundend);
		free(line);
		free(boundend);
		return task;
	}
	if (line && strstr(line, boundary))
	{
		// continue upload
		task->type = HEAVY;
		task->handle = decode_multi_part_request_data;
	}
	free(line);
	free(boundend);
	return task;
}
/**
 * Decode a query string (GET request or POST URL encoded) to  
 * a dictionary of key-value
 * @param  query : the query string
 * @return       a dictionary of key-value
 */
void decode_url_request(const char *query, dictionary dic)
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
	while (readlen > 0 && stat > 0)
	{
		stat = antd_recv(client, ptr + read, readlen);
		if (stat > 0)
		{
			read += stat;
			readlen = (len - read) > BUFFLEN ? BUFFLEN : (len - read);
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
	antd_task_t *task = antd_create_task(NULL, (void *)rq, NULL);
	task->priority++;
	LOG("Plugin name '%s'\n", pname);

	//load the plugin
	if ((plugin = plugin_lookup((char *)pname)) == NULL)
	{
		pthread_mutex_lock(&server_mux);
		plugin = plugin_load((char *)pname);
		pthread_mutex_unlock(&server_mux);
		if (plugin == NULL)
		{
			unknow(rq->client);
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
		LOG("Problem when finding %s method from %s : %s \n", PLUGIN_HANDLER, pname, error);
		unknow(rq->client);
		return task;
	}
	// check if we need the raw data or not
	if (meta && meta->raw_body == 1)
	{
		task->handle = fn;
		task->type = HEAVY;
	}
	else
	{
		free(task);
		task = antd_create_task(decode_post_request, (void *)rq, fn);
		task->priority++;
	}
	return task;
}

#ifdef USE_OPENSSL
int usessl()
{
	return server_config.usessl;
}
#endif

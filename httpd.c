
#include <dirent.h>
#include "http_server.h"
#include "libs/ini.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
int server_sock = -1;
#ifdef USE_OPENSSL
static int ssl_session_ctx_id = 1;
SSL_CTX *ctx;
void init_openssl()
{ 
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
#if defined(SSL_CTX_set_ecdh_auto)
    SSL_CTX_set_ecdh_auto(ctx, 1);
#else
    SSL_CTX_set_tmp_ecdh(ctx, EC_KEY_new_by_curve_name(NID_X9_62_prime256v1));
#endif
    //SSL_CTX_set_ecdh_auto(ctx, 1);
	/* Set some options and the session id.
     * SSL_OP_NO_SSLv2: SSLv2 is insecure, disable it.
     * SSL_OP_NO_TICKET: We don't want TLS tickets used because this is an SSL server caching example.
     *                   It should be fine to use tickets in addition to server side caching.
     */
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2|SSL_OP_NO_TICKET);
    SSL_CTX_set_session_id_context(ctx, (void *)&ssl_session_ctx_id, sizeof(ssl_session_ctx_id));
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, server_config.sslcert, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, server_config.sslkey, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }
	if (!SSL_CTX_check_private_key(ctx)) {
        LOG("Failed to validate cert \n");
        ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }
}

#endif

static int config_handler(void* conf, const char* section, const char* name,
                   const char* value)
{
    config_t* pconfig = (config_t*)conf;
	//char * ppath = NULL;
    if (MATCH("SERVER", "port")) {
        pconfig->port = atoi(value);
    } else if (MATCH("SERVER", "plugins")) {
        pconfig->plugins_dir = strdup(value);
    } else if (MATCH("SERVER", "plugins_ext")) {
        pconfig->plugins_ext = strdup(value);
    } else if(MATCH("SERVER", "database")) {
        pconfig->db_path = strdup(value);
    } else if(MATCH("SERVER", "htdocs")) {
        pconfig->htdocs = strdup(value);
    } else if(MATCH("SERVER", "tmpdir")) {
        pconfig->tmpdir = strdup(value);
    }
 	else if(MATCH("SERVER", "maxcon")) {
        pconfig->maxcon = atoi(value);
    }
	else if(MATCH("SERVER", "backlog")) {
        pconfig->backlog = atoi(value);
    }
#ifdef USE_OPENSSL
	else if(MATCH("SERVER", "ssl.enable")) {
        pconfig->usessl = atoi(value);
    }
	else if(MATCH("SERVER", "ssl.cert")) {
        pconfig->sslcert = strdup(value);
    }
	else if(MATCH("SERVER", "ssl.key")) {
        pconfig->sslkey = strdup(value);
    }
#endif
	else if (strcmp(section, "RULES") == 0)
	{
		list_put_s(&pconfig->rules,  name);
		list_put_s(&pconfig->rules,  value);
    }
	else if (strcmp(section, "FILEHANDLER") == 0)
	{
		dput( pconfig->handlers, name ,strdup(value));
    }
	else if(strcmp(section,"AUTOSTART")==0){
		// The server section must be added before the autostart section
		// auto start plugin
		plugin_load(value);
    } else {
        return 0;  /* unknown section/name, error */
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
   		removeAll(server_config.tmpdir,0);
   	}

}
void load_config(const char* file)
{
	server_config.port = 8888;
	server_config.plugins_dir = "plugins/";
	server_config.plugins_ext = ".dylib";
	server_config.db_path = "databases/";
	server_config.htdocs = "htdocs";
	server_config.tmpdir = "tmp";
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
	if (ini_parse(file, config_handler, &server_config) < 0) {
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
void stop_serve(int dummy) {
	list_free(&(server_config.rules));
	freedict(server_config.handlers);
	LOG("Unclosed connection: %d\n", server_config.connection);
    unload_all_plugin();
#ifdef USE_OPENSSL
	SSL_CTX_free(ctx);
#endif
	close(server_sock);
}
int main(int argc, char* argv[])
{
// load the config first
	if(argc==1)
		load_config(CONFIG);
	else
		load_config(argv[1]);
	unsigned port = server_config.port;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	pthread_t newthread;
	char* client_ip = NULL;
	// ignore the broken PIPE error when writing 
	//or reading to/from a closed socked connection
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, SIG_IGN);
	signal(SIGINT, stop_serve);

#ifdef USE_OPENSSL
	if( server_config.usessl == 1 )
	{
		init_openssl();
    	ctx = create_context();

    	configure_context(ctx);
	}
    
#endif

	server_sock = startup(&port);
	LOG("httpd running on port %d\n", port);

	while (1)
	{
		if( server_config.connection >= server_config.maxcon )
		{
			LOG("Too many unclosed connection (%d). Wait for it\n", server_config.connection);
			continue;
		}
		antd_client_t* client = (antd_client_t*)malloc(sizeof(antd_client_t));
		client_sock = accept(server_sock,(struct sockaddr *)&client_name,&client_name_len);
		if (client_sock == -1)
		{
			perror("Cannot accept client request\n");
			continue;
		}
		/*
			get the remote IP
		*/
		if (client_name.sin_family == AF_INET)
		{
			client_ip =  inet_ntoa(client_name.sin_addr);
			LOG("Client IP: %s\n", client_ip);
		}
		//return &(((struct sockaddr_in6*)sa)->sin6_addr);
		/* accept_request(client_sock); */
		client->sock = client_sock;
		server_config.connection++;
		//LOG("Unclosed connection: %d\n", server_config.connection);
#ifdef USE_OPENSSL
		client->ssl = NULL;
		if(server_config.usessl == 1)
		{
			client->ssl = (void*)SSL_new(ctx);
			if(!client->ssl) continue;
        	SSL_set_fd((SSL*)client->ssl, client_sock);

        	if (SSL_accept((SSL*)client->ssl) <= 0) {
            	ERR_print_errors_fp(stderr);
				antd_close(client);
				continue;
        	}
		}
#endif
		if (pthread_create(&newthread , NULL,(void *(*)(void *))accept_request, (void *)client) != 0)
		{
			perror("pthread_create");
			antd_close(client);
		}
		else
		{
			//reclaim the stack data when thread finish
			pthread_detach(newthread) ;
		}
		//accept_request(&client);
	}

	close(server_sock);

	return(0);
}
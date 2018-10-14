
#include <dirent.h>
#include "http_server.h"
#include "libs/ini.h"
static  antd_scheduler_t scheduler;
static int server_sock = -1;

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
	/* use the full chain bundle of certificate */
    //if (SSL_CTX_use_certificate_file(ctx, server_config->sslcert, SSL_FILETYPE_PEM) <= 0) {
	if (SSL_CTX_use_certificate_chain_file(ctx, config()->sslcert) <= 0) {
	    ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, config()->sslkey, SSL_FILETYPE_PEM) <= 0 ) {
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

void stop_serve(int dummy) {
	UNUSED(dummy);
	sigset_t mask;
	sigemptyset(&mask);	
	//Blocks the SIG_IGN signal (by adding SIG_IGN to newMask)
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGPIPE);
	sigaddset(&mask, SIGABRT);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	antd_scheduler_destroy(&scheduler);
	unload_all_plugin();
	destroy_config(); 
#ifdef USE_OPENSSL
	FIPS_mode_set(0);
	SSL_CTX_free(ctx);
	FIPS_mode_set(0);
	CONF_modules_unload(1);
	EVP_cleanup();
	ENGINE_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	ERR_free_strings();
#endif
	if(server_sock != -1)
		close(server_sock);
	sigprocmask(SIG_UNBLOCK, &mask, NULL); 
}
int main(int argc, char* argv[])
{
// load the config first
	if(argc==1)
		load_config(CONFIG);
	else
		load_config(argv[1]);
	unsigned port = config()->port;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	char* client_ip = NULL;
	// ignore the broken PIPE error when writing 
	//or reading to/from a closed socked connection
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, SIG_IGN);
	signal(SIGINT, stop_serve);

#ifdef USE_OPENSSL
	if( config()->usessl == 1 )
	{
		init_openssl();
    	ctx = create_context();

    	configure_context(ctx);
	}
    
#endif
	server_sock = startup(&port);
	LOG("httpd running on port %d\n", port);
	// default to 4 workers
	antd_scheduler_init(&scheduler, config()->n_workers);
	// use blocking server_sock
	// make the scheduler wait for event on another thread
	// this allow to ged rid of high cpu usage on
	// endless loop without doing anything
    // set_nonblock(server_sock);
	pthread_t scheduler_th;
	if (pthread_create(&scheduler_th, NULL,(void *(*)(void *))antd_wait, (void*)&scheduler) != 0)
	{
		perror("pthread_create: cannot create worker\n");
		stop_serve(0);
		exit(1);
	}
	else
	{
		// reclaim data when exit
		pthread_detach(scheduler_th);
	}
	antd_task_t* task = NULL;
	while (scheduler.status)
	{
		client_sock = accept(server_sock,(struct sockaddr *)&client_name,&client_name_len);
		if (client_sock == -1)
		{
			continue;
		}
		antd_client_t* client = (antd_client_t*)malloc(sizeof(antd_client_t));
		antd_request_t* request = (antd_request_t*)malloc(sizeof(*request));
		request->client = client;
		request->request = dict();
		/*
			get the remote IP
		*/
		client->ip = NULL;
		if (client_name.sin_family == AF_INET)
		{
			client_ip =  inet_ntoa(client_name.sin_addr);
			client->ip = strdup(client_ip);
			LOG("Client IP: %s\n", client_ip);
			//LOG("socket: %d\n", client_sock);
		}

		// set timeout to socket
		set_nonblock(client_sock);
		/*struct timeval timeout;      
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;

		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed\n");

		if (setsockopt (client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed\n");
		*/
		client->sock = client_sock;
#ifdef USE_OPENSSL
		client->ssl = NULL;
		client->status = 0;
		client->attempt = 0;
		if(config()->usessl == 1)
		{
			client->ssl = (void*)SSL_new(ctx);
			if(!client->ssl) continue;
        	SSL_set_fd((SSL*)client->ssl, client->sock);

        	/*if (SSL_accept((SSL*)client->ssl) <= 0) {
				LOG("EROOR accept\n");
            	ERR_print_errors_fp(stderr);
				antd_close(client);
				continue;
        	}*/
		}
#endif
		// create callback for the server
		task = antd_create_task(accept_request,(void*)request, finish_request );
		task->type = LIGHT;
		antd_add_task(&scheduler, task);
	}

	close(server_sock);

	return(0);
}
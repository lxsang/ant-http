
#include <dirent.h>
#include "http_server.h"
#include "lib/ini.h"

static  antd_scheduler_t scheduler;

#ifdef USE_OPENSSL

// define the cipher suit used
// dirty hack, this should be configured by the configuration file
#define CIPHER_SUIT "HIGH"

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
		ERROR("Unable to create SSL context");
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
    SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1|SSL_OP_NO_TLSv1_1|SSL_OP_NO_SSLv2|SSL_OP_NO_TICKET);
    SSL_CTX_set_session_id_context(ctx, (void *)&ssl_session_ctx_id, sizeof(ssl_session_ctx_id));
    // set the cipher suit
	config_t * cnf = config();
	const char* suit = cnf->ssl_cipher?cnf->ssl_cipher:CIPHER_SUIT;
	LOG("Cirpher suit used: %s", suit);
    if (SSL_CTX_set_cipher_list(ctx, suit) != 1)
    {
		ERROR("Fail to set ssl cirpher suit: %s", suit);
       ERR_print_errors_fp(stderr);
       exit(EXIT_FAILURE);
    }
    /* Set the key and cert */
	/* use the full chain bundle of certificate */
    //if (SSL_CTX_use_certificate_file(ctx, server_config->sslcert, SSL_FILETYPE_PEM) <= 0) {
	if (SSL_CTX_use_certificate_chain_file(ctx, cnf->sslcert) <= 0) {
		ERROR("Fail to read SSL certificate chain file: %s", cnf->sslcert);
	    ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, cnf->sslkey, SSL_FILETYPE_PEM) <= 0 ) {
		ERROR("Fail to read SSL private file: %s", cnf->sslkey);
        ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
    }
	if (!SSL_CTX_check_private_key(ctx)) {
        ERROR("Failed to validate SSL certificate");
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
#ifdef USE_OPENSSL
	FIPS_mode_set(0);
	SSL_CTX_free(ctx);
	FIPS_mode_set(0);
	// DEPRECATED: CONF_modules_unload(1);
	EVP_cleanup();
	EVP_PBE_cleanup();
	// DEPRECATED:ENGINE_cleanup();
	CRYPTO_cleanup_all_ex_data();
	// DEPRECATED: ERR_remove_state(0);
	ERR_free_strings();
#endif
	destroy_config(); 
	sigprocmask(SIG_UNBLOCK, &mask, NULL); 
}

int main(int argc, char* argv[])
{
// load the config first
	if(argc==1)
		load_config(CONFIG_FILE);
	else
		load_config(argv[1]);
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	char* client_ip = NULL;
	// ignore the broken PIPE error when writing 
	//or reading to/from a closed socked connection
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, SIG_IGN);
	signal(SIGINT, stop_serve);

	config_t* conf = config();

#ifdef USE_OPENSSL
	if( conf->enable_ssl == 1 )
	{
		init_openssl();
    	ctx = create_context();

    	configure_context(ctx);
	}
    
#endif
	// startup port
	chain_t it;
	port_config_t * pcnf;
	int nlisten = 0;
	for_each_assoc(it, conf->ports)
	{
		pcnf = (port_config_t*)it->value;
		if(pcnf)
		{
			pcnf->sock = startup(&pcnf->port);
			if(pcnf->sock>0)
			{
				nlisten++;
				set_nonblock(pcnf->sock);
				LOG("Listening on port %d", pcnf->port);
			}
			else
			{
				ERROR("Port %d is disabled", pcnf->port);
			}
		}
	}
	if(nlisten == 0)
	{
		ERROR("No port is listenned, quit!!");
		stop_serve(0);
		exit(1);
	}
	// default to 4 workers
	antd_scheduler_init(&scheduler, conf->n_workers);
	scheduler.validate_data = 1;
	scheduler.destroy_data = finish_request;
	// use blocking server_sock
	// make the scheduler wait for event on another thread
	// this allow to ged rid of high cpu usage on
	// endless loop without doing anything
    // set_nonblock(server_sock);
	pthread_t scheduler_th;
	if (pthread_create(&scheduler_th, NULL,(void *(*)(void *))antd_wait, (void*)&scheduler) != 0)
	{
		ERROR("pthread_create: cannot create worker");
		stop_serve(0);
		exit(1);
	}
	else
	{
		// reclaim data when exit
		pthread_detach(scheduler_th);
	}
	antd_task_t* task = NULL;

	fd_set read_flags, write_flags;
	// first verify if the socket is ready
	struct timeval timeout;
	// select

	while (scheduler.status)
	{
		if(conf->connection > conf->maxcon)
		{
			//ERROR("Reach max connection %d", conf->connection);
			timeout.tv_sec = 0;
			timeout.tv_usec = 5000; // 5 ms
			select(0, NULL, NULL, NULL, &timeout);
			continue;
		}
		for_each_assoc(it, conf->ports)
		{
			pcnf = (port_config_t*) it->value;
			if(pcnf->sock > 0)
			{
				FD_ZERO(&read_flags);
				FD_SET(pcnf->sock, &read_flags);
				FD_ZERO(&write_flags);
				FD_SET(pcnf->sock, &write_flags);
				timeout.tv_sec = 0;
				timeout.tv_usec = 5000; // 5 ms
				int sel = select(pcnf->sock + 1, &read_flags, &write_flags, (fd_set *)0, &timeout);
				if(sel > 0 && (FD_ISSET(pcnf->sock, &read_flags) || FD_ISSET(pcnf->sock, &write_flags)))
				{
					client_sock = accept(pcnf->sock,(struct sockaddr *)&client_name,&client_name_len);
					if (client_sock > 0)
					{
						// just dump the scheduler when we have a connection
						antd_client_t* client = (antd_client_t*)malloc(sizeof(antd_client_t));
						antd_request_t* request = (antd_request_t*)malloc(sizeof(*request));
						request->client = client;
						request->request = dict();
						client->port_config = pcnf;
						/*
							get the remote IP
						*/
						client->ip = NULL;
						if (client_name.sin_family == AF_INET)
						{
							client_ip =  inet_ntoa(client_name.sin_addr);
							client->ip = strdup(client_ip);
							LOG("Connect to client IP: %s on port:%d", client_ip, pcnf->port);
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
						time(&client->last_io);
	#ifdef USE_OPENSSL
						client->ssl = NULL;
						client->status = 0;
						if(pcnf->usessl == 1)
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
						conf->connection++;
						// create callback for the server
						task = antd_create_task(accept_request,(void*)request, finish_request, client->last_io);
						//task->type = LIGHT;
						antd_add_task(&scheduler, task);
					}
				}
			}
		}
	}

	stop_serve(0);
	return(0);
}

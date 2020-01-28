
#include <dirent.h>
#include "http_server.h"
#include "lib/ini.h"
#define MAX_VALIDITY_INTERVAL 20

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
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
static unsigned char antd_protocols[] = {
	//TODO: add support to HTTP/2 protocol:
	2,'h', '2',
    8, 'h', 't', 't', 'p', '/', '1', '.', '1'
};
static int alpn_advertise_protos_cb(SSL *ssl, const unsigned char **out, unsigned int *outlen,void *arg)
{
	UNUSED(ssl);
	UNUSED(arg);
	*out = antd_protocols;
	*outlen = sizeof(antd_protocols);
	return SSL_TLSEXT_ERR_OK;
}
static int alpn_select_cb(SSL *ssl, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen, void *arg)
{
	UNUSED(arg);
	char buf[64];
	if(SSL_select_next_proto((unsigned char **)out, outlen,antd_protocols,sizeof(antd_protocols),in, inlen) == OPENSSL_NPN_NEGOTIATED)
	{
		// set client flag to indicate protocol
		int sock = SSL_get_fd(ssl);
		if(sock <= 0)
		{
			return SSL_TLSEXT_ERR_ALERT_FATAL;
		}
			
		antd_client_t* client = SSL_get_ex_data(ssl, sock);
		if(!client)
		{
			return SSL_TLSEXT_ERR_ALERT_FATAL;
		}
		memcpy(buf,*out,*outlen);
		buf[*outlen] = '\0';
		if(strcmp(buf,"http/1.1") !=0 )
		{
			client->flags &= ~CLIENT_FL_HTTP_1_1;
		}
		return SSL_TLSEXT_ERR_OK;
	}
	else
	{
		ERROR("No protocol support overlap found between client and server\n");
		return SSL_TLSEXT_ERR_ALERT_FATAL;
	}
}
#endif
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
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	SSL_CTX_set_alpn_select_cb(ctx,alpn_select_cb, NULL);
	SSL_CTX_set_next_protos_advertised_cb(ctx,alpn_advertise_protos_cb,NULL);
#endif
}

#endif


void stop_serve(int dummy) {
	UNUSED(dummy);
	// close log server
	closelog ();
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


static int validate_data(antd_task_t* task)
{
	if(difftime( time(NULL), task->access_time) > MAX_VALIDITY_INTERVAL)
		return 0;
	return 1;
}

static int is_task_ready(antd_task_t* task)
{
	antd_request_t* rq = (antd_request_t*)task->data;
	if(!rq) return 0;
	// check if data is ready for read/write
	fd_set read_flags, write_flags;
	struct timeval timeout;
	FD_ZERO(&read_flags);
	FD_SET(rq->client->sock, &read_flags);
	FD_ZERO(&write_flags);
	FD_SET(rq->client->sock, &write_flags);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0; 
	int sel = select(rq->client->sock + 1, &read_flags, &write_flags, (fd_set *)0, &timeout);
	if(sel > 0 && (FD_ISSET(rq->client->sock, &read_flags)|| FD_ISSET(rq->client->sock, &write_flags)))
	{
		if(FD_ISSET(rq->client->sock, &read_flags))
		{
			rq->client->flags |= CLIENT_FL_READABLE;
		}
		else
		{
			rq->client->flags &= ~CLIENT_FL_READABLE;
		}

		if(FD_ISSET(rq->client->sock, &write_flags))
		{
			rq->client->flags |= CLIENT_FL_WRITABLE;
		}
		else
		{
			rq->client->flags &= ~CLIENT_FL_WRITABLE;
		}
		return 1;
	}
	return 0;
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
	// start syslog
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog (SERVER_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

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
	scheduler.validate_data = validate_data;
	scheduler.destroy_data = finish_request;
	scheduler.task_ready = is_task_ready;
	
	
	// make the scheduler wait for event on another thread
	// this allow to ged rid of high cpu usage on
	// endless loop without doing anything
	pthread_t scheduler_th;
	if (pthread_create(&scheduler_th, NULL,(void *(*)(void *))antd_scheduler_wait, (void*)&scheduler) != 0)
	{
		ERROR("pthread_create: cannot create scheduler thread");
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
			timeout.tv_sec = 0;
			timeout.tv_usec = 10000; // 5 ms
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
				timeout.tv_usec = 10000; // 10 ms
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
						client->zstream = NULL;
						client->z_level = ANTD_CNONE;

						dictionary_t xheader = dict();
						dput(request->request, "REQUEST_HEADER", xheader);
						dput(request->request, "REQUEST_DATA", dict());
						dput(xheader, "SERVER_PORT", (void *)__s("%d", pcnf->port));
						dput(xheader, "SERVER_WWW_ROOT", (void*)strdup(pcnf->htdocs));
						/*
							get the remote IP
						*/
						if (client_name.sin_family == AF_INET)
						{
							client_ip =  inet_ntoa(client_name.sin_addr);
							LOG("Connect to client IP: %s on port:%d", client_ip, pcnf->port);
							// ip address
							dput(xheader, "REMOTE_ADDR", (void *)strdup(client_ip));
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
						client->ssl = NULL;
						// default selected protocol is http/1.1
						client->flags = CLIENT_FL_HTTP_1_1;
	#ifdef USE_OPENSSL
						if(pcnf->usessl == 1)
						{
							client->ssl = (void*)SSL_new(ctx);
							if(!client->ssl) continue;
							SSL_set_fd((SSL*)client->ssl, client->sock);
							// this can be used in the protocol select callback to
							// set the protocol selected by the server
							if(!SSL_set_ex_data((SSL*)client->ssl, client->sock, client))
							{
								ERROR("Cannot set ex data to ssl client:%d", client->sock);
							}
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

#include "http_server.h"
#include "libs/scheduler.h"
#include <fcntl.h>
/*
this node is a relay from the http
to https
*/
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
int server_sock = -1;
void stop_serve(int dummy) {
    UNUSED(dummy);
    antd_scheduler_destroy();
	close(server_sock);
}
/*
HTTP/1.1 301 Moved Permanently
Location: http://www.example.org/
Content-Type: text/html
Content-Length: 174
*/
void* antd_redirect(void* user_data)
{
    void** data = (void**)user_data;
    void* client = data[0];
    char* host = (char*)data[1];
    __t(client,"%s", "HTTP/1.1 301 Moved Permanently");
    __t(client, "Location: https://%s", host);
    __t(client, "%s", "Content-Type: text/html");
    __t(client, "");
    __t(client, "This page has moved to https://%s", host);
    free(host);
    free(user_data);
    return antd_create_task(NULL,client, NULL);
}

void* antd_free_client(void* client)
{
    antd_client_t * source = (antd_client_t *) client;
    if(source->ip) free(source->ip);
    close(source->sock);
    free(client);
    return NULL;
}

void* antd_get_host(void * client)
{
    char buff[1024];
    char* line, *token;
    char* host = NULL;
    while((read_buf(client,buff,sizeof(buff))) && strcmp("\r\n",buff))
	{
        line = buff;
        trim(line, '\n');
        trim(line, '\r');
        token = strsep(&line, ":");
        trim(token,' ');
		trim(line,' ');
        if(token && strcasecmp(token,"Host")==0)
            if(line)
            {
                host = strdup(line);
                break;
            }
    }
    if(!host) host = strdup("lxsang.me");
    void** data = (void**)malloc(2*(sizeof *data));
    data[0] = client;
    data[1] = (void*)host;
    LOG("[%s] Request for: %s --> https://%s\n", ((antd_client_t*)client)->ip, host, host);
    return antd_create_task(antd_redirect,data, NULL);
}

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
// load the config first
	unsigned port = 80;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	// ignore the broken PIPE error when writing 
	//or reading to/from a closed socked connection
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, SIG_IGN);
	signal(SIGINT, stop_serve);
	server_sock = startup(&port);
    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;
    // 0 worker
    antd_scheduler_init(0);
    antd_worker_t worker;
    worker.status = 0;
    // set server socket to non blocking
    fcntl(server_sock, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
	LOG("relayd running on port %d\n", port);

	while (antd_scheduler_status())
	{
        // execute task
        antd_attach_task(&worker);
		client_sock = accept(server_sock,(struct sockaddr *)&client_name,&client_name_len);
		if (client_sock == -1)
		{
			continue;
		}
        antd_client_t* client = (antd_client_t*)malloc(sizeof(antd_client_t));
		// set timeout to socket

		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed\n");

		if (setsockopt (client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed\n");

        /*
			get the remote IP
		*/
		client->ip = NULL;
		if (client_name.sin_family == AF_INET)
			client->ip = strdup(inet_ntoa(client_name.sin_addr));
        client->sock = client_sock;
		//accept_request(&client);
        antd_add_task(antd_create_task(antd_get_host,(void*)client, antd_free_client ));
	}

	return(0);
}
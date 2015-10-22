
#include <dirent.h>
#include "http_server.h"
#include "ini.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

static int config_handler(void* conf, const char* section, const char* name,
                   const char* value)
{
    config_t* pconfig = (config_t*)conf;

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
    }else {
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
	if (ini_parse(file, config_handler, &server_config) < 0) {
		LOG("Can't load '%s'\n. Used defaut configuration", file);
	}
	else
	{
		LOG("Using configuration : %s\n", file);
	}
	init_file_system();
}

int main(int argc, char* argv[])
{
// load the config first
	if(argc==1)
		load_config(CONFIG);
	else
		load_config(argv[1]);

	int server_sock = -1;
	unsigned port = server_config.port;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	pthread_t newthread;

	// ignore the broken PIPE error when writing 
	//or reading to/from a closed socked connection
	signal(SIGPIPE, SIG_IGN);

	server_sock = startup(&port);
	LOG("httpd running on port %d\n", port);

	while (1)
	{
		client_sock = accept(server_sock,(struct sockaddr *)&client_name,&client_name_len);
		if (client_sock == -1)
		{
			perror("Cannot accept client request\n");
			continue;
		}
		/* accept_request(client_sock); */
		if (pthread_create(&newthread , NULL,(void *(*)(void *))accept_request, (void *)client_sock) != 0)
			perror("pthread_create");
		else
		{
			//reclaim the stack data when thread finish
			pthread_detach(newthread) ;
		}
	}

	close(server_sock);

	return(0);
}
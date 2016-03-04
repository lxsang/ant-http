// network support
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <time.h>
#include <resolv.h>

#include "../plugin.h"
#define RQ 
#define REQUEST_PATTERN  "POST /node_register HTTP/1.0\r\nHost: antd\r\nUser-Agent: antd\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s"
#define JSON_MSG  "{\"ip\":\"%s\",\"port\":\"%d\"}"
void init();
struct master_conf_t{
	int port;
	char* ip;
} ;

struct master_conf_t mconfig;

call __init__ = init;
int request_socket(const char* ip, int port)
{
	int sockfd = -1;
	struct sockaddr_in dest;
	char* request;
	// time out setting
	struct timeval timeout;      
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		return -1;
	}
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof(timeout)) < 0)
	        perror("setsockopt failed\n");

    //if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
    //    perror("setsockopt failed\n");
	
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if ( inet_aton(ip, &dest.sin_addr.s_addr) == 0 )
    {
		perror(ip);
		close(sockfd);
		return -1;
    }
	if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
	{
		close(sockfd);
		//perror("Connect");
		return -1;
	}
	return sockfd;
}

char* get_ip_address()
{
	struct ifaddrs* addrs;
	getifaddrs(&addrs);
	struct ifaddrs* tmp = addrs;
	char* ip;
	while (tmp) 
	{
	    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
	    {
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
	        ip = inet_ntoa(pAddr->sin_addr);
			if(strcmp(ip,"127.0.0.1") != 0) 
				return ip;
	    }
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return "127.0.0.1";
}
int inform_master()
{
	int sockfd;
	//rpc_response_t* rdata = NULL;
	char* request;
	char* data = __s(JSON_MSG,get_ip_address(),__plugin__.sport);
	while((sockfd = request_socket(mconfig.ip, mconfig.port)) == -1)
	{
		// wait for 3s and then request to server
		usleep(3000000);
	} 
    request = __s(REQUEST_PATTERN, strlen(data), data);
	send(sockfd,request, strlen(request),0);
	//rdata = parse_response(sockfd);
	close(sockfd);
	LOG("%s","OK, master registered \n");
	free(request);
	return 0;
}
static int config_handler(void* conf, const char* section, const char* name,
                   const char* value)
{
    struct master_conf_t* pconfig = (struct master_conf_t*)conf;
	char * ppath = NULL;
    if (strcmp(name, "port") == 0) {
        pconfig->port = atoi(value);
    } else if (strcmp(name, "ip") == 0) {
		pconfig->ip = strdup(value);
    }  else {
        return 0;
    }
    return 1;
}
void read_config()
{
	mconfig.ip = "127.0.0.1";
	mconfig.port = 8080;
	char* file = __s("%s%s%s.ini",config_dir(), DIR_SEP, __plugin__.name);
	if (ini_parse(file, config_handler, &mconfig) < 0) {
		LOG("Can't load '%s'\n. Used defaut configuration", file);
	}
	printf("%s %d\n",mconfig.ip, mconfig.port );
} 
void init()
{
	read_config();
	pthread_t newthread;
	if (pthread_create(&newthread , NULL,(void(*)()) inform_master, NULL) != 0)
		perror("pthread_create: cannot create daemon for finding master");
	else
	{
		//reclaim the stack data when thread finish
		pthread_detach(newthread) ;
	}
}
void pexit()
{
	LOG("%s","EXIT daemon");
}
void execute(int c, const char* m, dictionary d)
{
	text(c);
	__t(c,"This is a system plugin. It cant be acessed from the web");
}
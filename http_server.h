#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include "plugin_manager.h"


#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: ant-httpd\r\n"

void accept_request(int);
void bad_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(unsigned *);
void unimplemented(int);
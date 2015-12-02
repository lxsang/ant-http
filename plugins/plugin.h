#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dbhelper.h"
#include "dictionary.h"
#include "list.h"


#define IS_POST(method) (strcmp(method,"POST")== 0)
#define IS_GET(method) (strcmp(method,"GET")== 0)
#define R_STR(d,k) ((char*)dvalue(d,k))
#define R_INT(d,k) (atoi(dvalue(d,k)))
#define R_FLOAT(d,k) ((double)atof(dvalue(d,k)))
#define R_PTR(d,k) (dvalue(d,k))
#define __RESULT__ "{\"result\":%d,\"msg\":\"%s\"}"

typedef struct  { 
    char *name; 
    char *dbpath;
    char * htdocs;
    char*pdir;
} plugin_header;

 

typedef void(*call)();

typedef sqlite3* sqldb;

extern plugin_header __plugin__;
extern call __init__;

int response(int, const char*);
void header_base(int);
void header(int,const char*);
void redirect(int,const char*);
void html(int);
void text(int);
void json(int);
void jpeg(int);
void octstream(int, char*);
void textstream(int);
int __ti(int,int);
int __t(int, const char*,...);
int __b(int, const unsigned char*, int);
int __f(int, const char*);
int __fb(int, const char*);
int upload(const char*, const char*);
char* route(const char*);
char* htdocs(const char*);
sqldb getdb();
void dbclose(sqldb);
void set_cookie(int,dictionary);

/*Default function for plugin*/
void execute(int, const char*,dictionary);

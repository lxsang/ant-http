#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "ws.h"
#include "scheduler.h"

typedef struct  { 
    char *name; 
    char *dbpath;
    char * htdocs;
    char*pdir;
	int sport;
#ifdef USE_OPENSSL
    int usessl;
#endif
} plugin_header_t;

 

//typedef void(*call)();
#ifdef USE_DB
typedef sqlite3* sqldb;
#endif


#ifdef USE_DB
sqldb getdb();
sqldb __getdb(char *name);
#endif

char* route(const char*);
char* htdocs(const char*);
char* config_dir();
/*Default function for plugin*/
// init the plugin
void init();
void destroy();
void* handle(void*);
plugin_header_t* meta();
#endif

#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "handle.h"

typedef struct  { 
    char *name; 
    char *dbpath;
    char * htdocs;
    char*pdir;
	int sport;
} plugin_header;

 

typedef void(*call)();
#ifdef USE_DB
typedef sqlite3* sqldb;
#endif
extern plugin_header __plugin__;
extern call __init__;


#ifdef USE_DB
sqldb getdb();
sqldb __getdb(char *name);
#endif

char* route(const char*);
char* htdocs(const char*);
char* config_dir();
/*Default function for plugin*/
void handler(int, const char*,const char*,dictionary);

#endif

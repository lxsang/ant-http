#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "ws.h"

typedef struct  { 
    char *name; 
    char *dbpath;
    char * htdocs;
    char*pdir;
	int sport;
#ifdef USE_OPENSSL
    int usessl;
#endif
} plugin_header;

 

//typedef void(*call)();
#ifdef USE_DB
typedef sqlite3* sqldb;
#endif
/*
Two server,
Two configuration different
Does it work
Replace this by a accessing function
that execute the set value to 
the header, instead of 
exporting global variables
*/
extern plugin_header __plugin__;
//extern call __init__;


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
void handle(void*, const char*,const char*,dictionary);
void __release();
#endif

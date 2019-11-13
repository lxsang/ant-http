#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "ws.h"
#include "scheduler.h"

 

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
void __attribute__((weak)) init();
void __attribute__((weak)) destroy();
void* handle(void*);
plugin_header_t* meta();
void use_raw_body();
#endif

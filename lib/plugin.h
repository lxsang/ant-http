#ifndef PLUGIN_H
#define PLUGIN_H


#include <sys/stat.h>

#include "utils.h"
#include "handle.h"
#include "dbhelper.h"
 

//typedef void(*call)();
#ifdef USE_DB
typedef sqlite3* sqldb;
#endif


#ifdef USE_DB
sqldb getdb();
sqldb __getdb(char *name);
#endif

char* config_dir();
/*Default function for plugin*/
// init the plugin
void  init();
void  destroy();
void* handle(void*);
plugin_header_t* meta();
void use_raw_body();

/*
STATIC PART, should be included in any plugin
*/
#ifdef PLUGIN_IMPLEMENT

#define PLUGIN_PANIC(a,...) \
		ERROR("%s: "a,__plugin__.name, ##__VA_ARGS__); \
		__plugin__.status = ANTD_PLUGIN_PANNIC;

static plugin_header_t __plugin__;
// private function
void __init_plugin__(plugin_header_t* pl, dictionary_t* conf){
	(void) memcpy(&__plugin__, pl, sizeof(plugin_header_t));
	__plugin__.status = ANTD_PLUGIN_READY;
	init();
}; 
void  use_raw_body()
{
	__plugin__.raw_body = 1;
}
#ifdef USE_DB
sqldb __getdb(char *name)
{
	int plen = strlen(name)+strlen(__plugin__.dbpath)+4;
	char* path = (char*) malloc(plen*sizeof(char));
	strcpy(path,__plugin__.dbpath);
	strcat(path,name);
	strcat(path,".db");
	//LOG("database: %s\n", path);
	sqldb ret = (sqldb)database(path);
	free(path);
	return ret;
}
sqldb getdb()
{
	return __getdb(__plugin__.name);
}
#endif

plugin_header_t* meta()
{
	return &__plugin__;
}

char* config_dir()
{
	struct stat st;
	char* path = __s("%s%s%s", __plugin__.pdir,DIR_SEP, __plugin__.name);
	if (stat(path, &st) == -1)
		mkdir(path, 0755);
	return path;
}

void __release__()
{
	destroy();
}
#endif
#endif

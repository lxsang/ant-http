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
void  init();
void  destroy();
void* handle(void*);
plugin_header_t* meta();
void use_raw_body();

/*
STATIC PART, should be included in any plugin
*/
#ifdef PLUGIN_IMPLEMENT
static plugin_header_t __plugin__;
// private function
void __init_plugin__(const char* pl,config_t* conf){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= conf->db_path;
	__plugin__.pdir = conf->plugins_dir;
	__plugin__.tmpdir = conf->tmpdir; 
	__plugin__.raw_body = 0;
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
char* route(const char* repath)
{
	int len = strlen(__plugin__.name) + 2;
	if(repath != NULL)
		len += strlen(repath)+1;
	char * path = (char*) malloc(len*sizeof(char));
	strcpy(path,"/");
	strcat(path,__plugin__.name);
	if(repath != NULL)
	{
		strcat(path,"/");
		strcat(path,repath);
	}
	return path;
}

const char* tmpdir()
{
	return (const char*) __plugin__.tmpdir;
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
	LOG("Releasing plugin\n");
	if(__plugin__.name) free(__plugin__.name);
	//if(__plugin__.dbpath) free(__plugin__.dbpath);
	//if(__plugin__.htdocs) free(__plugin__.htdocs);
	//if(__plugin__.pdir) free(__plugin__.pdir);
}
#endif
#endif

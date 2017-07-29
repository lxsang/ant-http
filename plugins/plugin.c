#include "plugin.h"

plugin_header __plugin__;
// private function
call __init__;

void __init_plugin__(const char* pl,const char*ph,const char* htdocs, const char* pdir,int port){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= strdup(ph);
	__plugin__.htdocs = strdup(htdocs);
	__plugin__.pdir = strdup(pdir);
	__plugin__.sport = port;
	if(__init__ != NULL) __init__();
}; 

#ifdef USE_DB
sqldb __getdb(char *name)
{
	int plen = strlen(name)+strlen(__plugin__.dbpath)+4;
	char* path = (char*) malloc(plen*sizeof(char));
	strcpy(path,__plugin__.dbpath);
	strcat(path,__plugin__.name);
	strcat(path,".db");
	sqldb ret = (sqldb)database(path);
	free(path);
	return ret;
}
sqldb getdb()
{
	return __getdb(__plugin__.name);
}
#endif

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

char* htdocs(const char* repath)
{
	if(repath != NULL)
		return __s("%s/%s",__plugin__.htdocs,repath);
	else
		return __s("%s",__plugin__.htdocs);
}

char* config_dir()
{
	struct stat st;
	char* path = __s("%s%s%s", __plugin__.pdir,DIR_SEP, __plugin__.name);
	if (stat(path, &st) == -1)
		mkdir(path, 0755);
	return path;
}
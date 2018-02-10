#include "plugin.h"

plugin_header __plugin__;
// private function
call __init__;
void __init_plugin__(const char* pl,config_t* conf){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= strdup(conf->db_path);
	__plugin__.htdocs = strdup(conf->htdocs);
	__plugin__.pdir = strdup(conf->plugins_dir);
	__plugin__.sport = conf->port;
#ifdef USE_OPENSSL
	__plugin__.usessl = conf->usessl;
#endif
	if(__init__ != NULL) __init__();
}; 

#ifdef USE_DB
sqldb __getdb(char *name)
{
	int plen = strlen(name)+strlen(__plugin__.dbpath)+4;
	char* path = (char*) malloc(plen*sizeof(char));
	strcpy(path,__plugin__.dbpath);
	strcat(path,name);
	strcat(path,".db");
	LOG("data base: %s\n", path);
	sqldb ret = (sqldb)database(path);
	free(path);
	return ret;
}
sqldb getdb()
{
	return __getdb(__plugin__.name);
}
#endif

/*#ifdef USE_OPENSSL
int usessl()
 {
	 LOG("CALLED from plugin \n");
	 return __plugin__.usessl;
 }
 #endif*/

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
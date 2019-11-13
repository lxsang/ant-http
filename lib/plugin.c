#include "plugin.h"

plugin_header_t __plugin__;
// private function
void __init_plugin__(const char* pl,config_t* conf){
	__plugin__.name = strdup(pl);
	__plugin__.dbpath= strdup(conf->db_path);
	__plugin__.htdocs = strdup(conf->htdocs);
	__plugin__.pdir = strdup(conf->plugins_dir);
	__plugin__.sport = conf->port;
#ifdef USE_OPENSSL
	__plugin__.usessl = conf->usessl;
#endif
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

/*#ifdef USE_OPENSSL
int usessl()
 {
	 LOG("CALLED from plugin \n");
	 return __plugin__.usessl;
 }
 #endif*/
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

void __release__()
{
	destroy();
	LOG("Releasing plugin\n");
	if(__plugin__.name) free(__plugin__.name);
	if(__plugin__.dbpath) free(__plugin__.dbpath);
	if(__plugin__.htdocs) free(__plugin__.htdocs);
	if(__plugin__.pdir) free(__plugin__.pdir);
}
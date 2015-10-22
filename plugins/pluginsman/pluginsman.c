#include "../plugin.h"
#define PEXT "dylib"
#define MAXSIZE 500000

void execute(int client,const char* method,dictionary rq)
{
	//all plugin file
	DIR           *d;
  	struct dirent *dir;
  	struct stat st;
  	d = opendir(__plugin__.pdir);
  	json(client);
  	__t(client,"{ \"records\":[");
  	int nrec = 0;
  	if (d)
  	{
   		while ((dir = readdir(d)) != NULL)
    	{
    		if(strcmp(dir->d_name,".") == 0 || 
    			strcmp(dir->d_name,"..")==0 || *(dir->d_name)=='.') continue;
      		if( stat(__s("%s%s",__plugin__.pdir,dir->d_name), &st) == 0 )
      		{
      			if(nrec != 0)
      				__t(client,",");
      			__t(client,"{\"name\":\"%s\",\"size\":%d,\"changed\":\"%s\"}",
      					dir->d_name,
      					(int)st.st_size,
      					__time(st.st_mtime)
      				);
      			nrec++;
      		}
    	}
    	closedir(d);
  	}
  	__t(client,"], \"total\":%d}",nrec);
	
}

void install(int c, const char* m, dictionary rq)
{
	char * result = "{\"result\":%d,\"msg\":\"%s\"}";
	json(c);
	if(IS_GET(m))
	{
		__t(c,result,0,"Bad request:GET");
		return;
	}
	if(R_INT(rq,"test") != NULL)
		LOG("Test is :%d \n",R_INT(rq,"test"));
	char * file_name = R_STR(rq,"pfile.file");
	char* file_ext = R_STR(rq,"pfile.ext");
	if(file_name == NULL)
	{
		__t(c,result,0,"Cannot send file to server");
		return;
	}

	if(strcasecmp(file_ext,PEXT) == 0)
	{
		int size = R_INT(rq,"pfile.size");
		if(size>MAXSIZE)
		{
			__t(c,result,0,"Cannot accept file more than 500Kb");
			return;
		}
		if(!upload(R_STR(rq,"pfile.tmp"),__s("%s/%s",__plugin__.pdir,file_name)))
		{
			__t(c,result,0,"Cannot move file to plugin dir");
			return;
		}
		__t(c,result,1,"OK");
		return;
	}
	
	__t(c,result,0,"This is not a plugin file");
}

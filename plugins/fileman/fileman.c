#include "../plugin.h"
#define MAXSIZE 500000
#define FRECORD "{\"recid\":%d,\"name\":\"%s\",\"size\":%d,\"changed\":\"%s\",\"type\":%d,\"style\":\"%s\",\"mime\":\"%s\"}"
#define RRECORD "{ \"records\":[%s], \"total\":%d,\"folder\":\"%s\",\"router\":[%s]}"
#define FOLLIST "{ \"name\":\"%s\", \"path\":\"%s\"}"

char* folder_list_from(const char* aPath)
{
	if(aPath == NULL || strlen(aPath)==0 || strcmp(aPath,DIR_SEP)==0) 
		return __s(FOLLIST,"HTDOCS",DIR_SEP);
	list path = split(aPath,DIR_SEP);
  	char* flist=__s(FOLLIST,"HTDOCS",DIR_SEP);
  	char* route = "";
  	if(path)
  	{
  		for(list np = path; np != NULL; np=np->next)
  		{
  			route = __s("%s%s%s", route, DIR_SEP, np->e.value.s);
  			if(flist == NULL)
  				flist = __s(FOLLIST, np->e.value.s,route);
  			else
  				flist = __s("%s,%s", flist,__s(FOLLIST,np->e.value.s,route));
  		}
  	}
  	free(path);
  	//free(route);
  	return flist;
}

void execute(int client,const char* method,dictionary rq)
{
	DIR           *d;
  	struct dirent *dir;
  	struct stat st;
  	int frec=0,rrec = 0, id =0;
  	char* flist = "",*dlist = "";
  	char* tmp= NULL;
  	char* rpath = R_STR(rq,"path"); 
  	if(!rpath || strlen(rpath) == 0) rpath = DIR_SEP;
  	char* path = __s("%s%s",__plugin__.htdocs,rpath);
  	d = opendir(path);
  	
  	if (d)
  	{
   		while ((dir = readdir(d)) != NULL)
    	{
    		//ignore curent directory, parent directory and hidden files and folders
    		if(strcmp(dir->d_name,".") == 0 || 
    			strcmp(dir->d_name,"..")==0|| *(dir->d_name)=='.') continue;
      		if( stat(__s("%s%s%s",path,DIR_SEP,dir->d_name), &st) == 0 )
      		{
      			if(S_ISDIR(st.st_mode))
      			{
      				tmp = __s(FRECORD,id,
      							dir->d_name,
      							(int)st.st_size,
      							__time(st.st_mtime),
      							0,
      							"font-weight: bold;",
      							"folder");
      				if(rrec != 0)
      					dlist = __s("%s,%s",dlist,tmp);
      				else
      					dlist = tmp;
      				rrec++;
      				//free(tmp);

      			} else
      			{
      				tmp = __s(FRECORD, id,
      							dir->d_name,
      							(int)st.st_size,
      							__time(st.st_mtime),1,"",
      							mime(dir->d_name));
      				if(frec != 0)
      					flist = __s("%s,%s",flist,tmp);
      				else
      					flist = tmp;
      				frec++;
      				//free(tmp);
      			}
      		id++;
      		}
    	}
    	closedir(d);
    	if(strlen(dlist) == 0)
    		dlist = flist;
    	else if(strlen(flist) > 0)
    		dlist = __s("%s,%s",dlist,flist);
  	}
  	json(client);
  	__t(client,RRECORD,dlist,frec+rrec,rpath,folder_list_from(rpath)); 
  	if(path) free(path);
  	//if(rpath) free(rpath);
  	if(tmp) free(tmp);
  	
}

void add(int c, const char* m, dictionary rq)
{
	json(c);
	if(IS_GET(m))
	{
		__t(c,__RESULT__,0,"Bad request:GET");
		return;
	}
	char* rpath = R_STR(rq,"path");
	if(!rpath) 
	{
		__t(c,__RESULT__,0,"Unknow path");
		return;
	}
	
	char * file_name = R_STR(rq,"pfile.file");
	if(file_name == NULL)
	{
		__t(c,__RESULT__,0,"Cannot send file to server");
		return;
	}

	int size = R_INT(rq,"pfile.size");
	if(size>MAXSIZE)
	{
		__t(c,__RESULT__,0,"Cannot accept file more than 500Kb");
		return;
	}
	if(!upload(R_STR(rq,"pfile.tmp"),__s("%s/%s/%s",__plugin__.htdocs,rpath,file_name)))
	{
		__t(c,__RESULT__,0,"Cannot move file to plugin dir");
		return;
	}
	__t(c,__RESULT__,1,"OK");
	return;
}

void mkfolder(int c, const char* m, dictionary rq)
{
	json(c);
	if(IS_GET(m))
	{
		__t(c,__RESULT__,0,"Bad request:GET");
		return;
	}
	char* dname = R_STR(rq,"dname");
	char* rpath = R_STR(rq, "path");
	if(!dname) 
	{
		__t(c,__RESULT__,0,"Folder name is empty");
		return;
	}
	if(!rpath) 
	{
		__t(c,__RESULT__,0,"Unknow path");
		return;
	}
	if(mkdir(__s("%s%s%s%s",__plugin__.htdocs,rpath,DIR_SEP,dname), 0755))
	{
		__t(c,__RESULT__,0,"Error when create directory.");
		return;
	}
	__t(c,__RESULT__,1,"OK");
}

void rmfolder(int c, const char* m, dictionary rq)
{
	json(c);
	if(IS_GET(m))
	{
		__t(c,__RESULT__,0,"Bad request:GET");
		return;
	}
	char* name = R_STR(rq,"name");
	char* rpath = R_STR(rq, "path");
	if(!name) 
	{
		__t(c,__RESULT__,0,"Folder name is empty");
		return;
	}
	if(!rpath) 
	{
		__t(c,__RESULT__,0,"Unknow path");
		return;
	}
	removeAll(__s("%s%s%s%s",__plugin__.htdocs,rpath,DIR_SEP,name),1);
	//LOG("%s\n",name );
	//LOG("%s\n",rpath );
	__t(c,__RESULT__,1,"OK");
}

#include "plugin_manager.h"
/**
 * Plugin table to store the loaded plugin
 */
static struct plugin_entry *plugin_table[HASHSIZE]; 
config_t server_config; 
/**
 * Decode the HTTP POST request with URL encode
 * @param  client socket client
 * @param  len    content length
 * @return        query string
 */
char* post_url_decode(int client,int len)
{
	char *query = (char*) malloc((len+1)*sizeof(char));
    for (int i = 0; i < len; i++) {
      recv(client, (query+i), 1, 0);
    }
    query[len]='\0';
    //query = url_decode(query);
    LOG("POST Query %s\n", query);
    return query;
}
/**
 * Decode the HTTP request
 * Get the cookie values
 * if it is the GET request, decode the query string into a dictionary
 * if it is a POST, check the content type of the request
 * 		- if it is a POST request with URL encoded : decode the url encode
 * 		- if it is a POST request with multipart form data: de code the multipart
 * 		- if other - UNIMPLEMENTED
 * @param  client socket client
 * @param  method HTTP method
 * @param  query  query string in case of GET
 * @return        a dictionary of key- value
 */
dictionary decode_request(int client,const char* method,const char* query)
{
	dictionary request = NULL;
	dictionary cookie = NULL;
	char* line;
	if(strcmp(method,"GET") == 0)
	{
		while((line = read_line(client)) && strcmp("\r\n",line))
		{
			if(!cookie) cookie = decode_cookie(line);
		}
		request = decode_url_request(query);
	}
	else
	{
		char * token;
		char* ctype = NULL;
		int clen = -1;
		line = read_line(client);
		while ((strlen(line) > 0) && strcmp("\r\n",line))
		{
			token = strsep(&line,":");
			trim(token,' ');
			if(token != NULL &&strcasecmp(token,"Content-Type") == 0)
			{
				ctype = strsep(&line,":");
				trim(ctype,' ');
				trim(ctype,'\n');
				trim(ctype,'\r');
			} else if(token != NULL &&strcasecmp(token,"Content-Length") == 0)
			{
				token = strsep(&line,":");
				trim(token,' ');
				clen = atoi(token);
			}
			else
			{
				if(!cookie) cookie = decode_cookie(line);
			}

			line = read_line(client);
		}
		free(line);
		if(ctype == NULL || clen == -1)
		{
			LOG("Bad request\n");
			return NULL;
		}
		// decide what to do with the data
		if(strstr(ctype,FORM_URL_ENCODE) > 0)
		{
			request = decode_url_request(post_url_decode(client,clen));
		} else if(strstr(ctype,FORM_MULTI_PART)> 0)
		{
			//printf("Multi part form : %s\n", ctype);
			request = decode_multi_part_request(client,ctype);
		} 
		else if(strstr(ctype,APP_JSON) > 0)
		{
			char* query = json_data_decode(client,clen);
			request = dict();
			dput(request,"json", strdup(query));
			free(query);
		}
		else
		{
			LOG("Un supported yet %s\n",ctype);
			return NULL;
		}
	}
	//if(cookie->key == NULL) {free(cookie);cookie= NULL;}
	if(cookie && !request)
			request = dict();
		
	dput(request,"cookie",cookie);
	return request;
}
void __px(const char* data,int size)
{
	for (int i = 0; i < size; ++i)
			printf(" %02x", data[i]);
			
	printf("\n");
}
/**
 * Decode the cookie header to a dictionary
 * @param  client The client socket
 * @return        The Dictionary socket or NULL
 */
dictionary decode_cookie(const char* line)
{
	char *token,*token1;
	char *cpstr = strdup(line);
	dictionary dic = NULL;
	token = strsep(&cpstr,":");
	trim(token,' ');
	if(token != NULL &&strcasecmp(token,"Cookie") == 0)
	{
		while((token = strsep(&cpstr,";")))
		{
			token1 = strsep(&token,"=");
			if(token1)
			{
				if(dic == NULL)
					dic = dict();
				LOG("Found cookie : %s = %s\n",token1,token);
				dput(dic,token1,token);
			}
		}
	}
	return dic;
	//free(cpstr);
}
/**
 * Decode the multi-part form data from the POST request
 * If it is a file upload, copy the file to tmp dir
 * and generate the metadata for the server-side
 * @param  client the socket client
 * @param  ctype  Content-Type of the request
 * @param  clen   Content length, but not used here
 * @return        a dictionary of key - value
 */
dictionary decode_multi_part_request(int client,const char* ctype)
{
	char * boundary;
	char * boundend;
	char * line;
	char * str_copy = strdup(ctype);
	char* token;
	char* keytoken ;
	char* valtoken ;
	char* part_name;
	char* part_file;
	char* file_path;
	char  buf[BUFFLEN];
	char* field;
	dictionary dic = NULL;
	FILE *fp = NULL;
	boundary = strsep(&str_copy,"="); //discard first part
	boundary = strsep(&str_copy,"="); 
	if(boundary && strlen(boundary)>0)
	{
		dic = dict();
		trim(boundary,' ');
		boundend = __s("%s--",boundary);
		//find first boundary
		while((line = read_line(client))&&strstr(line,boundary) <= 0); 
		// loop through each part separated by the boundary
		while(line && strstr(line,boundary) > 0){
			// search for content disposition:
			while((line = read_line(client)) &&
					strstr(line,"Content-Disposition:") <= 0);
			if(strstr(line,"Content-Disposition:") <= 0) return NULL;
			// extract parameters from header
			part_name = NULL;
			part_file = NULL;
			while((token = strsep(&line,";")))
			{
				keytoken = strsep(&token,"=");
				if(keytoken && strlen(keytoken)>0)
				{
					trim(keytoken,' ');
					valtoken = strsep(&token,"=");
					if(valtoken)
					{
						trim(valtoken,' ');
						trim(valtoken,'\n');
						trim(valtoken,'\r');
						trim(valtoken,'\"');
						if(strcmp(keytoken,"name") == 0)
						{
							part_name = valtoken;
						} else if(strcmp(keytoken,"filename") == 0)
						{
							part_file = valtoken;
						}
					}
				}
			}
			// get the binary data
			if(part_name != NULL)
			{
				// go to the beginer of data bock
				while((line = read_line(client)) && strcmp(line,"\r\n") != 0);
				if(part_file == NULL)
				{
					/**
					 * This allow only 1024 bytes of data (max),
					 * out of this range, the data is cut out.
					 * Need an efficient way to handle this
					 */
					line = read_line(client);
					trim(line,'\n');
					trim(line,'\r');
					trim(line,' ');
					dput(dic,part_name,line);
					// find the next boundary
					while((line = read_line(client)) && strstr(line,boundary) <= 0); 
				}
				else
				{
					file_path = __s("%s%s.%u",server_config.tmpdir,part_file,(unsigned)time(NULL));
					fp=fopen(file_path, "wb");
					if(fp)
					{
						int totalsize=0,len=0;
						//read until the next boundary
						while((len = read_buf(client,buf,sizeof(buf))) > 0 && strstr(buf,boundary) <= 0)
						{
							fwrite(buf, len, 1, fp);
							totalsize += len;
						}
						//remove \r\n at the end
						fseek(fp,-2, SEEK_CUR);
						totalsize -= 2;
						fclose(fp);
						line = buf;

						field = __s("%s.file",part_name);
						dput(dic,field,part_file);
						field = __s("%s.tmp",part_name);
						dput(dic,field,strdup(file_path));
						field = __s("%s.size",part_name);
						dput(dic,field,__s("%d",totalsize));
						field = __s("%s.ext",part_name);
						dput(dic,field,ext(part_file));
						free(field);

					}
					else
					{
						LOG("Cannot wirte file to :%s\n", file_path );
					}
					free(file_path);
				}
			}
			//printf("[Lines]:%s\n",line);
			// check if end of request
			if(line&&strstr(line,boundend)>0)
			{
				LOG("End request %s\n", boundend);
				break;
			}
		} 
	}
	free(str_copy);
	return dic;
}
/**
 * Decode a query string (GET request or POST URL encoded) to  
 * a dictionary of key-value
 * @param  query : the query string
 * @return       a dictionary of key-value
 */
dictionary decode_url_request(const char* query)
{
	char* str_copy = strdup(query);
	//str_copy = ;
	char* token;
	if(query == NULL) return NULL;
	if(strlen(query) == 0) return NULL;
	dictionary dic = dict();
	while ((token = strsep(&str_copy, "&")))
	{
		char* key;
		char* val;
		if(strlen(token)>0)
		{
			key = strsep(&token,"=");
			if(key && strlen(key)>0)
			{
				val = strsep(&token,"="); 
				dput(dic,key,url_decode(val));
			}
		}
	}
	free(str_copy);
	return dic;
}
/**
* Decode JSON query string to string
*/
char* json_data_decode(int client,int len)
{
	char *query = (char*) malloc((len+1)*sizeof(char));
    for (int i = 0; i < len; i++) {
      recv(client, (query+i), 1, 0);
    }
    query[len]='\0';
    //query = url_decode(query);
    LOG("JSON Query %s\n", query);
    return query;
}

/**
 * read the request as a string line format
 * @param  sock socket
 * @return      a request string
 */
char* read_line(int sock)
{
	char buf[BUFFLEN];
	read_buf(sock,buf,sizeof(buf));
	return strdup(buf);
}
/**
 * Read the socket request in to a buffer or size
 * The data is read until the buffer is full or
 * there are a carrier return character
 * @param  sock socket
 * @param  buf  buffer
 * @param  size size of buffer
 * @return      number of bytes read
 */
int read_buf(int sock, char*buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		if (n > 0)
		{
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}

/**
 * Locate a plugin in the plugin table
 * @param  s plugin name
 * @return   a plugin entry in the plugin table
 */
struct plugin_entry *plugin_lookup(char *s)
{
    struct plugin_entry *np;
    for (np = plugin_table[hash(s, HASHSIZE)]; np != NULL; np = np->next)
        if (strcmp(s, np->pname) == 0)
          return np; /* found */
    return NULL; /* not found */
}

/**
 * Load a plugin to the plugin table
 * Only load when not available in the plugin table
 * @param  name plugin name
 * @return      pointer to the loaded plugin
 */
struct plugin_entry *plugin_load(char *name)
{
    struct plugin_entry *np;
    unsigned hashval;
    if ((np = plugin_lookup(name)) == NULL) { /* not found */
        np = (struct plugin_entry *) malloc(sizeof(*np));
        if (np == NULL || (np->pname = strdup(name)) == NULL)
          return NULL;
        if ((np->handle = plugin_from_file(name)) == NULL)
       		return NULL;
       	hashval = hash(name,HASHSIZE);
        np->next = plugin_table[hashval];
        plugin_table[hashval] = np;
    } else /* already there */
    {
    	LOG("The plugin %s id already loaded\n", name);
    }
   
    return np;
}
/**
 * Find a plugin in a file, and load it in to the plugin table
 * @param  name Name of the plugin
 * @return      
 */
void * plugin_from_file(char* name)
{
	void *lib_handle;
  char* error;
  char* path = __s("%s%s%s",server_config.plugins_dir,name,server_config.plugins_ext);
  void (*fn)(const char*,const char*,const char*,const char*,int);
   lib_handle = dlopen(path, RTLD_LAZY);
   if (!lib_handle) 
   {
      LOG("Cannot load plugin '%s' : '%s'\n",name,dlerror());
      return NULL;
   }
   // set database path
   fn = (void (*)(const char *, const char *, const char *, const char *,int))dlsym(lib_handle, "__init_plugin__");
  if ((error = dlerror()) != NULL)  
  		LOG("Problem when setting data path for %s : %s \n", name,error);
  else
    (*fn)(name,server_config.db_path, server_config.htdocs,server_config.plugins_dir,server_config.port);
	free(path);
   return lib_handle;
}

void unload_plugin(struct plugin_entry* np)
{
	char* error;
	void (*fn)() = NULL;
	// find and execute the exit function
    fn = (void (*)())dlsym(np->handle, "pexit");
 	if ((error = dlerror()) != NULL)  
 	{
     	LOG("Cant not find exit method from %s : %s \n", np->pname,error);
    }
	else
	{
		// execute it
		(*fn)();
	}	
	dlclose(np->handle);
	//free((void *) np->handle);
	free((void *) np->pname);
}
/*
	Unload a plugin by its name
*/
void unload_plugin_by_name(const char* name)
{
	LOG("%s\n","Unloading thing");
	struct plugin_entry *np;
	int hasval = hash(name, HASHSIZE);
	np = plugin_table[hasval];
	if(strcmp(np->pname,name) == 0)
	{
		unload_plugin(np);
		plugin_table[hasval] = np->next;
	}
	else
	{
	    for (np; np != NULL; np = np->next)
	        if (np->next != NULL  && strcmp(name, np->next->pname) == 0)
				break;
		if(np == NULL) return; // the plugin is is not loaded
		unload_plugin(np->next);
		np->next = np->next->next;
	}
}
/**
 * Unload all the plugin loaded on the plugin table
 */
void unload_all_plugin()
{
	LOG("Unload all plugins\n");
	for(int i=0;i<HASHSIZE;i++)
	{
		struct plugin_entry *np;
    	for (np = plugin_table[i]; np != NULL; np = np->next)
    	{
			unload_plugin(np);
        }
        plugin_table[i] = NULL;
	}
	exit(0);
}

/**
 * Execute a plugin based on the http requeset
 * First decode the http request header to find the correct plugin
 * and the correct function on the plugin
 * Second, decode all parameters necessary of the request and pass it 
 * to the callback function.
 * Execute the callback function if sucess
 * @param  client       soket client
 * @param  path         request path
 * @param  method       request method
 * @param  query_string GET query string
 * @return              -1 if failure
 *                      1 if sucess
 */
int execute_plugin(int client, const char *path, const char *method, const char *query_string)
{
	char pname[255];
 	char pfunc[255];
 	void (*fn)(int, const char*,const char*, dictionary);
 	struct plugin_entry *plugin ;
	int plen = strlen(path);
	char * rpath = (char*) malloc((plen+1)*sizeof(char));
	char *error;
	memcpy(rpath,path+1,plen);
	rpath[plen] = '\0';
	trim(rpath,'/');
 	char * delim = strchr(rpath,'/');
 	if(delim == NULL)
 	{
 		strcpy(pname,rpath);
 		strcpy(pfunc,"default");
	} 
	else
	{
		int npos,fpos;
		npos = delim - rpath;
		fpos = strlen(rpath) - npos ;
		memcpy(pname,rpath,npos);
		pname[npos] = '\0';
		memcpy(pfunc,rpath+npos+1,fpos);
		pfunc[fpos-1]='\0';
	}
	LOG("Client %d\n",client );
	LOG("Path : '%s'\n", rpath);
	LOG("Method:%s\n", method);
	LOG("Plugin name '%s'\n",pname);
	LOG("Query path. '%s'\n", pfunc);
	LOG("query :%s\n", query_string);

	//load the plugin
	if((plugin = plugin_lookup(pname)) == NULL)
		if((plugin= plugin_load(pname)) == NULL)
			return -1;
	// load the function
   fn = (void (*)(int, const char *, const char*, dictionary))dlsym(plugin->handle, PLUGIN_HANDLER);
	if ((error = dlerror()) != NULL)  
	{
    	LOG("Problem when finding %s method from %s : %s \n", PLUGIN_HANDLER, pname,error);
    	return -1;
   }
   dictionary dic = decode_request(client,method,query_string);
   	(*fn)(client,method,pfunc,dic);
   free(dic);
   free(rpath);
   return 1;
}

#include "http_server.h"
/**********************************************************************/
/* A request has caused a call to accept() on the server port to
* return.  Process the request appropriately.
* Parameters: the socket connected to the client */
/**********************************************************************/
void accept_request(int client)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[4096];
	char path[1024];
	char* token;
	char *line;
	size_t i, j;
	struct stat st;

	char *query_string = NULL;

	numchars = get_line(client, buf, sizeof(buf));
	i = 0; j = 0;
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		printf("METHOD NOT FOUND %s\n", method);
		// unimplemented
		//while(get_line(client, buf, sizeof(buf)) > 0) printf("%s\n",buf );
		unimplemented(client);
		close(client);
		return;
	}


	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';

	if (strcasecmp(method, "GET") == 0)
	{
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if (*query_string == '?')
		{
			*query_string = '\0';
			query_string++;
		}
	}

	// get the HOST header
	line = read_line(client);
	trim(line, '\n');
	trim(line, '\r');
	token = strsep(&line,":");
	trim(token,' ');
	trim(line,' ');
	if(strcasecmp(token, "HOST"))
	{
		badrequest(client);
		close(client);
		return;
	}

	// perform rule check in domain
	

	sprintf(path, server_config.htdocs);
	strcat(path, url);
	//if (path[strlen(path) - 1] == '/')
	//	strcat(path, "index.html");
	if (stat(path, &st) == -1) {
		if(execute_plugin(client,url,method,query_string) < 0)
			not_found(client);
	}
	else
	{
		if (S_ISDIR(st.st_mode))
			strcat(path, "/index.html");
		// check if the mime is supported
		// if the minme is not supported
		// find an handler plugin to process it
		// if the plugin is not found, forbidden access to the file should be sent
		char* mime_type = mime(path);
		if(strcmp(mime_type,"application/octet-stream") == 0)
		{
			sprintf(buf,"/%s-api%s",ext(path),url);
			LOG("WARNING::::Access octetstream via handler %s\n", buf);
			if(execute_plugin(client,buf,method,query_string) < 0)
				cannot_execute(client);
		}
		else
		{
			ctype(client,mime_type);
			// if the mime is supported, send the file
			serve_file(client, path);
		}		
	}

	close(client);
}


/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
* is named after the UNIX "cat" command, because it might have been
* easier just to do something like pipe, fork, and exec("cat").
* Parameters: the client socket descriptor
*             FILE pointer for the file to cat */
/**********************************************************************/
void catb(int client, FILE* ptr)
{
	unsigned char buffer[BUFFLEN];
	size_t size;
	while(!feof(ptr))
	{
		size = fread(buffer,1,BUFFLEN,ptr);
		__b(client,buffer,size);
		//if(!__b(client,buffer,size)) return;
	}
	//fclose(ptr);
}
void cat(int client, FILE *resource)
{
	char buf[1024];

	//fgets(buf, sizeof(buf), resource);
	while (fgets(buf, sizeof(buf), resource) != NULL)
	{
		send(client, buf, strlen(buf), 0);
		//fgets(buf, sizeof(buf), resource);
	}


}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
* Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
	set_status(client,500,"Internal Server Error");
	__t(client,SERVER_STRING);
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client, "<P>Error prohibited CGI execution.");
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
* on value of errno, which indicates system call errors) and exit the
* program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
	perror(sc);
	exit(1);
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
* carriage return, or a CRLF combination.  Terminates the string read
* with a null character.  If no newline indicator is found before the
* end of the buffer, the string is terminated with a null.  If any of
* the above three line terminators is read, the last character of the
* string will be a linefeed and the string will be terminated with a
* null character.
* Parameters: the socket descriptor
*             the buffer to save the data in
*             the size of the buffer
* Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}


/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
	set_status(client,404,"NOT FOUND");
	__t(client,SERVER_STRING);
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client, "<HTML><TITLE>Not Found</TITLE>");
	__t(client, "<BODY><P>The server could not fulfill");
	__t(client, "your request because the resource specified");
	__t(client, "is unavailable or nonexistent.");
	__t(client, "</BODY></HTML>");
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
* errors to client if they occur.
* Parameters: a pointer to a file structure produced from the socket
*              file descriptor
*             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
	FILE *resource = NULL;
	int numchars = 1;
	char buf[1024];

	buf[0] = 'A'; buf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));

	resource = fopen(filename, "rb");
	if (resource == NULL)
		not_found(client);
	else
	{
		if(is_bin(filename))
			catb(client,resource);
		else
			cat(client, resource);
	}
	fclose(resource);
}

/**********************************************************************/
/* This function starts the process of listening for web connections
* on a specified port.  If the port is 0, then dynamically allocate a
* port and modify the original port variable to reflect the actual
* port.
* Parameters: pointer to variable containing the port to connect on
* Returns: the socket */
/**********************************************************************/
int startup(unsigned *port)
{
	int httpd = 0;
	struct sockaddr_in name;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
		error_die("socket");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		error_die("bind");
	if (*port == 0)  /* if dynamically allocating a port */
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	if (listen(httpd, 5) < 0)
		error_die("listen");
	return(httpd);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
* implemented.
* Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client)
{
	set_status(client,501,"Method Not Implemented");
	__t(client,SERVER_STRING);
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client, "<HTML><HEAD><TITLE>Method Not Implemented");
	__t(client, "</TITLE></HEAD>");
	__t(client, "<BODY><P>HTTP request method not supported.");
	__t(client, "</BODY></HTML>");
}

void badrequest(int client)
{
	set_status(client,400,"Bad Request");
	__t(client,SERVER_STRING);
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client,"The request could not be understood by the server due to malformed syntax.");
}


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
	dictionary xheader = dict();
	char* line;
	char * token;
	if(strcmp(method,"GET") == 0)
	{ 
		// this for check if web socket is enabled
		int ws= 0;
		char* ws_key = NULL;
		while((line = read_line(client)) && strcmp("\r\n",line))
		{
			trim(line, '\n');
			trim(line, '\r');
			token = strsep(&line,":");
			trim(token,' ');
			trim(line,' ');
			dput(xheader,token,line);
			if(token != NULL &&strcasecmp(token,"Cookie") == 0)
			{
				if(!cookie) cookie = decode_cookie(line);
			}
			else if(token != NULL && strcasecmp(token,"Upgrade") == 0)
			{
				// verify that the connection is upgrade to websocket
				trim(line, ' ');
				if(line != NULL && strcasecmp(line,"websocket") == 0)
					ws = 1;
			} else if(token != NULL && strcasecmp(token,"Sec-WebSocket-Key") == 0)
			{
				// get the key from the client
				trim(line, ' ');
				ws_key = strdup(line);
			}
		}
		request = decode_url_request(query);
		if(ws && ws_key != NULL)
		{
			ws_confirm_request(client, ws_key);
			free(ws_key);
			// insert wsocket flag to request
			// plugin should handle this ugraded connection
			// not the server
			if(!request) request = dict();
			dput(request,"__web_socket__","1");
		}
	}
	else
	{
		char* ctype = NULL;
		int clen = -1;
		line = read_line(client);
		while (line && strcmp("\r\n",line))
		{
			//printf("%s\n",line);
			trim(line, '\n');
			trim(line, '\r');
			token = strsep(&line,":");
			trim(token,' ');
			trim(line, ' ');
			dput(xheader,token,line);
			if(token != NULL &&strcasecmp(token,"Content-Type") == 0)
			{
				ctype = strsep(&line,":");
				trim(ctype,' ');
			} else if(token != NULL &&strcasecmp(token,"Content-Length") == 0)
			{
				token = strsep(&line,":");
				trim(token,' ');
				clen = atoi(token);
			}
			else if(token != NULL &&strcasecmp(token,"Cookie") == 0)
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
		LOG("ContentType %s\n", ctype);
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
	if(!request)
			request = dict();
		
	dput(request,"cookie",cookie);
	dput(request,"__xheader__",xheader);
	return request;
}
void __px(const char* data,int size)
{
	for (int i = 0; i < size; ++i)
			printf(" %02x", data[i]);
			
	printf("\n");
}
/**
* Send header to the client to confirm 
* that the websocket is accepted by
* our server
*/
void ws_confirm_request(int client, const char* key)
{
	char buf[256];
	char rkey[128];
	char sha_d[20];
	char base64[64];
	strcpy(rkey,key);
	strcat(rkey,WS_MAGIC_STRING);
	//printf("RESPONDKEY '%s'\n", rkey);
	SHA1_CTX context;
    SHA1_Init(&context);
    SHA1_Update(&context, rkey, strlen(rkey));
    SHA1_Final(&context, sha_d); 
	Base64encode(base64, sha_d, 20);
	//printf("Base 64 '%s'\n", base64);
	// send accept to client
	sprintf(buf, "HTTP/1.1 101 Switching Protocols\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Upgrade: websocket\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Connection: Upgrade\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Sec-WebSocket-Accept: %s\r\n",base64);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	
	LOG("%s\n", "Websocket is now enabled for plugin");
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
	trim(cpstr,' ');
	trim(cpstr,'\n');
	trim(cpstr,'\r');
	//printf("FUCKIT %s\n",cpstr );
	dictionary dic = NULL;
	while((token = strsep(&cpstr,";")))
	{
		trim(token,' ');
		token1 = strsep(&token,"=");
		if(token1)
		{
			if(dic == NULL)
				dic = dict();
			//LOG("Found cookie : %s = %s\n",token1,token);
			dput(dic,token1,strdup(token));
		}
	}
		//}
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

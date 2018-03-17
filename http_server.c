#include "http_server.h"
static pthread_mutex_t server_mux = PTHREAD_MUTEX_INITIALIZER;
/**********************************************************************/
/* A request has caused a call to accept() on the server port to
* return.  Process the request appropriately.
* Parameters: the socket connected to the client */
/**********************************************************************/
void accept_request(void* client)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[4096]; 
	char path[1024];
	char* token;
	char *line;
	char* oldurl = NULL;
	char* tmp = NULL;
	dictionary rq = NULL;
	size_t i, j;
	struct stat st;

	//char *query_string = NULL;
	//LOG("SOCK IS %d\n", ((antd_client_t*)client)->sock);
	numchars = read_buf(client, buf, sizeof(buf));
	if(numchars <= 0)
	{
		unknow(client); 
		goto end;
	}
	i = 0; j = 0;
	while (j < numchars && !ISspace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		LOG("METHOD NOT FOUND %s\n", method);
		// unimplemented
		//while(get_line(client, buf, sizeof(buf)) > 0) printf("%s\n",buf );
		unimplemented(client);
		antd_close(client);
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


	
	oldurl = strdup(url);
	tmp = strchr(oldurl,'?');
	if(tmp)
		*tmp = '\0';

	rq = decode_request(client, method, url);
	if(rq == NULL)
	{
		badrequest(client);
		goto end;
	}	
	sprintf(path, server_config.htdocs);
	strcat(path, url);
	LOG("Path is : %s \n", path);
	//if (path[strlen(path) - 1] == '/')
	//	strcat(path, "index.html");
	if (stat(path, &st) == -1) {
		if(execute_plugin(client,oldurl,method,rq) < 0)
			not_found(client);
	}
	else
	{
		if (S_ISDIR(st.st_mode))
		{
			int l = strlen(path);
			int ul = strlen(url);
			strcat(path, "/index.html");
			if(stat(path, &st) == -1)
			{
				association it;
				for_each_assoc(it, server_config.handlers)
				{
					path[l] = '\0';
					url[ul] = '\0';
					strcat(url,"/index.");
					strcat(path, "/index.");
					strcat(url,it->key);
					strcat(path, it->key);
					if(stat(path, &st) == 0)
					{
						l = -1;
						i = HASHSIZE;
						break;
					}
				}
				if(l!= -1)
				{
					not_found(client);
					goto end;
				}
			}
		}
		// check if the mime is supported
		// if the mime is not supported
		// find an handler plugin to process it
		// if the plugin is not found, forbidden access to the file should be sent
		char* mime_type = mime(path);
		if(strcmp(mime_type,"application/octet-stream") == 0)
		{
			char * ex = ext(path);
			char* h = dvalue(server_config.handlers,ex);
			if(ex) free(ex);
			if(h)
			{
				sprintf(buf,"/%s%s",h,url);
				LOG("WARNING::::Access octetstream via handler %s\n", buf);
				if(execute_plugin(client,buf,method,rq) < 0)
					cannot_execute(client);
			}
			else
				unknow(client);
			
		}
		else
		{
			ctype(client,mime_type);
			// if the mime is supported, send the file						
			serve_file(client, path);
			//response(client,"this is the file");
		}		
	}
end:
	if(oldurl) free(oldurl);
	if(rq)
	{
		dictionary subdict;
		subdict = (dictionary)dvalue(rq, "__xheader__");
		if(subdict)
		{
			freedict(subdict);
			dput(rq, "__xheader__", NULL);
		}

		subdict = (dictionary)dvalue(rq, "cookie");
		if(subdict)
		{
			freedict(subdict);
			dput(rq, "cookie", NULL);
		}
		freedict(rq);
	}
	antd_close(client);
}

int rule_check(const char*k, const char* v, const char* host, const char* _url, const char* _query, char* buf)
{
	// first perfom rule check on host, if not success, perform on url
	regmatch_t key_matches[10];
	regmatch_t val_matches[2];
	char* query = strdup(_query);
	char* url = strdup(_url);
	int ret;
	char* target;
	char* tmp, rep[10];
	int idx = 0;
	memset(rep,0,10);
	// 1 group
	if(!host ||  !(ret = regex_match(k,host, 10, key_matches)) )
	{
		target = url;
		ret = regex_match(k,url, 10, key_matches);
	}
	else
		target = host;

	if(!ret) 
	{
		free(url);
		free(query);
		return 0;
	}
	tmp = (char*) v;
	char * search = "<([a-zA-Z0-9]+)>";
	//printf("match again %s\n",tmp);
	while((ret = regex_match( search,tmp, 2, val_matches)))
	{
		memcpy(buf + idx, tmp, val_matches[1].rm_so - 1);
		idx += val_matches[1].rm_so - 1;
		memcpy(rep, tmp + val_matches[1].rm_so, val_matches[1].rm_eo - val_matches[1].rm_so);
		if(strcasecmp(rep,"url") == 0)
		{
			memcpy(buf+idx, url, strlen(url));
			idx += strlen(url);
		} else if(strcasecmp(rep,"query") == 0)
		{
			memcpy(buf+idx, query, strlen(query));
			idx += strlen(query);
		} else if(match_int(rep))
		{
			int i = atoi(rep);
			memcpy(buf+idx, target + key_matches[i].rm_so, key_matches[i].rm_eo - key_matches[i].rm_so);
			idx += key_matches[i].rm_eo - key_matches[i].rm_so;
		} else { // just keep it
			memcpy(buf+idx, tmp + val_matches[1].rm_so-1, val_matches[1].rm_eo + 2 - val_matches[1].rm_so);
			idx+= val_matches[1].rm_eo + 2 - val_matches[1].rm_so;
		}
		tmp += val_matches[1].rm_eo + 1;
		//break;
	}
	// now modify the match 2 group
	if(idx > 0)
	{
		if(tmp)
		{
			// copy the remainning of tmp
			memcpy(buf+idx, tmp, strlen(tmp));
			idx += strlen(tmp);
		}
		buf[idx] = '\0';
	}
	free(url);
	free(query);
	return 1;
}
/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
* is named after the UNIX "cat" command, because it might have been
* easier just to do something like pipe, fork, and exec("cat").
* Parameters: the client socket descriptor
*             FILE pointer for the file to cat */
/**********************************************************************/
void catb(void* client, FILE* ptr)
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
void cat(void* client, FILE *resource)
{
	char buf[1024];
	//fgets(buf, sizeof(buf), resource);
	while (fgets(buf, sizeof(buf), resource) != NULL)
	{
		antd_send(client, buf, strlen(buf));
		//fgets(buf, sizeof(buf), resource);
	}


}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
* Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(void* client)
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
//This function is deprecate
/*int get_line(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				
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
}*/


/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(void* client)
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
void serve_file(void* client, const char *filename)
{
	LOG("Serve file: %s\n", filename);
	FILE *resource = NULL;
	int numchars = 1;
	//char buf[1024];

	//buf[0] = 'A'; buf[1] = '\0';
	//while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
	//	numchars = get_line(client, buf, sizeof(buf));

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
	printf("back log is %d\n", server_config.backlog);
	if (listen(httpd, server_config.backlog) < 0)
		error_die("listen");
	return(httpd);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
* implemented.
* Parameter: the client socket */
/**********************************************************************/
void unimplemented(void* client)
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

void badrequest(void* client)
{
	set_status(client,400,"Bad Request");
	__t(client,SERVER_STRING);
	__t(client,"Content-Type: text/html");
	response(client,"");
	__t(client,"The request could not be understood by the server due to malformed syntax.");
}

char* apply_rules(const char* host, char*url)
{
	association it;
	// rule check
	char* query_string = url;
	while ((*query_string != '?') && (*query_string != '\0'))
		query_string++;
	if (*query_string == '?')
	{
		*query_string = '\0';
		query_string++;
	}
	//char* oldurl = strdup(url);
	int size = list_size(server_config.rules);
	for(int i = 0; i < size; i+= 2)
	{
		char *k, *v;
		k  = list_at(server_config.rules, i)->value.s;
		v  = list_at(server_config.rules, i+1)->value.s;
		// 1 group
		if(rule_check(k, v,host, url, query_string, url)){
			query_string = url;
		
			while ((*query_string != '?') && (*query_string != '\0'))
				query_string++;
			if (*query_string == '?')
			{
				*query_string = '\0';
				query_string++;
			}
		}
	}
	
	return strdup(query_string);
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
dictionary decode_request(void* client,const char* method, char* url)
{
	dictionary request = NULL;
	dictionary cookie = NULL;
	dictionary xheader = dict();
	char* line;
	char * token;
	char* query = NULL;
	char* ctype = NULL;
	char* host = NULL;
	int clen = -1;

	// first real all header
// this for check if web socket is enabled
	int ws= 0;
	char* ws_key = NULL;
	char buf[BUFFLEN];

	//while((line = read_line(client)) && strcmp("\r\n",line))
	while((read_buf(client,buf,sizeof(buf))) && strcmp("\r\n",buf))
	{
		line = buf;
		trim(line, '\n');
		trim(line, '\r');
		token = strsep(&line,":");
		trim(token,' ');
		trim(line,' ');
		dput(xheader,token,strdup(line));
		if(token != NULL &&strcasecmp(token,"Cookie") == 0)
		{
			if(!cookie) cookie = decode_cookie(line);
		}
		else if(token != NULL &&strcasecmp(token,"Content-Type") == 0)
		{
			ctype = strdup(line); //strsep(&line,":");
			trim(ctype,' ');
		} else if(token != NULL &&strcasecmp(token,"Content-Length") == 0)
		{
			token = line; //strsep(&line,":");
			trim(token,' ');
			clen = atoi(token);
		}
		else if(token != NULL && strcasecmp(token,"Upgrade") == 0)
		{
			// verify that the connection is upgrade to websocket
			trim(line, ' ');
			if(line != NULL && strcasecmp(line,"websocket") == 0)
				ws = 1;
		}else if(token != NULL && strcasecmp(token,"Host") == 0)
		{
			host = strdup(line);
		}
			else if(token != NULL && strcasecmp(token,"Sec-WebSocket-Key") == 0)
		{
			// get the key from the client
			trim(line, ' ');
			ws_key = strdup(line);
		}
	}
	//if(line) free(line);
	query = apply_rules(host, url);
	if(host) free(host);
	if(strcmp(method,"GET") == 0)
	{
		if(ctype) free(ctype); 
		if(query)
		{
			LOG("Query: %s\n", query);
			request = decode_url_request(query);
			free(query);
		}
		if(ws && ws_key != NULL)
		{
			ws_confirm_request(client, ws_key);
			free(ws_key);
			// insert wsocket flag to request
			// plugin should handle this ugraded connection
			// not the server
			if(!request) request = dict();
			dput(request,"__web_socket__",strdup("1"));
		}
	}
	else
	{
		if(query)
			free(query);
		if(ws_key)
			free(ws_key);
		if(ctype == NULL || clen == -1)
		{
			LOG("Bad request\n");
			if(ctype) free(ctype);
			if(cookie) freedict(cookie);
			free(xheader);
			return NULL;
		}
		LOG("ContentType %s\n", ctype);
		// decide what to do with the data
		if(strstr(ctype,FORM_URL_ENCODE) > 0)
		{
			request = decode_url_request(post_data_decode(client,clen));
		} else if(strstr(ctype,FORM_MULTI_PART)> 0)
		{
			//printf("Multi part form : %s\n", ctype);
			request = decode_multi_part_request(client,ctype);
		} 
		else
		{
			char* query = post_data_decode(client,clen);
			char* key = strstr(ctype,"/");
			if(key)
				key++;
			else
				key = ctype;
			request = dict();
			dput(request,key, strdup(query));
			free(query);
		}
		
	}
	if(ctype) free(ctype);
	//if(cookie->key == NULL) {free(cookie);cookie= NULL;}
	if(!request)
			request = dict();
	if(cookie)
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
void ws_confirm_request(void* client, const char* key)
{
	char buf[256];
	char rkey[128];
	char sha_d[20];
	char base64[64];
	strcpy(rkey,key);
	strcat(rkey,WS_MAGIC_STRING);
	//printf("RESPONDKEY '%s'\n", rkey);
#ifdef USE_OPENSSL
	SHA_CTX context;
#else
	SHA1_CTX context;
#endif
	
    SHA1_Init(&context);
    SHA1_Update(&context, rkey, strlen(rkey));
    SHA1_Final(sha_d, &context); 
	Base64encode(base64, sha_d, 20);
	//printf("Base 64 '%s'\n", base64);
	// send accept to client
	sprintf(buf, "HTTP/1.1 101 Switching Protocols\r\n");
	antd_send(client, buf, strlen(buf));
	sprintf(buf, "Upgrade: websocket\r\n");
	antd_send(client, buf, strlen(buf));
	sprintf(buf, "Connection: Upgrade\r\n");
	antd_send(client, buf, strlen(buf));
	sprintf(buf, "Sec-WebSocket-Accept: %s\r\n",base64);
	antd_send(client, buf, strlen(buf));
	sprintf(buf, "\r\n");
	antd_send(client, buf, strlen(buf));
	
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
	char *orgcpy = cpstr;
	trim(cpstr,' ');
	trim(cpstr,'\n');
	trim(cpstr,'\r');
	
	dictionary dic = NULL;
	while((token = strsep(&cpstr,";")))
	{
		trim(token,' ');
		token1 = strsep(&token,"=");
		if(token1 && token && strlen(token) > 0)
		{
			if(dic == NULL)
				dic = dict();
			dput(dic,token1,strdup(token));
		}
	}
		//}
	free(orgcpy);
	return dic;
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
dictionary decode_multi_part_request(void* client,const char* ctype)
{
	char * boundary;
	char * boundend;
	char * line;
	char * orgline;
	char * str_copy = strdup(ctype);
	char* orgcpy = str_copy;
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
		while((line = read_line(client))&&strstr(line,boundary) <= 0)
		{
			if(line) free(line);
		}
		// loop through each part separated by the boundary
		while(line && strstr(line,boundary) > 0){
			free(line);
			// search for content disposition:
			while((line = read_line(client)) &&
					strstr(line,"Content-Disposition:") <= 0)
			{
				free(line);
				line = NULL;
			}
			if(!line || strstr(line,"Content-Disposition:") <= 0)
			{
				if(line)
					free(line);
				free(orgcpy);
				free(boundend);
				return NULL;
			}
			orgline = line;
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
							part_name = strdup(valtoken);
						} else if(strcmp(keytoken,"filename") == 0)
						{
							part_file = strdup(valtoken);
						}
					}
				}
			}
			free(orgline);
			// get the binary data
			if(part_name != NULL)
			{
				// go to the beginer of data bock
				while((line = read_line(client)) && strcmp(line,"\r\n") != 0)
				{
					free(line);
				}
				if(line) free(line);
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
					while((line = read_line(client)) && strstr(line,boundary) <= 0)
					{
						free(line);
					}
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
						dput(dic,field, strdup(part_file));
						free(field);
						field = __s("%s.tmp",part_name);
						dput(dic,field,strdup(file_path));
						free(field);
						field = __s("%s.size",part_name);
						dput(dic,field,__s("%d",totalsize));
						free(field);
						field = __s("%s.ext",part_name);
						dput(dic,field,ext(part_file));
						free(field);

					}
					else
					{
						LOG("Cannot wirte file to :%s\n", file_path );
					}
					free(file_path);
					free(part_file);
				}
				free(part_name);
			}
			//printf("[Lines]:%s\n",line);
			// check if end of request
			if(line&&strstr(line,boundend)>0)
			{
				LOG("End request %s\n", boundend);
				break;
			}
		}
		free(boundend);
	}
	free(orgcpy);
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
	if(query == NULL) return NULL;
	//str_copy = ;
	char* token;
	if(strlen(query) == 0) return NULL;
	char* str_copy = strdup(query);
	char* org_copy = str_copy;
	dictionary dic = dict();
	while ((token = strsep(&str_copy, "&")))
	{
		char* key;
		char* val = NULL;
		if(strlen(token)>0)
		{
			key = strsep(&token,"=");
			if(key && strlen(key)>0)
			{
				val = strsep(&token,"=");
				if(!val)
					val = "";
				dput(dic,key,url_decode(val));
			}
		}
	}
	free(org_copy);
	return dic;
}
/**
* Decode post query string to string
*/
char* post_data_decode(void* client,int len)
{
	char *query = (char*) malloc((len+1)*sizeof(char));
    for (int i = 0; i < len; i++) {
      antd_recv(client, (query+i), 1);
    }
    query[len]='\0';
    //query = url_decode(query);
    //LOG("JSON Query %s\n", query);
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
int execute_plugin(void* client, const char *path, const char *method, dictionary dic)
{
	char pname[255];
 	char pfunc[255];
 	void (*fn)(void*, const char*,const char*, dictionary);
 	struct plugin_entry *plugin ;
	int plen = strlen(path);
	char * rpath = (char*) malloc((plen+1)*sizeof(char));
	char* orgs = rpath;
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
	LOG("Client %d\n",((antd_client_t*)client)->sock );
	LOG("Path : '%s'\n", rpath);
	LOG("Method:%s\n", method);
	LOG("Plugin name '%s'\n",pname);
	LOG("Query path. '%s'\n", pfunc);
	//LOG("query :%s\n", query_string);

	//load the plugin
	if((plugin = plugin_lookup(pname)) == NULL)
	{
		pthread_mutex_lock(&server_mux);
		plugin= plugin_load(pname);
		pthread_mutex_unlock(&server_mux);
		if( plugin == NULL)
		{
			if(orgs) free(orgs);
			return -1;
		}
	}
	// load the function
   fn = (void (*)(void*, const char *, const char*, dictionary))dlsym(plugin->handle, PLUGIN_HANDLER);
	if ((error = dlerror()) != NULL)  
	{
		if(orgs) free(orgs);
    	LOG("Problem when finding %s method from %s : %s \n", PLUGIN_HANDLER, pname,error);
    	return -1;
   }
   //dictionary dic = decode_request(client,method,query_string);
   	(*fn)(client,method,pfunc,dic);
   //free(dic);
   free(orgs);
   return 1;
}

 #ifdef USE_OPENSSL
 int usessl()
 {
	 return server_config.usessl;
 }
 #endif

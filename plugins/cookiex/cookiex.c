#include "../plugin.h"

void init();
call __init__ = init;

void init()
{
	printf("Finish init\n");
}

void execute(int client,const char* method,dictionary rq)
{

	/**
	 * Set cookie to the browser
	 */
	char* c = server_time();
	dictionary d = dict();
	dput(d,"test",c);
	dput(d,"test1","This is another cookie");
	set_cookie(client,d);

	LOG("%s",c);
	__t(client,"<h1>Set the cookie</h1>");
	freedict(d);
}

void get(int client,const char* method,dictionary rq)
{

	
}
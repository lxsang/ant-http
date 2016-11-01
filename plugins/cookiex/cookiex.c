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
	set_cookie(client,"text/html; charset=utf-8",d);

	LOG("%s",c);
	__t(client,"<h1>Set the cookie</h1>");
	freedict(d);
}

void get(int client,const char* method,dictionary rq)
{
	html(client);
	if(rq)
	{
		dictionary ck = (dictionary)dvalue(rq,"cookie");
		if(ck)
		{
			association as;
			for_each_assoc(as, ck)
			{
				__t(client,"%s -> %s <br/>",as->key, as->value);
			}
		}
		else
			__t(client,"noo cookie");
	}
	else
		__t(client,"no request");
	
}
void handler(int client, const char* method, const char* rqpth, dictionary rq)
{
	if(EQU(rqpth,"default"))
	{
		execute(client,method,rq);
	}
	else if(EQU(rqpth,"get"))
	{
		get(client,method,rq);
	}
	else
	{
		unknow(client);
	}
}
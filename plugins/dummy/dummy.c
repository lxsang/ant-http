#include "../plugin.h"

void init();
call __init__ = init;
sqldb db;

void init()
{
	db = getdb();
	if(db == NULL)
		printf("Unable to get database %s \n", __plugin__.name);
	// create table 
	char* sql = "CREATE TABLE dummy(id INTEGER PRIMARY KEY, color TEXT);";
	if(hastable(db,"dummy") == 0)
		dbquery(db,sql,NULL);
	printf("Finish init\n");
}
void pexit()
{
	LOG("%s\n","Plugin DUMMY is exited");
}
void execute(int client,const char* method,dictionary rq)
{

	char * question;

	question = dvalue(rq,"color");
	dbfield field = __field();

	if(question != NULL)
	{
		add_field(&field,"color",question);
		dbinsert(db,"dummy",field);
	}
	free(field);
	json(client);
	
	dbrecord records = dball(db,"dummy");
	
	__t(client,"{ \"records\":[");
	int cnt = 0;
	 for(dbrecord r = records;r != NULL; r=r->next)
	 {
	 	
	 	if(cnt!=0) __t(client,",");
	 	dbfield row = r-> fields;
	 	__t(client,"{\"id\":%s,\"color\":\"%s\"}",
	 		value_of(row,"id"),
	 		value_of(row,"color"));
	 	free(row);
	 	cnt++;
	 } 
	__t(client, "],\"total\":%d}",records->idx);
	free(records);
	//__t(client, query);
}

// delete record
void delete(int client,const char* method,dictionary rq)
{
	char* id = dvalue(rq,"id");
	html(client);
	if(id==NULL)
	{
		__t(client,"<p>Invalid delere request <a href='%s'>Back</a></p>", route(NULL));
	}
	else
	{
		if(dbdelete(db,"dummy","id=%s",id))
		{
			redirect(client,route(NULL));
		} else
		{
			__t(client,"<p>Invalid delere request <a href='%s'>Back</a></p>", route(NULL));
		}
	}

}
void update(int client,const char* method,dictionary rq)
{
	char * id;
	html(client);
	if(IS_GET(method))
	{
		id = dvalue(rq,"id");
		if(id!=NULL)
		{
			dbrecord rc = dbselect(db,"dummy","id=%s",id);
			if(rc->idx > 0)
			{
				__t(client,"<html><body><FORM ACTION=\"%s\" METHOD=\"POST\">\
					Enter a color: \
					<textarea rows=\"4\" cols=\"50\" NAME=\"color\">%s</textarea>\
					<INPUT TYPE=\"hidden\" NAME=\"id\" Value=\"%s\">\
					<INPUT id=\"button\" TYPE=\"submit\">\
					</FORM> </body></html>",route("update"),value_of(rc->fields,"color"),id);
				free(rc);
				return;
			}
		} 
		__t(client,"<p>No data found. <a href='%s'>Back</a></p>",route(NULL));
		
	}
	else
	{
		char* color;
		id = dvalue(rq,"id");
		color = dvalue(rq,"color");
		dbfield fields = __field();
		add_field(&fields,"color",color);
		if(dbupdate(db,"dummy",fields,"id=%s",id))
		{
			redirect(client,route(NULL));
		}
		else
		{
			__t(client,"<p>Invalid Update request <a href='%s'>Back</a></p>", route(NULL));
		}
		free(fields);
	}
}


void jsonex(int client,const char* method,dictionary rq)
{	
	//json(client);
	//__t(client,"{name:\"%s\", age:%d}","Sang",30);
	jpeg(client);
	__f(client,htdocs("images/ex.jpg"));
}

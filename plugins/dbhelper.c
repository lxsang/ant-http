#include "dbhelper.h"

sqlite3 * database(const char* file)
{
	sqlite3* db;
	int rc = sqlite3_open(file,&db);
	if (rc != SQLITE_OK) {    
        LOG( "Cannot open database: %s %s\n",file, sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}
void dbclose(sqlite3* db)
{
	sqlite3_close(db);
}
int dbquery(sqlite3* db,const char* sql, int (*call_back)())
{
	char *err_msg = 0;
	sqlite3_mutex_enter(sqlite3_db_mutex(db));
	int rc = sqlite3_exec(db,sql,call_back,0,&err_msg);
	sqlite3_mutex_leave(sqlite3_db_mutex(db));
	if(rc != SQLITE_OK)
	{
		LOG("Cannot query : '%s' [%s]\n", sql,err_msg);
		sqlite3_free(err_msg);
		return 0;
	}
	return 1;
}
dbfield __field()
{
	dbfield ret = malloc(sizeof *ret);
	ret->name = NULL;
	ret->value = NULL;
	ret->next = NULL;
	return ret;
}
dbrecord __record()
{
	dbrecord ret = malloc(sizeof *ret);
	ret->fields = NULL;
	ret->idx = 0;
	ret->next = NULL;
	return ret;
}

char* __name_list(const dbfield f)
{
	char* sql = f->name;
	for(dbfield p=f->next;p!=NULL;p=p->next)
	{
		sql = __s("%s,%s",sql,p->name);
	}
	return sql;
}
char* __value_list(const dbfield f)
{
	char* sql = __s("'%s'",f->value);
	for(dbfield p=f->next;p!=NULL;p=p->next)
	{
		sql = __s("%s,'%s'",sql,p->value);
	}
	return sql;
}
char* __name_value_list(const dbfield f)
{
	char* sql = __s("%s='%s'", f->name,f->value);
	for(dbfield p=f->next;p!=NULL;p=p->next)
	{
		sql = __s("%s,%s='%s'",sql, p->name,f->value);
	}
	return sql;
}

void add_record(dbrecord* r,dbfield f)
{
	dbrecord new_r = malloc(sizeof *new_r);
	new_r->fields = f;
	new_r->idx =  1;
	new_r->next = NULL;
	if((*r)->idx == 0)
		*r = new_r;
	else
	{
		dbrecord* temp;
		for(temp = r;(*temp)->next!=NULL;temp=&((*temp)->next))
		{
			(*temp)->idx++;			
		}
		(*temp)->next = new_r;
	}
	//new_r->next = *r;
	//*r = new_r;
}

void add_field(dbfield* field,const char* name, const char* value)
{
	dbfield new_field = malloc(sizeof *new_field);
	new_field->name = strdup(name);
	new_field->value = strdup(value);
	new_field->next = *field;
	*field = new_field;
}
char* value_of(const dbfield f,const char* key)
{
	for(dbfield t = f; t != NULL; t=t->next)
		if(strcmp(t->name,key)==0)
			return t->value;
	return NULL;
}
int dbinsert(sqlite3* db,const char* table, const dbfield fields)
{
	char* sqlprefix = "INSERT INTO %s (%s) VALUES (%s)";
	char* name_list = __name_list(fields);
	char* value_list = __value_list(fields);
	char* sql = __s(sqlprefix,table,name_list,value_list);
	int ret = dbquery(db,sql,NULL);
	free(name_list);
	free(value_list);
	free(sql);

	if(ret == 0)
		return -1;
	return sqlite3_last_insert_rowid(db);
}
dbrecord dball(sqlite3* db,const char* table)
{
	return dbselect(db,table,"1=%d",1);
}
dbrecord dbselect(sqlite3* db, const char* table,const char* fstring,...)
{
	char* sql;
	char* prefix = "SELECT * FROM %s WHERE (%s)";
	char* cond;
	va_list arguments; 
	int dlen;
	sqlite3_stmt *statement;
	dbrecord records = __record();

	va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments) + 1;
    va_end(arguments);
    cond = (char*) malloc(dlen*sizeof(char));
    va_start(arguments, fstring);
    vsnprintf(cond, dlen, fstring, arguments);
    va_end(arguments);
    sql = __s(prefix,table,cond);

    if(sqlite3_prepare_v2(db, sql, -1, &statement, 0) == SQLITE_OK)
    {
        int cols = sqlite3_column_count(statement);
        int result = 0;
        while((result = sqlite3_step(statement)) == SQLITE_ROW)
        {
            dbfield fields = __field();
            for(int col = 0; col < cols; col++)
            {
                char *value = (char*)sqlite3_column_text(statement, col);
               	char *name = (char*)sqlite3_column_name(statement, col);
                add_field(&fields,name,(value!=0)?value:"");
            }
            add_record(&records,fields);
        }
        sqlite3_finalize(statement);
    }
    else
    {
    	LOG("Can not select:%s [%s]\n",sql,sqlite3_errmsg(db));
    }

    free(cond);
    free(sql);
    return records;
}
int hastable(sqlite3* db,const char* table)
{
	char * prefix = "select * from sqlite_master where type='table' and name='%s'";
	char* sql = __s(prefix,table);
	int ret = dbquery(db,sql,NULL);
	free(sql);
	return ~ret;
}
int dbupdate(sqlite3* db,const char* table,const dbfield field,const char* fstring,...)
{
	char* sql;
	char* prefix = "UPDATE %s SET %s WHERE (%s)";
	char* cond;
	char* list;
	va_list arguments; 
	int dlen;
	va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments) + 1;
    va_end(arguments);
    cond = (char*) malloc(dlen*sizeof(char));
    va_start(arguments, fstring);
    vsnprintf(cond, dlen, fstring, arguments);
    va_end(arguments);

    list = __name_value_list(field);
    sql =  __s(prefix,table,list,cond);
    int ret = dbquery(db,sql,NULL);
    free(cond);
    free(list);
    free(sql);
    return ret;
}

int dbdelete(sqlite3* db,const char* table,const char* fstring,...)
{
	char* sql;
	char* prefix = "DELETE  FROM %s WHERE (%s)";
	char* cond;
	va_list arguments; 
	int dlen;
	va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments) + 1;
    va_end(arguments);
    cond = (char*) malloc(dlen*sizeof(char));
    va_start(arguments, fstring);
    vsnprintf(cond, dlen, fstring, arguments);
    va_end(arguments);
    sql = __s(prefix,table,cond);
    int ret = dbquery(db,sql,NULL);
    free(cond);
    free(sql);
    return ret;
}	
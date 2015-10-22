#include <sqlite3.h>
#include "utils.h"

sqlite3 * database(const char*);
typedef struct _dbfield
{
	char* name;
	char* value;
	struct _dbfield* next;
} *dbfield ;

typedef struct _dbrecord
{
	dbfield fields;
	int idx;
	struct _dbrecord* next;
} * dbrecord;

int dbquery(sqlite3*,const char*, int (*)());
int dbinsert(sqlite3*,const char*,const dbfield);
int hastable(sqlite3*,const char*);
int dbupdate(sqlite3*,const char*,const dbfield,const char*,...);
dbrecord dbselect(sqlite3*, const char*,const char*,...);
dbrecord dball(sqlite3*,const char*);
int dbdelete(sqlite3*,const char*,const char*,...);
dbfield __field();
dbrecord __record();
char* __name_list(const dbfield);
char* __value_list(const dbfield);
char* __name_value_list(const dbfield);
char* value_of(const dbfield,const char*);
void add_field(dbfield*,const char*, const char*);
void add_record(dbrecord*,dbfield);
void dbclose(sqlite3*);
#include "utils.h"

#define T_STR	0
#define T_INT	1
#define T_FLOAT	2
#define T_FIELD	3
#define T_UNDEF 4


typedef struct
{
	char* 	name;
	int 	type;
	union
	{
		int 	i;
		float 	f;
		char* 	s;
	} value;
} field;

typedef struct 
{
	int  type;
	union
	{
		int 		 	i;
		float 		 	f;
		char* 		 	s;
		field 		 	fi;
	} value;
} Atom;

typedef struct _item
{
	Atom e;
	struct _item* next;
} *list ;


list __list();
list lput(list*,Atom);
list last(list);
int lremove(list,int);
int size(list);
list lat(list,int);
Atom lvalueAt(list, int);
Atom atom(int type);
char* a2s(Atom);
char* f2s(field);
char* fv2s(field);
list split(const char*, const char*);






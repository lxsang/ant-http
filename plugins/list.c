#include "list.h"

Atom __atom()
{
	Atom ret = (Atom){.type = T_UNDEF};
	return ret;
}
Atom atom(int type)
{
	Atom ret = (Atom){.type = type};
	return ret;	
}
list __list()
{
	list ret = malloc(sizeof *ret);
	ret->e.type = T_UNDEF;
	ret->next = NULL;
	return ret;
}

list last(list l)
{
	list p = l;
	while( p && p->next != NULL)
		p = p->next;
	return p;
}

int size(list l)
{
	int i = 0;
	list np = l;
	while(np)
	{
		np = np->next;
		i++;
	}
	return i;
}
Atom lvalueAt(list l,int idx)
{
	list np = lat(l,idx);
	if(np == NULL) return __atom();
	return np->e;
}

list lat(list l,int idx)
{
	int i = 0;
	list np = l;
	while(np && np->next != NULL)
	{
		if(i== idx)
			return np;
		np = np->next;
		i++;
	}
	return NULL;
}
list lput(list* l, Atom a)
{
	list new_item = malloc(sizeof *new_item);
	new_item->e = a;
	new_item->next = NULL;
	if((*l) == NULL || (*l)->e.type == T_UNDEF) 
	{
		(*l) = new_item;
		return (*l);
	}
	list np = last(*l);
	np->next = new_item;
	return np->next;
}
int lremove(list l,int idx)
{
	if(l == NULL) return 0;

	if(idx == 0)
	{
		l = l->next;
		return 1;
	}
	list np = lat(l,idx-1);
	if(np == NULL) return 0;
	if(np->next == NULL) return 1;
	np->next = np->next->next;
	return 1;
}
char* to_string(Atom a)
{
	switch(a.type)
	{
		case T_STR:
			return a.value.s;
			break;
		case T_INT:
			return __s("%d",a.value.i);
			break;
		case T_FLOAT:
			return __s("%f",a.value.f);
			break;
		case T_FIELD:
			return f2s(a.value.fi);
			break;
		default:
			return "";
	}
}
 char* fv2s(field f)
{
	switch(f.type)
	{
		case T_STR:
			return __s("%s",f.value.s);
			break;
		case T_INT:
			return __s("%d",f.value.i);
			break;
		case T_FLOAT:
			return __s("%f",f.value.f);
			break;
		default:
			return "";
	}
}
char* f2s(field f)
{
	switch(f.type)
	{
		case T_STR:
			return __s("%s='%s'",f.name,f.value.s);
			break;
		case T_INT:
			return __s("%s=%d",f.name,f.value.i);
			break;
		case T_FLOAT:
			return __s("%s=%f",f.name,f.value.f);
			break;
		default:
			return "";
	}
}

list split(const char* str, const char* delim)
{
	if(str == NULL || delim == NULL) return NULL;
	char* str_cpy = strdup(str);
	char* token;
	Atom v = atom(T_STR);
	list l = __list();
	while((token = strsep(&str_cpy,delim)))
	{
		if(strlen(token) > 0) 
		{
			v = (Atom){.value.s = strdup(token)};
			lput(&l,v);
		}
	}
	free(str_cpy);
	if(l->e.type== T_UNDEF)
		return NULL;

	return l;
}

/*
The MIT License (MIT)

Copyright (c) 2015 LE Xuan Sang xsang.le@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "list.h"

list list_init()
{
	list ret = (list)malloc(sizeof *ret);
	ret->type = RPC_TYPE_NIL;
	ret->next = NULL;
	return ret;
}
void list_put(list* l, item it)
{
	if(*l == NULL || (*l)->type == RPC_TYPE_NIL)
	{
		*l = it;
		return ;
	}	
	item np = list_last(*l);
	np->next = it;
}
void list_put_special(list* l, const char* str)
{
	item v;
	if(IS_INT(str))
	{
		v = list_item(RPC_TYPE_INT);
		v->value.i = atoi(str);
	} 
	else if(IS_FLOAT(str))
	{
		v = list_item(RPC_TYPE_DOUBLE);
		v->value.d = (double)atof(str);
	}
	else
	{
		v = list_item(RPC_TYPE_STRING);
		v->value.s = strdup(str);
	}
	list_put(l,v);
}
item list_last(list l)
{
	item p = l;
	while(p && p->next != NULL)
		p = p->next;
	return p;
}
int list_remove(list l,int idx)
{
	if(l==NULL) return 0;
	if(idx <0 || idx >= list_size(l)) return 0;
	if(idx == 0)
	{
		l=l->next;
		return 1;
	}
	item np = list_at(l,idx-1);
	if(np == NULL) return 0;
	if(np->next == NULL) return 1;
	np->next = np->next->next;
}
int list_size(list l)
{
	if(l == NULL || l->type == RPC_TYPE_NIL) return 0;
	int i=0;
	item np = l;
	while(np)
	{
		np = np->next;
		i++;
	}
	return i;
}
char* as_string(list l)
{
	char* str = "";
	if(l != NULL && l->type != RPC_TYPE_NIL)
	{
		switch(l->type)
		{
			case RPC_TYPE_BASE64:
			str = __s("b64:%s", l->value.b64);
			break;
		
			case RPC_TYPE_BOOL:
			str = __s("bool:%d", l->value.b);
			break;
		
			case RPC_TYPE_DOUBLE:
			str = __s("double:%lf", l->value.d);
			break;
		
			case RPC_TYPE_DATE:
			str = __s("date:%s", l->value.date);
			break;
		
			case RPC_TYPE_INT:
			case RPC_TYPE_I4:
			str = __s("int:%d", l->value.i);
			break;
		
			case RPC_TYPE_STRING:
			str = __s("string:%s", l->value.s);
			break;
			
			case RPC_TYPE_ARRAY:
			str = __s("[%s]", as_string(l->value.array));
			break;
			default:
			str = "<Unknown>";
			break;
		}
		item np = l->next;
		if(np)
		{
			str = __s("%s,\n%s", str, as_string(np));
		}
		return str;
	}
	return "[empty]";
}
item list_at(list l,int idx)
{
	if(l == NULL || idx<0 || idx>= list_size(l)) 
		return NULL;
	int i=0;
	item np = l;
	while(np)
	{
		if(i==idx)
			return np;
		np = np->next;
		i++;
	}
	return NULL;
}
item list_item(int type)
{
	item ret = (item)malloc(sizeof *ret);
	ret->type = type;
	ret->next = NULL;
	return ret;
}
list split(const char* str, const char* delim)
{
	if(str == NULL || delim == NULL) return NULL;
	char* str_cpy = strdup(str);
	char* token;
	list l = list_init();
	while((token = strsep(&str_cpy,delim)))
	{
		if(strlen(token) > 0) 
		{
			list_put_special(&l,token);
		}
	}
	free(str_cpy);
	if(l->type== RPC_TYPE_NIL)
		return NULL;
	return l;
}
void list_put_i(list* l,int v)
{
	item it = list_item(RPC_TYPE_INT);
	it->value.i = v;
	list_put(l,it);
}
void list_put_d(list* l,double v)
{
	item it = list_item(RPC_TYPE_DOUBLE);
	it->value.d = v;
	list_put(l,it);
}
void list_put_b(list* l,int v)
{
	item it = list_item(RPC_TYPE_BOOL);
	it->value.b = v;
	list_put(l,it);
}
void list_put_b64(list* l,const char* v)
{
	item it = list_item(RPC_TYPE_BASE64);
	it->value.b64 = strdup(v);
	list_put(l,it);
}
void list_put_date(list* l,const char* v)
{
	item it = list_item(RPC_TYPE_DATE);
	it->value.date = strdup(v);
	list_put(l,it);
}
void list_put_s(list* l,const char* v)
{
	item it = list_item(RPC_TYPE_STRING);
	it->value.s = strdup(v);
	list_put(l,it);
}
void list_put_array(list* l,list v)
{
	item it = list_item(RPC_TYPE_ARRAY);
	it->value.array = v;
	list_put(l,it);
}
void list_free(list *l)
{
	item curr;
	while ((curr = (*l)) != NULL) { 
	    (*l) = (*l)->next;
		if(curr->type == RPC_TYPE_ARRAY)
			list_free(&curr->value.array);
	    free (curr);
	}
}
int list_empty(list l)
{
	return l== NULL || l->type == RPC_TYPE_NIL;
}
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

list_t list_init()
{
	list_t ret = (list_t)malloc(sizeof *ret);
	ret->type = LIST_TYPE_NIL;
	ret->next = NULL;
	return ret;
}
void list_put(list_t* l, item_t it)
{
	if(*l == NULL || (*l)->type == LIST_TYPE_NIL)
	{
		if(*l != NULL)
			free(*l);
		*l = it;
		return ;
	}	
	item_t np = list_last(*l);
	np->next = it;
}
void list_put_special(list_t* l, const char* str)
{
	item_t v;
	if(IS_INT(str))
	{
		v = list_item(LIST_TYPE_INT);
		v->value.i = atoi(str);
	} 
	else if(IS_FLOAT(str))
	{
		v = list_item(LIST_TYPE_DOUBLE);
		v->value.d = (double)atof(str);
	}
	else
	{
		v = list_item(LIST_TYPE_POINTER);
		v->value.ptr = strdup(str);
	}
	list_put(l,v);
}
item_t list_last(list_t l)
{
	item_t p = l;
	while(p && p->next != NULL)
		p = p->next;
	return p;
}
int list_remove(list_t* l,int idx)
{
	if(l==NULL) return 0;
	if(*l == NULL) return 0;
	item_t it;
	if(idx <0 || idx >= list_size(*l)) return 0;
	if(idx == 0)
	{
		it = *l;
		*l=(*l)->next;
		it->next = NULL;
		list_free(&it);
		return 1;
	}
	item_t np = list_at(*l,idx-1);
	if(np == NULL) return 0;
	if(np->next != NULL)
	{
		item_t it = np->next;
		it->next = NULL;
		list_free(&it);
		np->next = np->next->next;
	}
	return 1;
}
int list_size(list_t l)
{
	if(l == NULL || l->type == LIST_TYPE_NIL) return 0;
	int i=0;
	item_t np = l;
	while(np)
	{
		np = np->next;
		i++;
	}
	return i;
}
item_t list_at(list_t l,int idx)
{
	if(l == NULL || idx<0 || idx>= list_size(l)) 
		return NULL;
	int i=0;
	item_t np = l;
	while(np)
	{
		if(i==idx)
			return np;
		np = np->next;
		i++;
	}
	return NULL;
}
item_t list_item(int type)
{
	item_t ret = (item_t)malloc(sizeof *ret);
	ret->type = type;
	ret->next = NULL;
	return ret;
}
list_t split(const char* str, const char* delim)
{
	if(str == NULL || delim == NULL) return NULL;
	char* str_cpy = (char*)str;
	char* token;
	list_t l = list_init();
	while((token = strsep(&str_cpy,delim)))
	{
		if(strlen(token) > 0) 
		{
			list_put_special(&l,token);
		}
	}
	if(l->type== LIST_TYPE_NIL)
	{
		free(l);
		return NULL;
	}
	return l;
}
void list_put_i(list_t* l,int v)
{
	item_t it = list_item(LIST_TYPE_INT);
	it->value.i = v;
	list_put(l,it);
}
void list_put_d(list_t* l,double v)
{
	item_t it = list_item(LIST_TYPE_DOUBLE);
	it->value.d = v;
	list_put(l,it);
}

void list_put_ptr(list_t* l,void* v)
{
	item_t it = list_item(LIST_TYPE_POINTER);
	it->value.ptr = v;
	list_put(l,it);
}
void list_put_array(list_t* l,list_t v)
{
	item_t it = list_item(LIST_TYPE_ARRAY);
	it->value.array = v;
	list_put(l,it);
}
// unliked dictionary
// user have the responsibility to free
// pointer data
void list_free(list_t *l)
{
	item_t curr;
	while ((curr = (*l)) != NULL) { 
		(*l) = (*l)->next;
		if(curr->type == LIST_TYPE_ARRAY && curr->value.array)
			list_free(&curr->value.array);
		else if( curr->type == LIST_TYPE_POINTER && curr->value.ptr)
			free(curr->value.ptr);
	    free (curr);
	}
}
int list_empty(list_t l)
{
	return l== NULL || l->type == LIST_TYPE_NIL;
}
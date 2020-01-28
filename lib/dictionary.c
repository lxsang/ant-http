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
#include "dictionary.h"
#include "utils.h"
#include "list.h"

dictionary_t dict()
{
	return dict_n(DHASHSIZE);
}
dictionary_t dict_n(unsigned int size)
{
	dictionary_t d = (dictionary_t) malloc(sizeof(struct __dict));
	if(!d) return NULL;
	d->map= (map_t)malloc(size*sizeof(chain_t));
	if(!d->map)
	{
		free(d);
		return NULL;
	}
	d->cap = size;
	d->size = 0;
	for(unsigned int i=0; i< size;i++)
		d->map[i] = NULL;
	return d;
}
chain_t dlookup(dictionary_t dic,const char* key)
{
	chain_t np;
	if(dic->map == NULL) return NULL;
    for (np = dic->map[hash(key,dic->cap)]; np != NULL; np = np->next)
    {
    	if(!np || !np->key)
    	{
    		return NULL;
    	}
        if (strcmp(key, np->key) == 0)
          return np; /* found */
    }
    return NULL; /* not found */
}
chain_t __put_el_with_key(dictionary_t dic, const char* key)
{
	chain_t np;
    unsigned hashval;
	if(dic->map == NULL) return NULL;
    if ((np = dlookup(dic,key)) == NULL) { /* not found */
        np = (chain_t) malloc(sizeof(*np));
        if (np == NULL)
          return NULL;
		if((np->key = strdup(key)) == NULL)
		{
			free(np);
			return NULL;
		}
		np->value = NULL;
        hashval = hash(key, dic->cap);
        np->next = dic->map[hashval];
        dic->map[hashval] = np;
		dic->size++;
    }
	// found
    return np;
}

static void free_ditem_value(void* value, antd_dict_item_type_t type)
{
	switch (type)
	{
	case ANTD_DI_HEAP:
		if(value)
			free(value);
		break;
	case ANTD_DI_LIST:
		if(value)
			list_free((list_t*)&value);
		break;
	case ANTD_DI_DIC:
		if(value)
			freedict(value);
	default:
		break;
	}
}

chain_t insert(dictionary_t dic,const char* key, void* value, antd_dict_item_type_t type)
{
	chain_t np = __put_el_with_key(dic,key);
	if(np == NULL)
	{
		free_ditem_value(value, type);
		return NULL;
	}
	if(np->value && value) free_ditem_value(np->value, np->type);
	np->type = type;
	np->value = value;
    return np;
}

chain_t dremove(dictionary_t dic, const char* key)
{
	if(dic->map == NULL) return 0;
	int hashval = hash(key, dic->cap);
	chain_t np = dic->map[hashval];
	if(np!=NULL && strcmp(key,np->key)==0)
	{
		dic->size--;
		dic->map[hashval] = np->next;
		np->next = NULL;
		return np;
	}
    for (np= dic->map[hashval]; np != NULL; np = np->next)
        if (np->next!=NULL&&strcmp(key, np->next->key) == 0)
        {
			chain_t ret = np->next;
         	np->next = np->next->next; /* found */
			dic->size--;
			ret->next = NULL;
         	return ret;
        }
    return NULL; /* not found */

}
void* dvalue(dictionary_t dic, const char* key)
{
	chain_t as = dlookup(dic,key);
	if(as == NULL) return NULL;
	return as->value;
}

void free_association(chain_t * asoc)
{
	
	while( (*asoc) != NULL )
	{
		chain_t a = *asoc;
		(* asoc) = (*asoc) ->next;
		
		if(a->key)
		{
			free(a->key);
			if(a->value) free_ditem_value(a->value, a->type);
		}
		free(a);
	}

}

void freedict(dictionary_t dic){
	for(unsigned int i = 0; i < dic->cap; i++)
		free_association(&(dic->map[i]));
	free(dic->map);
	free(dic);
}



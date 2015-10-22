#include "dictionary.h"

dictionary dict()
{
	dictionary d = (dictionary)malloc(HASHSIZE*sizeof(association));
	for(int i=0; i< HASHSIZE;i++)
		d[i] = NULL;
	return d;
}
unsigned hash(const char* key)
{
	unsigned hashval;
    for (hashval = 0; *key != '\0'; key++)
      hashval = *key + 31 * hashval;
    return hashval % HASHSIZE;	
}
association dlookup(dictionary dic,const char* key)
{
	association np;
	if(dic == NULL) return NULL;
    for (np = dic[hash(key)]; np != NULL; np = np->next)
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
association __put_el_with_key(dictionary dic, const char* key)
{
	association np;
    unsigned hashval;
	if(dic == NULL) return NULL;
    if ((np = dlookup(dic,key)) == NULL) { /* not found */
        np = (association) malloc(sizeof(*np));
        if (np == NULL || (np->key = strdup(key)) == NULL)
          return NULL;
        hashval = hash(key);
        np->next = dic[hashval];
        dic[hashval] = np;
    }
    return np;
}
association dput(dictionary dic,const char* key, const char* value)
{
	if(IS_INT(value))
		return dput_i(dic,key,atoi(value));
	else if(IS_FLOAT(value))
		return dput_f(dic,key,atof(value));
	else
		return dput_s(dic,key,value);
}
association dput_s(dictionary dic,const char* key, const char* value)
{
	association np = __put_el_with_key(dic,key);
	if(np == NULL) return NULL;
	if(value == NULL) np->value.s="";
    else if ((np->value.s = strdup(value)) == NULL)
       return NULL;
    return np;
}
association dput_i(dictionary dic,const char* key, int value)
{
	association np = __put_el_with_key(dic,key);
	if(np == NULL) return NULL;
	np->value.i = value;
    return np;
}
association dput_f(dictionary dic,const char* key, float value)
{
	association np = __put_el_with_key(dic,key);
	if(np == NULL) return NULL;
	np->value.f = value;
    return np;
}
association dput_p(dictionary dic,const char* key, void* value)
{
	association np = __put_el_with_key(dic,key);
	if(np == NULL) return NULL;
	np->value.p = value;
    return np;
}
int dremove(dictionary dic, const char* key)
{
	if(dic == NULL) return 0;
	int hashval = hash(key);
	association np = dic[hashval];
	if(np!=NULL && strcmp(key,np->key)==0)
	{
		dic[hashval] = np->next;
		return 1;
	}
    for (np= dic[hashval]; np != NULL; np = np->next)
        if (np->next!=NULL&&strcmp(key, np->next->key) == 0)
        {
         	np->next = np->next->next; /* found */
         	return 1;
        }
    return 0; /* not found */

}
/**
 * Get the string data by key
 * @param  dic the dictionary
 * @param  key @
 * @return     the string value
 */
char* dvalue(dictionary dic,const char* key)
{
	association as = dlookup(dic,key);
	if(as == NULL) return NULL;
	return as->value.s;
}

int dvalue_i(dictionary dic, const char* key)
{
	association as = dlookup(dic,key);
	if(as == NULL) return 0;
	return as->value.i;
}
float dvalue_f(dictionary dic, const char* key)
{
	association as = dlookup(dic,key);
	if(as == NULL) return 0;
	return as->value.f;
}
void* dvalue_p(dictionary dic, const char* key)
{
	association as = dlookup(dic,key);
	if(as == NULL) return NULL;
	return as->value.p;
}

void freedict(dictionary dic){free(dic);}


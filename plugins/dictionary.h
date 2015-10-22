#include "utils.h"
#define HASHSIZE 255
#define for_each_assoc(assoc, dic) \
    for(int i = 0; i < HASHSIZE; i++) \
    	for(assoc = dic[i];assoc!= NULL; assoc = assoc->next)

/**
 * Dictionary for header
 */
typedef struct  __assoc{ 
    struct __assoc *next; 
    char *key; 
    union
    {
        int        	i;
        char*       s;
        float      	f;
        void* 		p; 
    } value;
    //char *value;
} * association;

typedef  association* dictionary;
dictionary dict();
unsigned hash(const char*);
association dlookup(dictionary,const char*);
char* dvalue(dictionary, const char*);
int dvalue_i(dictionary, const char*);
float dvalue_f(dictionary, const char*);
void* dvalue_p(dictionary, const char*);
association dput(dictionary,const char*, const char*);
association dput_s(dictionary,const char*, const char*);
association dput_i(dictionary,const char*, int);
association dput_f(dictionary,const char*, float);
association dput_p(dictionary,const char*, void*);
int dremove(dictionary, const char*);
void freedict(dictionary);
void stest(const char* );
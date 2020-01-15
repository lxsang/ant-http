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
#ifndef DICTIONARY_H
#define DICTIONARY_H

#define DHASHSIZE 	16
#define dput(d,k,v)             (insert(d,k,v,ANTD_DI_HEAP))
#define dput_static(d,k,v)      (insert(d,k,v,ANTD_DI_STATIC))
#define dput_list(d,k,v)        (insert(d,k,v,ANTD_DI_LIST))
#define dput_dict(d,k,v)        (insert(d,k,v,ANTD_DI_DIC))
#define for_each_assoc(assoc, dic) \
    for(unsigned int i = 0; i < dic->cap; i++) \
    	for(assoc = dic->map[i];assoc!= NULL; assoc = assoc->next)

typedef enum{ANTD_DI_STATIC, ANTD_DI_HEAP, ANTD_DI_LIST, ANTD_DI_DIC} antd_dict_item_type_t;

/**
 * Dictionary for header
 */
typedef struct  __assoc{ 
    struct __assoc *next; 
    char *key; 
    antd_dict_item_type_t type;
    void* value;
} * chain_t;

typedef  chain_t* map_t;

typedef struct __dict{
    unsigned int cap;
    map_t map;
    unsigned int size;
}* dictionary_t;

dictionary_t dict();
dictionary_t dict_n(unsigned int n);
chain_t dlookup(dictionary_t,const char*);
void* dvalue(dictionary_t, const char*);
chain_t insert(dictionary_t,const char*, void*, antd_dict_item_type_t type);
chain_t dremove(dictionary_t, const char*);
void freedict(dictionary_t);

#endif
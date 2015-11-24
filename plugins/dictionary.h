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

#include "utils.h"
#define for_each_assoc(assoc, dic) \
    for(int i = 0; i < HASHSIZE; i++) \
    	for(assoc = dic[i];assoc!= NULL; assoc = assoc->next)

/**
 * Dictionary for header
 */
typedef struct  __assoc{ 
    struct __assoc *next; 
    char *key; 
    void* value;
    //char *value;
} * association;

typedef  association* dictionary;
dictionary dict();
association dlookup(dictionary,const char*);
void* dvalue(dictionary, const char*);
association dput(dictionary,const char*, void*);
int dremove(dictionary, const char*);
void freedict(dictionary);
void stest(const char* );

#endif
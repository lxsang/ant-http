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
#ifndef LIST_H
#define LIST_H
#include "utils.h"

#define LIST_TYPE_ARRAY		0x5
#define LIST_TYPE_POINTER	0x4
#define LIST_TYPE_DOUBLE	0x2
#define LIST_TYPE_INT		0x1
#define LIST_TYPE_NIL		0x0

#define list_for_each(item, list) \
            for(item = list;item!= NULL && item->type != LIST_TYPE_NIL; item = item->next)

typedef struct __item{
	int type;
	union{
		int 			i;
		double 			d;
		void* 			ptr;
		struct __item* 	array;
	} value;
	struct __item* next;
}*item_t;
typedef item_t list_t;
list_t list_init();
void list_put(list_t*,item_t);
void list_put_i(list_t*,int);
void list_put_d(list_t*,double);
void list_put_ptr(list_t*,void*);
void list_put_array(list_t*,list_t);

item_t list_last(list_t);
int list_remove(list_t*,int);
int list_size(list_t);
item_t list_at(list_t,int);
int list_empty(list_t);
item_t list_item(int type);
list_t split(const char*, const char*);
void list_put_special(list_t*, const char*);
void list_free(list_t *);
#endif
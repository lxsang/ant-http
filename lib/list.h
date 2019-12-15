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

#define LIST_TYPE_ARRAY	601//hash("array")
#define LIST_TYPE_BASE64	335//hash("base64")
#define LIST_TYPE_BOOL	40//hash("boolean")
#define LIST_TYPE_DOUBLE	977//hash("double")
#define LIST_TYPE_DATE	49//hash("dateTime.iso8601")
#define LIST_TYPE_INT	1007//hash("int")
#define LIST_TYPE_I4		235//hash("i4")
#define LIST_TYPE_STRING	17//hash("string")
#define LIST_TYPE_NIL	529//hash("nil")

typedef struct __item{
	int type;
	union{
		int 			i;
		int 			b;
		char* 			s;
		double 			d;
		char* 			date;
		char* 			b64;
		struct __item* 	array;
	} value;
	struct __item* next;
}*item_t;
typedef item_t list_t;
list_t list_init();
void list_put(list_t*,item_t);
void list_put_i(list_t*,int);
void list_put_d(list_t*,double);
void list_put_b(list_t*,int);
void list_put_b64(list_t*,const char*);
void list_put_date(list_t*,const char*);
void list_put_s(list_t*,const char*);
void list_put_array(list_t*,list_t);
item_t list_last(list_t);
int list_remove(list_t,int);
int list_size(list_t);
item_t list_at(list_t,int);
int list_empty(list_t);
item_t list_item(int type);
list_t split(const char*, const char*);
char* as_string(list_t);
void list_put_special(list_t*, const char*);
void list_free(list_t *);
#endif
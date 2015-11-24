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
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>

#define EQU(a,b) (strcmp(a,b) == 0)
#define IEQU(a,b) (strcasecmp(a,b) == 0)
#define IS_INT(a) (match_int(a))
#define IS_FLOAT(a) (match_float(a))
#define DIR_SEP "/"
#define true 1
#define false 0
#ifdef DEBUG
	#define LOG(a,...) printf("%s:%d: " a, __FILE__, \
		__LINE__, ##__VA_ARGS__)
#else
    #define LOG(a,...) do{}while(0)
#endif
#define	BUFFLEN 	1024
#define HASHSIZE 	1024
#define DHASHSIZE 	50
			
#define RPC_TYPE_ARRAY	601//hash("array")
#define RPC_TYPE_BASE64	335//hash("base64")
#define RPC_TYPE_BOOL	40//hash("boolean")
#define RPC_TYPE_DOUBLE	977//hash("double")
#define RPC_TYPE_DATE	49//hash("dateTime.iso8601")
#define RPC_TYPE_INT	1007//hash("int")
#define RPC_TYPE_I4		235//hash("i4")
#define RPC_TYPE_STRING	17//hash("string")
#define RPC_TYPE_NIL	529//hash("nil")
			
char* __s(const char*,...);
void trim(char*,const char);
void removeAll(const char* path,int mode);
char* __time(time_t time);
char* server_time();
char* ext(const char*);
char* mime(const char*);
int is_bin(const char*);
int match_int(const char*);
int match_float(const char*);
int regex_match(const char*,const char*);
char *url_decode(const char *str);
char *url_encode(const char *str);
char from_hex(char ch);
char to_hex(char code);
unsigned hash(const char*, int);
#endif

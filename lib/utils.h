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

#include <regex.h>
#include <syslog.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "dictionary.h"

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
#define EQU(a,b) (strcmp(a,b) == 0)
#define IEQU(a,b) (strcasecmp(a,b) == 0)
#define IS_INT(a) (match_int(a))
#define IS_FLOAT(a) (match_float(a))
#define FILE_OK(f) ( access( f, F_OK ) != -1 )
#define DIR_SEP "/"
#define true 1
#define false 0

#ifdef DEBUG
	#define LOG(a,...) syslog (LOG_NOTICE,"ANTD_LOG@[%s: %d]: " a "\n", __FILE__, \
		__LINE__, ##__VA_ARGS__)
#else
    #define LOG(a,...) do{}while(0)
#endif
#define ERROR(a,...) syslog (LOG_ERR, "ANTD_ERROR@[%s: %d]: " a "\n", __FILE__, \
		__LINE__, ##__VA_ARGS__)
// add this to the utils
#define UNUSED(x) (void)(x)

#define	BUFFLEN 	1024
#define HASHSIZE 	1024

typedef struct{
	const char* type;
	const char* ext;
} mime_t;


void __attribute__((weak)) error_log(const char*, ...);
#ifdef DEBUG
void __attribute__((weak)) server_log(const char*, ...);
#endif
dictionary_t __attribute__((weak)) mimes_list();
char* __s(const char*,...);
void trim(char*,const char);
void removeAll(const char* path,int mode);
void timestr(time_t time, char* buf,int len,char* format, int gmt);
void server_time(char* , int );
char* ext(const char*);
char* mime(const char*);
int match_int(const char*);
int match_float(const char*);
int regex_match(const char*,const char*, int, regmatch_t*);
int mkdirp(const char* path,mode_t mode);
char *url_decode(const char *str);
char *url_encode(const char *str);
char from_hex(char ch);
char to_hex(char code);
mime_t mime_from_type(const char* type);
mime_t mime_from_ext(const char* ex);
unsigned hash(const char*, int);
unsigned simple_hash(const char*);
int is_file(const char* f);
int is_dir(const char* f);
int _exist(const char* f);
void md5(uint8_t *, size_t , char*);
void sha1(const char*, char*);
void digest_to_hex(const uint8_t *, char *);
void verify_header(char* k);
#endif

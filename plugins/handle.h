#ifndef HANDLE_H
#define HANDLE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef USE_DB
#include "dbhelper.h"
#endif
#include "dictionary.h"
#include "list.h"
#include "ini.h"
#include "ws.h"

#define SERVER_NAME "antd"
#define IS_POST(method) (strcmp(method,"POST")== 0)
#define IS_GET(method) (strcmp(method,"GET")== 0)
#define R_STR(d,k) ((char*)dvalue(d,k))
#define R_INT(d,k) (atoi(dvalue(d,k)))
#define R_FLOAT(d,k) ((double)atof(dvalue(d,k)))
#define R_PTR(d,k) (dvalue(d,k))
#define __RESULT__ "{\"result\":%d,\"msg\":\"%s\"}"


 

int response(int, const char*);
void ctype(int,const char*);
void redirect(int,const char*);
void html(int);
void text(int);
void json(int);
void jpeg(int);
void octstream(int, char*);
void textstream(int);
int __ti(int,int);
int __t(int, const char*,...);
int __b(int, const unsigned char*, int);
int __f(int, const char*);
int __fb(int, const char*);
int upload(const char*, const char*);

void set_cookie(int, const char*,dictionary,const char*);
void set_status(int,int,const char*);
void clear_cookie(int, dictionary);
/*Default function for plugin*/
void unknow(int);
int ws_enable(dictionary);
#endif

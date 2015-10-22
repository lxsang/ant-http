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
#define	BUFFLEN 1024

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

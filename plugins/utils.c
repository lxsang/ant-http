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
#include "utils.h"
char* __s(const char* fstring,...)
{
	char* data;
	va_list arguments; 
	int dlen;
	va_start( arguments, fstring);
    dlen = vsnprintf(0,0,fstring,arguments) + 1;
    va_end(arguments); 
    va_end(arguments); 
    if ((data = (char*)malloc(dlen*sizeof(char))) != 0)
    {
        va_start(arguments, fstring);
        vsnprintf(data, dlen, fstring, arguments);
        va_end(arguments);
        return data;
    } else
    	return "";
}


/**
 * Trim a string by a character on both ends
 * @param str   The target string
 * @param delim the delim
 */
void trim(char* str, const char delim)
{
    char * p = str;
    int l = strlen(p);

    while(p[l - 1] == delim) p[--l] = 0;
    while(* p && (* p) == delim ) ++p, --l;
    memmove(str, p, l + 1);
}

void removeAll(const char* path,int mode)
{
	DIR           *d;
	struct dirent *dir;
	char* file;
	struct stat st;
	if( stat(path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
		{
			d = opendir(path);
			if(d)
			{
				while ((dir = readdir(d)) != NULL)
				{
					if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..")==0) continue;
		  			file = __s("%s/%s",path,dir->d_name);

		  			removeAll(file,1);
		  			free(file);
				}
				closedir(d);
			}
		}
		if(mode)
			remove(path);
	}
	
}

char* __time(time_t time)
{
	struct tm *t = localtime(&time);
	char * buf = asctime(t);
	char* pos = strchr(buf,'\n');
	if(pos) *pos = '\0';
	return buf;
}
char* server_time()
{
	return __time(time(NULL));
}

/**
 * Get extension of a file name
 * @param  file The file name
 * @return      the extension
 */
char* ext(const char* file)
{
	char* token,*ltoken = "";
	if(file == NULL) return NULL;
	char* str_cpy = strdup(file);
	if(strstr(str_cpy,".")<= 0) return "";
	if(*file == '.')
		trim(str_cpy,'.');

	while((token = strsep(&str_cpy,".")) && strlen(token)>0) {ltoken = token;}
	free(str_cpy);
	return ltoken;

}
/**
 * Get correct HTTP mime type of a file
 * This is a minimalistic mime type list supported
 * by the server
 * @param  file File name
 * @return      The HTTP Mime Type
 */
char* mime(const char* file)
{
	char * ex = ext(file);
	if(IEQU(ex,"bmp"))
		return "image/bmp";
	else if(IEQU(ex,"jpg") || IEQU(ex,"jpeg"))
		return "image/jpeg";
	else if(IEQU(ex,"css"))
		return "text/css";
	else if(IEQU(ex,"csv"))
		return "text/csv";
	else if(IEQU(ex,"pdf"))
		return "application/pdf";
	else if(IEQU(ex,"gif"))
		return "image/gif";
	else if(IEQU(ex,"html")||(IEQU(ex,"htm")))
		return "text/html";
	else if(IEQU(ex,"json"))
		return "application/json";
	else if(IEQU(ex,"js"))
		return "application/javascript";
	else if(IEQU(ex,"png"))
		return "image/png";
	else if(IEQU(ex,"ppm"))
		return "image/x-portable-pixmap";
	else if(IEQU(ex,"rar"))
		return "application/x-rar-compressed";
	else if(IEQU(ex,"tiff"))
		return "image/tiff";
	else if(IEQU(ex,"tar"))
		return "application/x-tar";
	else if(IEQU(ex,"txt"))
		return "text/plain";
	else if(IEQU(ex,"ttf"))
		return "application/x-font-ttf";
	else if(IEQU(ex,"xhtml"))
		return "application/xhtml+xml";
	else if(IEQU(ex,"xml"))
		return "application/xml";
	else if(IEQU(ex,"zip"))
		return "application/zip";
	else if(IEQU(ex,"svg"))
		return "image/svg+xml";
	else if(IEQU(ex,"eot"))
		return "application/vnd.ms-fontobject";
	else if(IEQU(ex,"woff") || IEQU(ex,"woff2"))
		return "application/x-font-woff";
	else if(IEQU(ex,"otf"))
		return "application/x-font-otf";
	else 
		// The other type will be undestant as binary
		return "application/octet-stream";
}

int is_bin(const char* file)
{
	char * ex = ext(file);
	if(IEQU(ex,"bmp"))
		return true;
	else if(IEQU(ex,"jpg") || IEQU(ex,"jpeg"))
		return true;
	else if(IEQU(ex,"css"))
		return false;
	else if(IEQU(ex,"csv"))
		return false;
	else if(IEQU(ex,"pdf"))
		return true;
	else if(IEQU(ex,"gif"))
		return true;
	else if(IEQU(ex,"html")||(IEQU(ex,"htm")))
		return false;
	else if(IEQU(ex,"json"))
		return false;
	else if(IEQU(ex,"js"))
		return false;
	else if(IEQU(ex,"png"))
		return true;
	else if(IEQU(ex,"ppm"))
		return true;
	else if(IEQU(ex,"rar"))
		return true;
	else if(IEQU(ex,"tiff"))
		return true;
	else if(IEQU(ex,"tar"))
		return true;
	else if(IEQU(ex,"txt"))
		return false;
	else if(IEQU(ex,"ttf"))
		return true;
	else if(IEQU(ex,"xhtml"))
		return false;
	else if(IEQU(ex,"xml"))
		return false;
	else if(IEQU(ex,"zip"))
		return true;
	else if(IEQU(ex,"svg"))
		return false;
	else if(IEQU(ex,"eot"))
		return true;
	else if(IEQU(ex,"woff") || IEQU(ex,"woff2"))
		return true;
	else if(IEQU(ex,"otf"))
		return true;
	else 
		// The other type will be undestant as binary
		return "application/octet-stream";
}

int match_int(const char* search)
{
	return regex_match("^[-+]?[0-9]+$",search);
}
int match_float(const char* search)
{
	return regex_match("^[+-]?[0-9]*\\.[0-9]+$",search);
}
int regex_match(const char* expr,const char* search)
{
	regex_t regex;
    int reti;
    char msgbuf[100];
    int ret;
	/* Compile regular expression */
    reti = regcomp(&regex, expr, REG_ICASE | REG_EXTENDED);
    if( reti ){ 
    	LOG("Could not compile regex: %s\n",expr);
    	return 0; 
    }

	/* Execute regular expression */
    reti = regexec(&regex, search, 0, NULL, 0);
    if( !reti ){
            //LOG("Match");
            ret = 1;
    }
    else if( reti == REG_NOMATCH ){
            //LOG("No match");
            ret = 0;
    }
    else{
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        LOG("Regex match failed: %s\n", msgbuf);
        ret = 0;
    }

	
	regfree(&regex);
	return ret;
}
char *url_decode(const char *str) {
	char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;

	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') { 
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';

	return buf;
}

char *url_encode(const char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}
unsigned hash(const char* key, int hash_size)
{
	unsigned hashval;
    for (hashval = 0; *key != '\0'; key++)
      hashval = *key + 31 * hashval;
    return hashval % hash_size;	
}

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
		return true;
		//return "application/octet-stream";
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
	unsigned hashval = simple_hash(key);
    return hashval % hash_size;	
}
unsigned simple_hash(const char* key)
{
	unsigned hashval;
    for (hashval = 0; *key != '\0'; key++)
      hashval = *key + 31 * hashval;
	return hashval;
}
int is_file(const char* f)
{
	struct stat st;
	return !(stat(f, &st) == -1);
}
 
// These vars will contain the hash
 
void md5(uint8_t *initial_msg, size_t initial_len, char* buff) {
	uint32_t h0, h1, h2, h3;
	char tmp[80];
    uint8_t *msg = NULL;
    uint32_t r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    // Use binary integer part of the sines of integers (in radians) as constants// Initialize variables:
    uint32_t k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
 
    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;
 
    // Pre-processing: adding a single 1 bit
    //append "1" bit to message    
    /* Notice: the input bytes are considered as bits strings,
       where the first bit is the most significant bit of the byte.[37] */
 
    // Pre-processing: padding with zeros
    //append "0" bit until message length in bit ≡ 448 (mod 512)
    //append length mod (2 pow 64) to message
 
    int new_len;
    for(new_len = initial_len*8 + 1; new_len%512!=448; new_len++);
    new_len /= 8;
 
    msg = calloc(new_len + 64, 1); // also appends "0" bits 
                                   // (we alloc also 64 extra bytes...)
    memcpy(msg, initial_msg, initial_len);
    msg[initial_len] = 128; // write the "1" bit
 
    uint32_t bits_len = 8*initial_len; // note, we append the len
    memcpy(msg + new_len, &bits_len, 4);           // in bits at the end of the buffer
 
    // Process the message in successive 512-bit chunks:
    //for each 512-bit chunk of message:
    int offset;
    for(offset=0; offset<new_len; offset += (512/8)) {
 
        // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
        uint32_t *w = (uint32_t *) (msg + offset);
 
        // Initialize hash value for this chunk:
        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
 
        // Main loop:
        uint32_t i;
        for(i = 0; i<64; i++) {
 
            uint32_t f, g;
 
             if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5*i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3*i + 5) % 16;          
            } else {
                f = c ^ (b | (~d));
                g = (7*i) % 16;
            }

            uint32_t temp = d;
            d = c;
            c = b;
            //printf("rotateLeft(%x + %x + %x + %x, %d)\n", a, f, k[i], w[g], r[i]);
            b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
            a = temp;
 
        }
 
        // Add this chunk's hash to result so far:
 	   
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
		
    }
	uint8_t *p;
	p=(uint8_t *)&h0;
	sprintf(tmp,"%02x%02x%02x%02x", p[0], p[1], p[2], p[3], h0);
	strcpy(buff, tmp);
	p=(uint8_t *)&h1;
	sprintf(tmp,"%02x%02x%02x%02x", p[0], p[1], p[2], p[3], h1);
	strcat(buff,tmp);
	p=(uint8_t *)&h2;
	sprintf(tmp,"%02x%02x%02x%02x", p[0], p[1], p[2], p[3], h2);
    strcat(buff,tmp);
	p=(uint8_t *)&h3;
	sprintf(tmp,"%02x%02x%02x%02x", p[0], p[1], p[2], p[3], h3);
	strcat(buff,tmp);
    // cleanup
    free(msg);
 
}
void sha1(const char* text, char* out)
{	
	uint8_t d [20];
	 SHA1_CTX context;
     SHA1_Init(&context);
     SHA1_Update(&context, text, strlen(text));
     SHA1_Final(&context, d);
	 digest_to_hex(d,out);
}
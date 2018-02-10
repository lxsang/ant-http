USE_DB=TRUE
USE_SSL = TRUE
CC=gcc
EXT=dylib

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    BUILDIRD=/opt/www
	PF_FLAG=-D_GNU_SOURCE -DLINUX
endif
ifeq ($(UNAME_S),Darwin)
	BUILDIRD=../ant-build
	PF_FLAG= -DMACOS
	SSL_HEADER_PATH = -I/usr/local/opt/openssl/include
	SSL_LIB_PATH = -L/usr/local/opt/openssl/lib
endif

ifeq ($(USE_DB),TRUE)
	DB_OBJ=libs/dbhelper.o
	DB_LIB=-lsqlite3
	DB_FLAG=-D USE_DB
endif

ifeq ($(USE_DB),FALSE)
	DB_OBJ=
	DB_LIB=
	DB_FLAG=
endif

ifeq ($(USE_SSL),TRUE)
	SSL_LIB= $(SSL_LIB_PATH) -lssl -lcrypto 
	SSL_FLAG=-D USE_OPENSSL
endif

ifeq ($(USE_SSL),FALSE)
	SSL_LIB=
	SSL_FLAG=
	SSL_HEADER_PATH =
	SSL_LIB_PATH = 
endif


CFLAGS= -W  -Wall -g -std=c99 -D DEBUG $(DB_FLAG) $(PF_FLAG) $(SSL_FLAG) $(SSL_HEADER_PATH)

LIB_PATH=$(BUILDIRD)/plugins
LIB_NAME=libantd
LIB_FLAG= $(LIB_NAME).$(EXT)
SERVERLIB= -ldl $(LIB_FLAG) $(DB_LIB) $(SSL_LIB)  -lpthread

SERVER_O=plugin_manager.o \
		http_server.o
#-lsocket
PLUGINS=	dummy.$(EXT) fileman.$(EXT) pluginsman.$(EXT) wterm.$(EXT) nodedaemon.$(EXT) cookiex.$(EXT) wsimg.$(EXT)

LIBOBJS = 	libs/ini.o \
			libs/handle.o \
			$(DB_OBJ) \
			libs/dictionary.o \
			libs/base64.o \
			libs/utils.o \
			libs/ws.o \
			libs/sha1.o \
			libs/list.o 
			
PLUGINSDEP = libs/plugin.o


main: httpd plugins 


httpd: lib $(SERVER_O)
	$(CC) $(CFLAGS)  $(SERVER_O)    -o $(BUILDIRD)/httpd httpd.c $(SERVERLIB)
	cp antd $(BUILDIRD)

lib: $(LIBOBJS)
	$(CC) $(CFLAGS)  $(DB_LIB) $(SSL_LIB)  -shared -o $(LIB_NAME).$(EXT) $(LIBOBJS)
	cp $(LIB_NAME).$(EXT) $(LIB_PATH$)/
%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c $< -o $@
	
plugins: $(PLUGINS)
	
%.$(EXT): $(PLUGINSDEP) 
	for file in $(wildcard libs/$(basename $@)/*.c) ; do\
		$(CC) -fPIC $(CFLAGS)  -c  $$file -o $$file.o; \
	done
	$(CC) $(CFLAGS) $(PLUGINLIBS) $(LIB_FLAG) -shared -o $(BUILDIRD)/plugins/$(basename $@).$(EXT) \
		$(PLUGINSDEP)  libs/$(basename $@)/*.c.o


clean: sclean pclean

sclean:
	-rm -f *.o $(BUILDIRD)/httpd
	-rm *.$(EXT)
pclean:
	-rm -rf $(BUILDIRD)/plugins/* libs/*.o
	-for file in libs/* ;do \
		if [ -d "$$file" ]; then \
			rm "$$file"/*.o; \
		fi \
	done
.PRECIOUS: %.o

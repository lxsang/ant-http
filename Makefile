USE_DB=TRUE
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
endif

ifeq ($(USE_DB),TRUE)
	DB_OBJ=plugins/dbhelper.o
	DB_LIB=-lsqlite3
	DB_FLAG=-D USE_DB
endif

ifeq ($(USE_DB),FALSE)
	DB_OBJ=
	DB_LIB=
	DB_FLAG=
endif


CFLAGS= -W  -Wall -g -std=c99 -D DEBUG $(DB_FLAG) $(PF_FLAG)

LIB_PATH=$(BUILDIRD)/plugins
LIB_NAME=libantd
LIB_FLAG= $(LIB_NAME).$(EXT)
SERVERLIB=-lpthread -ldl $(LIB_FLAG)

SERVER_O=plugin_manager.o \
		http_server.o \
		httpd.o
#-lsocket
PLUGINS=	dummy.$(EXT) fileman.$(EXT) pluginsman.$(EXT) wterm.$(EXT) nodedaemon.$(EXT) cookiex.$(EXT) wsimg.$(EXT)

LIBOBJS = 	plugins/ini.o \
			plugins/handle.o \
			$(DB_OBJ) \
			plugins/dictionary.o \
			plugins/base64.o \
			plugins/utils.o \
			plugins/ws.o \
			plugins/sha1.o \
			plugins/list.o 
			
PLUGINSDEP = plugins/plugin.o


main: httpd plugins 


httpd: lib $(SERVER_O)
	$(CC) $(CFLAGS)  $(SERVER_O)  $(SERVERLIB)  -o $(BUILDIRD)/httpd 
	cp antd $(BUILDIRD)

lib: $(LIBOBJS)
	$(CC) $(CFLAGS)  $(DB_LIB)  -shared -o $(LIB_NAME).$(EXT) $(LIBOBJS)
	cp $(LIB_NAME).$(EXT) $(LIB_PATH$)/
%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c $< -o $@
	
plugins: $(PLUGINS)
	
%.$(EXT): $(PLUGINSDEP) 
	for file in $(wildcard plugins/$(basename $@)/*.c) ; do\
		$(CC) -fPIC $(CFLAGS)  -c  $$file -o $$file.o; \
	done
	$(CC) $(CFLAGS) $(PLUGINLIBS) $(LIB_FLAG) -shared -o $(BUILDIRD)/plugins/$(basename $@).$(EXT) \
		$(PLUGINSDEP)  plugins/$(basename $@)/*.c.o


clean: sclean pclean

sclean:
	-rm -f *.o $(BUILDIRD)/httpd
	-rm *.$(EXT)
pclean:
	-rm -rf $(BUILDIRD)/plugins/* plugins/*.o
	-for file in plugins/* ;do \
		if [ -d "$$file" ]; then \
			rm "$$file"/*.o; \
		fi \
	done
.PRECIOUS: %.o

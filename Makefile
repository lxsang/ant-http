CC=gcc
EXT=dylib
SERVER=plugin_manager.o \
		plugins/ini.o \
		http_server.o \
		plugins/dictionary.o \
		plugins/base64.o \
		plugins/sha1.o \
		plugins/ws.o \
		plugins/utils.o
SERVERLIB=-lpthread -ldl
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    BUILDIRD=/opt/www
	PF_FLAG=-D_GNU_SOURCE -DLINUX
endif
ifeq ($(UNAME_S),Darwin)
	BUILDIRD=../ant-build
	PF_FLAG= -DMACOS
endif
CFLAGS=-W -Wall -g -std=c99 -D DEBUG -D USE_DB $(PF_FLAG)
#-lsocket
PLUGINS=	dummy.$(EXT) fileman.$(EXT) pluginsman.$(EXT) wterm.$(EXT) nodedaemon.$(EXT) cookiex.$(EXT)

PLUGINSDEP = plugins/ini.o \
				plugins/plugin.o \
				plugins/dbhelper.o \
				plugins/dictionary.o \
				plugins/base64.o \
				plugins/utils.o \
				plugins/ws.o \
				plugins/sha1.o \
				plugins/list.o 
PLUGINLIBS = -lsqlite3

main: httpd plugins 


httpd:$(SERVER)
	$(CC) $(CFLAGS) $(SERVERLIB) $(SERVER)  -o $(BUILDIRD)/httpd httpd.c
	cp antd $(BUILDIRD)

%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c $< -o $@
plugins: $(PLUGINS)
	
%.$(EXT): $(PLUGINSDEP) 
	for file in $(wildcard plugins/$(basename $@)/*.c) ; do\
		$(CC) -fPIC $(CFLAGS)  -c  $$file -o $$file.o; \
	done
	$(CC) $(CFLAGS) $(PLUGINLIBS) -shared -o $(BUILDIRD)/plugins/$(basename $@).$(EXT) \
		$(PLUGINSDEP) plugins/$(basename $@)/*.c.o


clean: sclean pclean

sclean:
	rm -f *.o $(BUILDIRD)/httpd
pclean:
	rm -rf $(BUILDIRD)/plugins/* plugins/*.o
	-for file in plugins/* ;do \
		if [ -d "$$file" ]; then \
			rm "$$file"/*.o; \
		fi \
	done
.PRECIOUS: %.o

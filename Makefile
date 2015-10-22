CC=gcc
CFLAGS=-W -Wall -g -std=c99 -D DEBUG
BUILDIRD=build
EXT=dylib
SERVER=plugin_manager.o ini.o http_server.o plugins/dictionary.o plugins/utils.o
SERVERLIB=-lpthread -ldl
#-lsocket
PLUGINS=	dummy.$(EXT) fileman.$(EXT) pluginsman.$(EXT)  

PLUGINSDEP = plugins/plugin.o plugins/dbhelper.o plugins/dictionary.o plugins/utils.o plugins/list.o
PLUGINLIBS = -lsqlite3

main: httpd plugins


httpd:$(SERVER)
	$(CC) $(CFLAGS) $(SERVERLIB) $(SERVER)  -o $(BUILDIRD)/httpd httpd.c

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
plugins: $(PLUGINS)
	
%.$(EXT): $(PLUGINSDEP) 
	for file in $(wildcard plugins/$(basename $@)/*.c) ; do\
		$(CC) $(CFLAGS) -c $$file -o $$file.o; \
	done
	$(CC) $(CFLAGS) $(PLUGINLIBS) -shared -o $(BUILDIRD)/plugins/$(basename $@).$(EXT) \
		$(PLUGINSDEP) plugins/$(basename $@)/*.c.o


clean: sclean pclean

sclean:
	rm -f *.o build/httpd
pclean:
	rm -f build/plugins/* plugins/*.o
	-for file in plugins/* ;do \
		if [ -d "$$file" ]; then \
			rm "$$file"/*.o; \
		fi \
	done
.PRECIOUS: %.o
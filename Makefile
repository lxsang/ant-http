include var.mk
LIB_PATH=$(BUILDIRD)/plugins
LIB_NAME=libantd
LIB_FLAG= $(LIB_NAME).$(EXT)
SERVERLIB= -ldl $(LIB_FLAG) $(DB_LIB) $(SSL_LIB)  -lpthread

SERVER_O=plugin_manager.o \
		http_server.o
#-lsocket

LIBOBJS = 	libs/ini.o \
			libs/handle.o \
			$(DB_OBJ) \
			libs/dictionary.o \
			libs/base64.o \
			libs/utils.o \
			libs/ws.o \
			libs/sha1.o \
			libs/list.o \
			libs/scheduler.o
			
PLUGINSDEP = libs/plugin.o


main:  initd httpd antd_plugins 

initd:
	-mkdir -p $(LIB_PATH)

httpd: lib $(SERVER_O)
	$(CC) $(CFLAGS)  $(SERVER_O)    -o $(BUILDIRD)/httpd httpd.c $(SERVERLIB)
	cp antd $(BUILDIRD)

relay: lib $(SERVER_O)
	$(CC) $(CFLAGS)  $(SERVER_O)    -o $(BUILDIRD)/relay relay.c $(SERVERLIB)
	cp forward $(BUILDIRD)
lib: $(LIBOBJS)
	$(CC) $(CFLAGS)  $(DB_LIB) $(SSL_LIB)  -shared -o $(LIB_NAME).$(EXT) $(LIBOBJS)
	cp $(LIB_NAME).$(EXT) $(LIB_PATH$)/
%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c $< -o $@
	
antd_plugins:
	- echo "make plugin"
	-for file in plugins/* ; do\
		echo $$file;\
		if [ -d "$$file" ]; then \
			make -C  "$$file" clean; \
			make -C  "$$file" main; \
		fi \
	done

plugin:
	read -r -p "Enter package name: " PKG;\
	cd plugins/$$PKG && make clean && make\

clean: sclean pclean

sclean:
	-rm -f *.o $(BUILDIRD)/httpd
	-rm *.$(EXT)
pclean:
	-rm -rf $(BUILDIRD)/plugins/* libs/*.o
	-for file in plugins/* ;do \
		if [ -d "$$file" ]; then \
			make -C  "$$file" clean; \
		fi \
	done
.PRECIOUS: %.o

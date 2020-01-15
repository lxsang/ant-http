AUTOMAKE_OPTIONS = foreign

# AM_CFLAGS = -g -O0

# check for system
#if LINUX
#    AM_CPPFLAGS = -Wl,--no-as-needed
#else
#    AM_CPPFLAGS = -Wl,-undefined,dynamic_lookup
#endif


AM_CPPFLAGS = -W  -Wall -g -std=c99 -DCONFIG_FILE=\"$(sysconfdir)/antd-config.ini\"
# dynamic library
lib_LTLIBRARIES = libantd.la
libantd_la_SOURCES =  lib/ini.c \
			lib/handle.c \
			lib/dictionary.c \
			lib/base64.c \
			lib/utils.c \
			lib/ws.c \
			lib/sha1.c \
			lib/list.c \
			lib/scheduler.c \
			lib/h2.c

pkginclude_HEADERS =  lib/ini.h \
			lib/handle.h \
			lib/dictionary.h \
			lib/base64.h \
			lib/utils.h \
			lib/ws.h \
			lib/sha1.h \
			lib/list.h \
			lib/scheduler.h \
            lib/plugin.h \
			lib/h2.h


EXTRA_DIST = plugin_manager.h http_server.h README.md LICENSE antd-config.ini
# check for db 
if DB
libantd_la_SOURCES += lib/dbhelper.c
pkginclude_HEADERS += lib/dbhelper.h
endif

# bin
bin_PROGRAMS = antd
# lib source files
antd_SOURCES = plugin_manager.c http_server.c httpd.c
antd_LDADD = libantd.la

sysconf_DATA = antd-config.ini

#install-data-local: $(srcdir)/conf/config.file $(srcdir)/conf/sub1/config.file
#        mkdir $(sysconfdir)/conf
#        cp $(srcdir)/conf/config.file $(sysconfdir)/conf
#        mkdir $(sysconfdir)/conf/sub1
#        cp $(srcdir)/conf/sub1/config.file $(sysconfdir)/conf/sub1
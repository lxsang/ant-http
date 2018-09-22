USE_DB=TRUE
USE_SSL = TRUE
CC=gcc
EXT=dylib
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    BUILDIRD=/opt/www
	PF_FLAG=-D_GNU_SOURCE -DLINUX
	PPF_FLAG=-D_GNU_SOURCE -DLINUX  -Wl,--no-as-needed
endif
ifeq ($(UNAME_S),Darwin)
	BUILDIRD=/Users/mrsang/Documents/build/www
	PF_FLAG= -DMACOS
	PPF_FLAG=-D_GNU_SOURCE -DMACOS -Wl,-undefined,dynamic_lookup
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

# xplugin variables
PLUGINS_BASE=../../libs
PBUILDIRD=$(BUILDIRD)/plugins
LIB_CFLAGS= -W -Wall  -g -std=c99 -W $(PPF_FLAG)
APP_DIR=$(BUILDIRD)/htdocs/
INCFLAG= -I$(PLUGINS_BASE)

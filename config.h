#define CONFIG "config.ini"
#include "plugins/dictionary.h"

typedef struct  { 
	int port;
    char *plugins_dir; 
    char *plugins_ext;
    char *db_path;
    char* htdocs;
    char* tmpdir;
    dictionary rules;
    int backlog;
}config_t;

extern config_t server_config; 
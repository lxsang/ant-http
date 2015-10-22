#define CONFIG "config.ini"

typedef struct  { 
	int port;
    char *plugins_dir; 
    char *plugins_ext;
    char *db_path;
    char* htdocs;
    char* tmpdir;
}config_t;

extern config_t server_config; 
#include "plugin_manager.h"
/**
 * Plugin table to store the loaded plugin
 */
static struct plugin_entry *plugin_table[HASHSIZE]; 

/**
 * Locate a plugin in the plugin table
 * @param  s plugin name
 * @return   a plugin entry in the plugin table
 */
struct plugin_entry *plugin_lookup(char *s)
{
    struct plugin_entry *np;
    for (np = plugin_table[hash(s, HASHSIZE)]; np != NULL; np = np->next)
        if (strcmp(s, np->pname) == 0)
          return np; /* found */
    return NULL; /* not found */
}

/**
 * Load a plugin to the plugin table
 * Only load when not available in the plugin table
 * @param  name plugin name
 * @return      pointer to the loaded plugin
 */
struct plugin_entry *plugin_load(char *name)
{
    struct plugin_entry *np;
    unsigned hashval;
    if ((np = plugin_lookup(name)) == NULL) { /* not found */
        np = (struct plugin_entry *) malloc(sizeof(*np));
        if (np == NULL || (np->pname = strdup(name)) == NULL)
        {
			if(np) free(np);
			return NULL;
		}
        if ((np->handle = plugin_from_file(name)) == NULL)
		{
			if(np) free(np);
       		return NULL;
		}
		hashval = hash(name,HASHSIZE);
        np->next = plugin_table[hashval];
        plugin_table[hashval] = np;
    } else /* already there */
    {
    	LOG("The plugin %s id already loaded\n", name);
    }
   
    return np;
}
/**
 * Find a plugin in a file, and load it in to the plugin table
 * @param  name Name of the plugin
 * @return      
 */
void * plugin_from_file(char* name)
{
	void *lib_handle;
  char* error;
  char* path = __s("%s%s%s",config()->plugins_dir,name,config()->plugins_ext);
  void (*fn)(const char*, config_t*);
   lib_handle = dlopen(path, RTLD_LAZY);
   if (!lib_handle) 
   {
      LOG("Cannot load plugin '%s' : '%s'\n",name,dlerror());
	  if(path)
		free(path);
      return NULL;
   }
   // set database path
   fn = (void (*)(const char *, config_t*))dlsym(lib_handle, "__init_plugin__");
  if ((error = dlerror()) != NULL)  
  		LOG("Problem when setting data path for %s : %s \n", name,error);
  else
	(*fn)(name,config()); 
  if(path)
	free(path);
   return lib_handle;
}

void unload_plugin(struct plugin_entry* np)
{
	char* error;
	void (*fn)() = NULL;
	// find and execute the exit function
	fn = (void(*)()) dlsym(np->handle, "__release__");
	if ((error = dlerror()) != NULL)  
 	{
     	LOG("Cant not release plugin %s : %s \n", np->pname,error);
    }
	if(fn)
	{
		(*fn)();
	}
	dlclose(np->handle);
	//free((void *) np->handle);
	free((void *) np->pname);
}
/*
	Unload a plugin by its name
*/
void unload_plugin_by_name(const char* name)
{
	LOG("%s\n","Unloading thing");
	struct plugin_entry *np;
	int hasval = hash(name, HASHSIZE);
	np = plugin_table[hasval];
	if(strcmp(np->pname,name) == 0)
	{
		unload_plugin(np);
		plugin_table[hasval] = np->next;
	}
	else
	{
	    for (; np != NULL; np = np->next)
	        if (np->next != NULL  && strcmp(name, np->next->pname) == 0)
				break;
		if(np == NULL) return; // the plugin is is not loaded
		unload_plugin(np->next);
		np->next = np->next->next;
	}
}
/**
 * Unload all the plugin loaded on the plugin table
 */
void unload_all_plugin()
{
	LOG("Unload all plugins\n");
	for(int i=0;i<HASHSIZE;i++)
	{
		struct plugin_entry **np, *curr;
		np = &plugin_table[i];

		while((curr = *np) != NULL)
		{
			(*np) = (*np)->next;
			unload_plugin(curr);
			free(curr);
		}
        plugin_table[i] = NULL;
	}
	exit(0);
}


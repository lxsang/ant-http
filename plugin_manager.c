#include <dlfcn.h>
#include <string.h>
#include "plugin_manager.h"
#include "lib/utils.h"
#include "lib/handle.h"
#include "http_server.h"
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

int require_plugin(const char* name)
{
	struct plugin_entry* ptr = plugin_load((char*)name);
	return ptr != NULL;
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
        if (np == NULL || name == NULL)
        {
			if(np) free(np);
			return NULL;
		}
		np->pname = strdup(name);
        if ((np->handle = plugin_from_file(name)) == NULL)
		{
			if(np->pname) free(np->pname);
			if(np) free(np);
       		return NULL;
		}
		hashval = hash(name,HASHSIZE);
        np->next = plugin_table[hashval];
        plugin_table[hashval] = np;
    } else /* already there */
    {
    	LOG("The plugin %s id already loaded", name);
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
  void (*fn)(const char*);
   lib_handle = dlopen(path, RTLD_LAZY);
   if (!lib_handle) 
   {
      ERROR("Cannot load plugin '%s' : '%s'",name,dlerror());
	  if(path)
		free(path);
      return NULL;
   }
   // set database path
   fn = (void (*)(const char *))dlsym(lib_handle, "__init_plugin__");
  if ((error = dlerror()) != NULL)  
  		ERROR("Problem when finding plugin init function for %s : %s", name,error);
  else
	(*fn)(name); 
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
     	ERROR("Cant not release plugin %s : %s", np->pname,error);
    }
	if(fn)
	{
		(*fn)();
	}
	dlclose(np->handle);
	//free((void *) np->handle);
	if(np->pname)
		free((void *) np->pname);
}
/*
	Unload a plugin by its name
*/
void unload_plugin_by_name(const char* name)
{
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
	    for (np = plugin_table[hasval] ; np != NULL; np = np->next)
		{
	        if (np->next != NULL  && strcmp(name, np->next->pname) == 0)
			{
				break;
			}
		}
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
	LOG("Unload all plugins");
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


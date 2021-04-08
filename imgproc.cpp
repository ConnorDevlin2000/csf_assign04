/*
 * Image processing with plugins
 * CSF Assignment 4
 * Connor Devlin - cdevlin4@jh.edu
 * Marc Helou - mhelou1@jh.edu
 */

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include "image.h"

using namespace std;

//Plugin struct
struct Plugin {
    void *handle;
    const char *(*get_plugin_name)(void);
    const char *(*get_plugin_desc)(void);
    void *(*parse_arguments)(int num_args, char *args[]);
    struct Image *(*transform_image)(struct Image *source, void *arg_data);
};

/*
 * Get the directory of the plugin from the environmental variable.
 * Parameters:
 *    N/A
 * Returns:
 *    A char array that contains the plugin directory name
 */
char* getPluginDirectory(){
    if(getenv("PLUGIN_DIR") == NULL){
        return "./plugins";
    } else {
        return getenv("PLUGIN_DIR");
    }
}

/*
 * Iterates through the directory and loads the plugins
 * Parameters:
 *    plugins - the vector you wish to push the plugins to
 *    PLUGIN_DIR - the name of the directory containing the plugins
 * Returns:
 *    An int, 1 if the function encountered an error, 0 otherwise.
 */
int loadPlugins(DIR* directory, vector<Plugin*> &plugins, const char* PLUGIN_DIR){
    struct dirent* entry;
    //Iterate over the directory
    while((entry = readdir(directory))) {
        //Begin forming path of plugin
        string name = entry->d_name;
        string path = PLUGIN_DIR;
        path += "/";
        //If file ends with .so then create the plugin struct and assign appropriate values
        if (name.rfind(".so") != string::npos && name.substr(name.rfind(".so")).compare(".so") == 0) {
            Plugin* p = (Plugin *)(malloc(sizeof(Plugin)));
            path += entry->d_name;
            char charArrPath[path.length() + 1];
            strcpy(charArrPath, path.c_str());
            charArrPath[path.length()] = '\0';
            p->handle = dlopen(charArrPath, RTLD_LAZY);
            //If the handle is NULL, we cannot load the plugin
            if(p->handle == NULL){
                cerr << "Error: Could not load plugin" << endl;
                return 0;
            }
            //Load in function pointers
            *(void**)(&p->get_plugin_name) = dlsym(p->handle, "get_plugin_name");
            *(void**)(&p->get_plugin_desc) = dlsym(p->handle, "get_plugin_desc");
            *(void**)(&p->parse_arguments) = dlsym(p->handle, "parse_arguments");
            *(void**)(&p->transform_image) = dlsym(p->handle, "transform_image");
            //Check if any function pointers are null
            if (p->get_plugin_name == NULL || p->get_plugin_desc == NULL || p->parse_arguments == NULL || p->transform_image == NULL) {
                cerr << "Error: Could not find required API function" << endl;
                return 0;
            }
            //Push plugin to vector of plugins if no error encountered
            plugins.push_back(p);
        }
    }
    return 1;
}

/*
 * Lists the plugins' names and descriptions
 * Parameters:
 *    plugins - the vector of plugins you want to iterate through
 * Returns:
 *    void
 */
void listPlugins(vector<Plugin*> plugins){
    cout << "Loaded " << plugins.size() << " plugin(s)" << endl;
    for(int i = 0; i < plugins.size(); i++){
        cout << " " << (plugins[i]->get_plugin_name()) << ": " << (plugins[i]->get_plugin_desc()) << endl;
    }
}

/*
 * Finds a specific plugin in a vector of plugins
 * Parameters:
 *    pluginName - a char array containing the name of the plugin you want to find
 *    plugin - the vector of plugins you want to iterate through
 * Returns:
 *    A pointer to the plugin struct of the found object, null if the plugin wasn't found
 */
Plugin* findPlugin(char* pluginName, vector<Plugin*> plugins){
    bool found = false;
    Plugin* toExecute;
    //Iterate through vector looking for plugin with pluginName
    for(int i = 0; i < plugins.size(); i++) {
        if (strcmp(plugins[i]->get_plugin_name(), pluginName) == 0) {
            toExecute = plugins[i];
            found = true;
            break;
        }
    }
    //If not found print error message and return NULL
    if (!found) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        cerr << "Error: Could not find plugin" << endl;
        return NULL;
    } else {
        return toExecute;
    }
}

/*
 * Checks commandline arguments for any issues
 * Parameters:
 *    argc - number of commandline arguments received
 *    argv - array of char arrays which contain the commandline arguments
 * Returns:
 *    void
 */
int handleCommandLineErrors(int argc, char* argv[]){
    if (argc == 1) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        return 0;
    } else if (strcmp(argv[1], "list") == 0 && argc != 2) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        cerr << "Error: Too many arguments" << endl;
        return 0;
    } else if (strcmp(argv[1], "exec") == 0 && argc < 5) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        cerr << "Error: Insufficient arguments" << endl;
        return 0;
    } else if (strcmp(argv[1], "list") != 0 && strcmp(argv[1], "exec") != 0) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        cerr << "Error: Unknown command" << endl;
    } else {
        return 1;
    }
}

/*
 * Frees all allocated memory (i.e. images, plugins, directory)
 * Parameters:
 *    plugins - the vector of plugins you want to free
 *    img - the image you want to destroy
 *    transformedImg - the other image you want to destroy
 *    directory - the directory you wish to close
 * Returns:
 *    void
 */
void freeAll(vector<Plugin*> plugins, Image* img, Image* transformedImg, DIR* directory){
    for(int i = 0; i < plugins.size(); i++) {
        dlclose(plugins[i]->handle);
        free(plugins[i]);
    }
    img_destroy(img);
    img_destroy(transformedImg);
    closedir(directory);
}

/*
 * Frees all plugins in plugins vector
 * Parameters:
 *    plugins - the vector of plugins you want to free
 *    directory - the directory you wish to close
 * Returns:
 *    void
 */
void freePlugins(vector<Plugin*> plugins, DIR* directory){
    for(int i = 0; i < plugins.size(); i++) {
        dlclose(plugins[i]->handle);
        free(plugins[i]);
    }
    closedir(directory);
}

/*
 * Frees all plugins in plugins vector and closes image as well as directory
 * Parameters:
 *    plugins - the vector of plugins you want to free
 *    img - the image you want to destroy
 *    directory - the directory you wish to close
 * Returns:
 *    void
 */
void freePluginAndImage(vector<Plugin*> plugins, Image* img, DIR* directory){
    freePlugins(plugins, directory);
    img_destroy(img);
}

/*
 * Loads a particular directory
 * Parameters:
 *    PLUGIN_DIR - a char array containing the directory name
 * Returns:
 *    A pointer to a DIR, NULL if the directory couldn't be opened
 */
DIR* loadDirectory(const char* PLUGIN_DIR){
    DIR* directory = opendir(PLUGIN_DIR);
    if(directory == NULL) {
        cerr << "Error: No such directory" << endl;
        return NULL;
    } else {
        return directory;
    }
}

int main(int argc, char *argv[]){
    const char* PLUGIN_DIR = getPluginDirectory();
    vector<Plugin*> plugins;
    //Check commandline arguments for validity
    if (handleCommandLineErrors(argc, argv) == 0) {
        return 1;
    } else {
        DIR* directory = loadDirectory(PLUGIN_DIR);
        //Check if directory was found
        if (directory == NULL) {
            return 1;
        } else {
            //If loadPlugins fails, free appropriate memory and return non-zero value
            if (loadPlugins(directory, plugins, PLUGIN_DIR) == 0) {
                freePlugins(plugins, directory);
                return 1;
            }
            //If list function was entered, list plugin names and descriptions
            if (strcmp(argv[1], "list") == 0) {
                listPlugins(plugins);
            } else if (strcmp(argv[1], "exec") == 0) {
                //Find the plugin they wish to execute
                Plugin* toExecute = findPlugin(argv[2], plugins);
                //Check if plugin was found, if not found free appropriate memory and return non-zero value
                if (toExecute == NULL) {
                    freePlugins(plugins, directory);
                    return 1;
                } else {
                    //Load in image they wish to apply plugin to
                    Image* img = img_read_png(argv[3]);
                    //If image not found, print error message, free appropriate memory, and return non-zero value
                    if (img == NULL) {
                        cerr << "Error: Cannot ready PNG file" << endl;
                        freePlugins(plugins, directory);
                        return 1;
                    } else {
                        Image* transformedImg;
                        void* arguments;
                        //Give necessary plugin arguments
                        arguments = toExecute->parse_arguments(argc - 5, argv + 5);
                        //If invalid arguments for plugin, print error message, free appropriate memory, and return non-zero value
                        if (arguments == NULL) {
                            cerr << "Error: Invalid plugin arguments" << endl;
                            freePluginAndImage(plugins, img, directory);
                            return 1;
                        }
                        //Apply image transformation
                        transformedImg = toExecute->transform_image(img, arguments);
                        //If image transformation fails, print error message, free appropriate memory, and return non-zero value
                        if (transformedImg == NULL) {
                            cerr << "Error: Could not transform image" << endl;
                            freePluginAndImage(plugins, img, directory);
                            return 1;
                        }
                        //Write transformed image to specified file name
                        img_write_png(transformedImg, argv[4]);
                        //Free all allocated memory
                        freeAll(plugins, img, transformedImg, directory);
                    }
                }
            }
        }
    }
    return 0;
}
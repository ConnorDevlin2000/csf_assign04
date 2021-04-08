//Connor Devlin - cdevlin4@jh.edu
//Marc Helou - mhelou1@jh.edu

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include "image.h"

using namespace std;

struct Plugin {
    void *handle;
    const char *(*get_plugin_name)(void);
    const char *(*get_plugin_desc)(void);
    void *(*parse_arguments)(int num_args, char *args[]);
    struct Image *(*transform_image)(struct Image *source, void *arg_data);
};

char* getPluginDirectory(){
    if(getenv("PLUGIN_DIR") == NULL){
        return "./plugins";
    } else {
        return getenv("PLUGIN_DIR");
    }
}

int loadPlugins(DIR* directory, vector<Plugin*> &plugins, const char* PLUGIN_DIR){
    struct dirent* entry;
    while((entry = readdir(directory))) {
        string name = entry->d_name;
        string path = PLUGIN_DIR;
        path += "/";
        if (name.rfind(".so") != string::npos && name.substr(name.rfind(".so")).compare(".so") == 0) {
            Plugin* p = (Plugin *)(malloc(sizeof(Plugin)));
            path += entry->d_name;
            char charArrPath[path.length() + 1];
            strcpy(charArrPath, path.c_str());
            charArrPath[path.length()] = '\0';
            p->handle = dlopen(charArrPath, RTLD_LAZY);
            if(p->handle == NULL){
                cerr << "Error: Could not load plugin" << endl;
                return 0;
            }
            *(void**)(&p->get_plugin_name) = dlsym(p->handle, "get_plugin_name");
            *(void**)(&p->get_plugin_desc) = dlsym(p->handle, "get_plugin_desc");
            *(void**)(&p->parse_arguments) = dlsym(p->handle, "parse_arguments");
            *(void**)(&p->transform_image) = dlsym(p->handle, "transform_image");
            if (p->get_plugin_name == NULL || p->get_plugin_desc == NULL || p->parse_arguments == NULL || p->transform_image == NULL) {
                cerr << "Error: Could not find required API function" << endl;
                return 0;
            }
            plugins.push_back(p);
        }
    }
    return 1;
}

int listPlugins(vector<Plugin*> plugins, int argc){
    cout << "Loaded " << plugins.size() << " plugin(s)" << endl;
    for(int i = 0; i < plugins.size(); i++){
        cout << " " << (plugins[i]->get_plugin_name()) << ": " << (plugins[i]->get_plugin_desc()) << endl;
    }
    return 1;
}

Plugin* findPlugin(char* pluginName, vector<Plugin*> plugins){
    bool found = false;
    Plugin* toExecute;
    for(int i = 0; i < plugins.size(); i++) {
        if (strcmp(plugins[i]->get_plugin_name(), pluginName) == 0) {
            toExecute = plugins[i];
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        cerr << "Error: Could not find plugin" << endl;
        return NULL;
    } else {
        return toExecute;
    }
}

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

void freeAll(vector<Plugin*> plugins, Image* img, Image* transformedImg, DIR* directory){
    for(int i = 0; i < plugins.size(); i++) {
        dlclose(plugins[i]->handle);
        free(plugins[i]);
    }
    img_destroy(img);
    img_destroy(transformedImg);
    closedir(directory);
}

void freePlugins(vector<Plugin*> plugins, DIR* directory){
    for(int i = 0; i < plugins.size(); i++) {
        dlclose(plugins[i]->handle);
        free(plugins[i]);
    }
    closedir(directory);
}

void freePluginAndImage(vector<Plugin*> plugins, Image* img, DIR* directory){
    freePlugins(plugins, directory);
    img_destroy(img);
}

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
    if (handleCommandLineErrors(argc, argv) == 0) {
        return 1;
    } else {
        DIR* directory = loadDirectory(PLUGIN_DIR);
        if (directory == NULL) {
            return 1;
        } else {
            if (loadPlugins(directory, plugins, PLUGIN_DIR) == 0) {
                freePlugins(plugins, directory);
                return 1;
            }
            if (strcmp(argv[1], "list") == 0) {
                listPlugins(plugins, argc);
            } else if (strcmp(argv[1], "exec") == 0) {
                Plugin* toExecute = findPlugin(argv[2], plugins);
                if (toExecute == NULL) {
                    freePlugins(plugins, directory);
                    return 1;
                } else {
                    Image* img = img_read_png(argv[3]);
                    if (img == NULL) {
                        cerr << "Error: Cannot ready PNG file" << endl;
                        freePlugins(plugins, directory);
                        return 1;
                    } else {
                        Image* transformedImg;
                        void* arguments;
                        arguments = toExecute->parse_arguments(argc - 5, argv + 5);
                        if (arguments == NULL) {
                            cerr << "Error: Invalid plugin arguments" << endl;
                            freePluginAndImage(plugins, img, directory);
                            return 1;
                        }
                        transformedImg = toExecute->transform_image(img, arguments);
                        if (transformedImg == NULL) {
                            cerr << "Error: Could not transform image" << endl;
                            freePluginAndImage(plugins, img, directory);
                            return 1;
                        }
                        img_write_png(transformedImg, argv[4]);
                        freeAll(plugins, img, transformedImg, directory);
                    }
                }
            }
        }
    }
    return 0;
}
//Connor Devlin - cdevlin4@jh.edu
//Marc Helou - mhelou1@jh.edu

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>

using namespace std;

struct Plugin {
    void *handle;
    const char *(*get_plugin_name)(void);
    const char *(*get_plugin_desc)(void);
    void *(*parse_arguments)(int num_args, char *args[]);
    struct Image *(*transform_image)(struct Image *source, void *arg_data);
};

int main(int argc, char *argv[]){

    const char* PLUGIN_DIR;

    if(getenv("PLUGIN_DIR") == NULL){
        PLUGIN_DIR = "./plugins";
    } else {
        PLUGIN_DIR = getenv("PLUGIN_DIR");
    }

    struct dirent* entry;
    vector<Plugin*> plugins;

    if(argc == 1 || !(argc >= 2 && argc <= 6)){

        cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
        return 1;

    } else {

        DIR* directory = opendir(PLUGIN_DIR);
        if(directory == NULL){

            cerr << "Error: No such directory." << endl;
            return 1;

        } else {
            while((entry = readdir(directory))){
                string name = entry->d_name;
                string path = PLUGIN_DIR;
                path += "/";
                if(name.rfind(".so") != string::npos && name.substr(name.rfind(".so")).compare(".so") == 0){
                    Plugin* p = (Plugin *)(malloc(sizeof(Plugin)));
                    path += entry->d_name;
                    char charArrPath[path.length() + 1];
                    strcpy(charArrPath, path.c_str());
                    charArrPath[path.length()] = '\0';
                    p->handle = dlopen(charArrPath, RTLD_LAZY);
                    *(void**)(&p->get_plugin_name) = dlsym(p->handle, "get_plugin_name");
                    *(void**)(&p->get_plugin_desc) = dlsym(p->handle, "get_plugin_desc");
                    *(void**)(&p->parse_arguments) = dlsym(p->handle, "parse_arguments");
                    *(void**)(&p->parse_arguments) = dlsym(p->handle, "transform_image");
                    plugins.push_back(p);
                }

            }
        }
        closedir(directory);
    }
    
    
    else if (strcmp(argv[1], "list") == 0){

        

    }

    

    

}
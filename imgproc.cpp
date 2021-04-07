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

int main(int argc, char *argv[]){

    const char* PLUGIN_DIR;

    if(getenv("PLUGIN_DIR") == NULL){
        PLUGIN_DIR = "./plugins";
    } else {
        PLUGIN_DIR = getenv("PLUGIN_DIR");
        //Will PLUGIN_DIR environmental variable have a / at the end?
    }

    struct dirent* entry;
    vector<Plugin*> plugins;
    //CALL DCLOSE ON CLEANUP
    if(argc == 1){

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
                    if(p->handle == NULL){

                        cerr << "Error: Could not load plugin" << endl;
                        return 1;

                    }
                    *(void**)(&p->get_plugin_name) = dlsym(p->handle, "get_plugin_name");
                    *(void**)(&p->get_plugin_desc) = dlsym(p->handle, "get_plugin_desc");
                    *(void**)(&p->parse_arguments) = dlsym(p->handle, "parse_arguments");
                    *(void**)(&p->transform_image) = dlsym(p->handle, "transform_image");
                    // if(p->get_plugin_name == NULL || p->get_plugin_desc == NULL || p->parse_arguments == NULL || p->transform_image == NULL) {

                    //     cerr << "Error: Could not find required API function" << endl;
                    //     return 1;

                    // }
                    plugins.push_back(p);
                }


            }

            if(strcmp(argv[1], "list") == 0){

                if(argc != 2){

                    cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
                    cerr << "Error: Too many arguments" << endl;
                    return 1;

                }
                cout << "Loaded " << plugins.size() << " plugin(s)" << endl;
                for(int i = 0; i < plugins.size(); i++){

                    cout << " " << (plugins[i]->get_plugin_name()) << ": " << (plugins[i]->get_plugin_desc()) << endl;

                }
            } else if (strcmp(argv[1], "exec") == 0){

                bool found = false;
                Plugin* toExecute = (Plugin*)(malloc(sizeof(Plugin)));
                for(int i = 0; i < plugins.size(); i++){

                    if(strcmp(plugins[i]->get_plugin_name(), argv[2]) == 0){

                        toExecute = plugins[i];
                        found = true;
                        break;

                    }

                }
                //Plugin* toExecute = plugins[2];//DELETE THIS AFTER TESTING
                if(!found){

                    cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
                    cerr << "Error: Could not find plugin" << endl;
                    return 1;

                } else {

                    Image* img = img_read_png(argv[3]);
                    if(img == NULL){

                        cerr << "Error: Cannot ready PNG file" << endl;
                        return 1;

                    } else {
                        Image* transformedImg;
                        void* arguments;
                        if(strcmp(argv[2], "tile") == 0 || strcmp(argv[2], "expose") == 0){
                            //WHY DOES parse_arguments NEED A  CHAR** FOR SECOND PARAM??
                            arguments = toExecute->parse_arguments(1, NULL);
                            transformedImg = toExecute->transform_image(img, arguments);

                        } else {

                            arguments = toExecute->parse_arguments(1, NULL);
                            transformedImg = toExecute->transform_image(img, arguments);
                            if(transformedImg == NULL){

                                cerr << "Error: Could not transform image" << endl;
 
                            }

                        }

                        img_write_png(transformedImg, argv[4]);
                        //WHY DO I HAVE 2 UNACCOUNTED FOR MEMORY ALLOCATIONS!!
                        for(int i = 0; i < plugins.size(); i++){

                            dlclose(plugins[i]->handle);
                            free(plugins[i]);

                        }
                        free(img);
                        free(transformedImg);

                    }

                }

            } else {

                cout << "Usage: imgproc <command> [<command args...>]\nCommands are:\n\tlist\n\texec <plugin> <input img> <output img> [<plugin args...>]" << endl;
                cerr << "Error: Unknown command" << endl;
                return 1;

            }
        }
        closedir(directory);
    }


    return 0;

}
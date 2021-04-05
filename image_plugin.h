//
// This header file defines functions that image plugins must implement.
//

#ifndef IMAGE_PLUGIN_H
#define IMAGE_PLUGIN_H

#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Get the plugin name as a NUL-terminated character string.
//
const char *get_plugin_name(void);

//
// Get a one-sentence description of what the plugin does
// as a NUL-terminated character string.
//
const char *get_plugin_desc(void);

//
// Parse the plugin's command line arguments.
// num_args indicates how many command line arguments are being passed
// to the plugin, args is an array of pointers to the command line
// arguments.
//
// Returns NULL if the command line arguments are invalid; otherwise
// returns a pointer to "argument data" which will be passed to the
// plugin's transform_image function.
//
void *parse_arguments(int num_args, char *args[]);

//
// Transform a source Image.
// The arg_data parameter is the pointer returned by the plugin's
// parse_arguments function.
//
// Returns a pointer to the result Image, or NULL if the plugin
// could not generate a result image for some reason.
//
// This function should free any memory allocated for arg_data
// by the previous call to parse_arguments.
//
struct Image *transform_image(struct Image *source, void *arg_data);

#ifdef __cplusplus
}
#endif

#endif // IMAGE_PLUGIN_H

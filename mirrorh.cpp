/*
 * Image processing with plugins
 * CSF Assignment 4
 * Connor Devlin - cdevlin4@jh.edu
 * Marc Helou - mhelou1@jh.edu
 */

#include <stdlib.h>
#include "image_plugin.h"

struct Arguments {
	// This plugin doesn't accept any command line arguments;
	// just define a single dummy field.
	int dummy;
};

const char *get_plugin_name(void) {
	return "mirrorh";
}

const char *get_plugin_desc(void) {
	return "mirror image horizontally";
}

void *parse_arguments(int num_args, char *args[]) {
	(void) args; // this is just to avoid a warning about an unused parameter

	if (num_args != 0) {
		return NULL;
	}
	return calloc(1, sizeof(struct Arguments));
}

struct Image *transform_image(struct Image *source, void *arg_data) {
	struct Arguments *args = (Arguments*)arg_data;

	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
		free(args);
		return NULL;
	}
    // Transform horizontally
	for (unsigned i = 0; i < source->height; i++) {
        for (unsigned j = 0; j < source-> width / 2; j++) {
            int x = source->width * i + j;
            int c = source->width * i + (source->width - 1 - j);
            out->data[x] = source->data[c];
            out->data[c] = source->data[x]; // Reverses pixel positioning
        }
	}

	free(args);

	return out;
}
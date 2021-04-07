//
// Applies the tile effect to a given png.
//

#include <stdlib.h>
#include "image_plugin.h"
#include <string>

struct Arguments {
	// Specifies the tiling factor.
	int n;
};

const char *get_plugin_name(void) {
	return "tile";
}

const char *get_plugin_desc(void) {
	return "generates an image containing an N x N arrangement of tiles";
}

void *parse_arguments(int num_args, char *args[]) {
    if (num_args != 1 || (atof(args[0]) < 2 || (args[0] == "0" && args[0][0] != '0'))) {
        return NULL;
    }
	struct Arguments *a = (Arguments*)calloc(1, sizeof(Arguments));
    a->n = std::stoi(args[0]);
	return a;
}

struct Image *transform_image(struct Image *source, void *arg_data) {
	struct Arguments *args = (Arguments*)arg_data;

	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
		free(args);
		return NULL;
	}

	

	free(args);

	return out;
}
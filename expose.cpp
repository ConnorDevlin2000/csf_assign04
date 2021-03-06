/*
 * Image processing with plugins
 * CSF Assignment 4
 * Connor Devlin - cdevlin4@jh.edu
 * Marc Helou - mhelou1@jh.edu
 */

#include <stdlib.h>
#include "image_plugin.h"
#include <string.h>

struct Arguments {
	// Factor to expose image by.
	double factor;
};

const char *get_plugin_name(void) {
	return "expose";
}

const char *get_plugin_desc(void) {
	return "adjust the intensity of all pixels";
}

void *parse_arguments(int num_args, char *args[]) {
    if (num_args != 1 || (atof(args[0]) < 0 || (strcmp(args[0], "0.0") == 0 && args[0][0] != '0'))) {
        return NULL;
    }
	struct Arguments *a = (Arguments*)calloc(1, sizeof(Arguments));
    a->factor = atof(args[0]);
	return a;
}

// Helper function to apply the expose effect onto an image.
static uint32_t multiplyFactor(uint32_t pix, Arguments *args) {
	uint8_t r, g, b, a;
	img_unpack_pixel(pix, &r, &g, &b, &a);
	// pixel components are cast to uint32 to avoid overflow
	if (args->factor * (uint32_t)r > 255) {
		r = 255; // maximum value is 255
	} else {
		r *= args->factor; // otherwise, multiply by factor
	}
	if (args->factor * (uint32_t)g > 255) {
		g = 255;
	} else {
		g *= args->factor;
	}
	if (args->factor * (uint32_t)b > 255) {
		b = 255;
	} else {
		b *= args->factor;
	}
	return img_pack_pixel(r, g, b, a); // return packed pixels
}

struct Image *transform_image(struct Image *source, void *arg_data) {
	struct Arguments *args = (Arguments*)arg_data;

	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
		free(args);
		return NULL;
	}

	// Iterate through each pixel, multiplying them by the expose factor
	unsigned num_pixels = source->width * source->height;
	for (unsigned i = 0; i < num_pixels; i++) {
		out->data[i] = multiplyFactor(source->data[i], args);
	}

	free(args);

	return out;
}
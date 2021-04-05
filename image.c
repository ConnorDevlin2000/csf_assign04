//
// Image data type and support for reading/writing PNG files
// See image.h for documentation
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pnglite.h"
#include "image.h"

static int s_pnglite_init;

struct Image *img_create(unsigned width, unsigned height) {
	struct Image *img;

	img = malloc(sizeof(struct Image));
	if (!img) {
		return NULL;
	}

	unsigned num_pixels = width * height;
	img->data = malloc(num_pixels * sizeof(uint32_t));
	if (!img->data) {
		free(img);
		return NULL;
	}

	img->width = width;
	img->height = height;
	for (unsigned i = 0; i < num_pixels; i++) {
		img->data[i] = 0;
	}

	return img;
}

struct Image *img_duplicate(struct Image *img) {
	struct Image *dup = img_create(img->width, img->height);
	if (dup) {
		memcpy(dup->data, img->data, img->width * img->height * sizeof(uint32_t));
	}
	return dup;
}

void img_destroy(struct Image *img) {
	if (img) {
		free(img->data);
		free(img);
	}
}

struct Image *img_read_png(const char *filename) {
	if (!s_pnglite_init) {
		png_init(0, 0);
	}

	png_t png;

	if (png_open_file_read(&png, filename) != PNG_NO_ERROR) {
		return NULL;
	}

	// only allow truecolor images
	if (png.color_type != PNG_TRUECOLOR && png.color_type != PNG_TRUECOLOR_ALPHA) {
		png_close_file(&png);
		return NULL;
	}

	//printf("Image is %u x %u\n", png.width, png.height);
	unsigned num_pixels = png.width * png.height;

	// allocate buffer for pixel data
	unsigned char *pixel_data = malloc(num_pixels * png.bpp);
	if (!pixel_data) {
		png_close_file(&png);
		return NULL;
	}

	// read pixel data
	if (png_get_data(&png, pixel_data) != PNG_NO_ERROR) {
		free(pixel_data);
		png_close_file(&png);
		return NULL;
	}

	// Create Image object
	struct Image *img = img_create(png.width, png.height);
	if (!img) {
		free(pixel_data);
		png_close_file(&png);
		return NULL;
	}

	// Translate pixel data to packed 32-bit RGBA values
	for (unsigned i = 0; i < num_pixels; i++) {
		unsigned offset = (i * png.bpp);
		unsigned r = pixel_data[offset + 0];
		unsigned g = pixel_data[offset + 1];
		unsigned b = pixel_data[offset + 2];
		unsigned a = (png.color_type == PNG_TRUECOLOR_ALPHA)
			? pixel_data[offset + 3]
			: 255;
		img->data[i] = img_pack_pixel(r, g, b, a);
	}

	// Success!
	// Close png file, and de-allocate pixel data buffer
	png_close_file(&png);
	free(pixel_data);

	return img;
}

int img_write_png(struct Image *img, const char *filename) {
	if (!s_pnglite_init) {
		png_init(0, 0);
	}

	// open png file for writing
	png_t png;
	if (png_open_file_write(&png, filename) != PNG_NO_ERROR) {
		return 0;
	}

	// translate packed RBGA data to pixel data
	unsigned num_pixels = img->width * img->height;
	unsigned char *pixel_data = malloc(num_pixels * 4);
	for (unsigned i = 0; i < num_pixels; i++) {
		unsigned offset = i * 4;
		uint32_t rgba = img->data[i];
		uint8_t r, g, b, a;
		img_unpack_pixel(rgba, &r, &g, &b, &a);
		pixel_data[offset + 0] = r;
		pixel_data[offset + 1] = g;
		pixel_data[offset + 2] = b;
		pixel_data[offset + 3] = a;
	}

	// set data
	if (png_set_data(&png, img->width, img->height, 8, PNG_TRUECOLOR_ALPHA, pixel_data) != PNG_NO_ERROR) {
		free(pixel_data);
		png_close_file(&png);
		return 0;
	}

	// Success?
	free(pixel_data);
	png_close_file(&png);
	return 1;
}

void img_unpack_pixel(uint32_t pixel, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
	*r = pixel & 0xFF;
	*g = (pixel >> 8) & 0xFF;
	*b = (pixel >> 16) & 0xFF;
	*a = (pixel >> 24) & 0xFF;
}

uint32_t img_pack_pixel(unsigned r, unsigned g, unsigned b, unsigned a) {
	assert(r < 256);
	assert(g < 256);
	assert(b < 256);
	assert(a < 256);
	return r | (g << 8) | (b << 16) | (a << 24);
}

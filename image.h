//
// Image data type and support for reading/writing PNG files
//

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Image {
	// Image width and height
	unsigned width, height;

	// pixel data in row-major order; pixel representation is:
	//   Bits 0-7 are the red color component value
	//   Bits 8-15 are the green color component value
	//   Bits 16-23 are the blue color component value
	//   Bits 24-31 are the alpha channel value (0=completely transparent, 255=completely opaque)
	uint32_t *data;
};

//
// Create an Image of specified dimensions.
// All red, green, blue, and alpha values are set to 0.
// Returns NULL if the Image can't be created.
//
struct Image *img_create(unsigned width, unsigned height);

//
// Return an exact duplicate of given Image.
//
struct Image *img_duplicate(struct Image *img);

//
// Free the memory of the given Image.
//
void img_destroy(struct Image *img);

//
// Read a PNG file, returning a pointer to an Image
// containing the image data.  Only true-color (24 bit color)
// PNG files are supported.  Returns NULL if the PNG file
// can't be read successfully.
//
struct Image *img_read_png(const char *filename);

//
// Write specified Image to an output PNG file.
// Returns 1 if the PNG file was written successfully, 0 otherwise.
//
int img_write_png(struct Image *img, const char *filename);

//
// Unpack a 32-bit encoded RGBA pixel value as separate red, green, blue, and alpha
// values.  The unpacked values are stored in the variables pointed to by the
// r, g, b, and a parameters.
//
void img_unpack_pixel(uint32_t pixel, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

//
// Pack red, green, blue, and alpha component values into a single
// 32-bit RGBA value.  Each component value must be in the range 0-255.
//
uint32_t img_pack_pixel(unsigned r, unsigned g, unsigned b, unsigned a);

#ifdef __cplusplus
}
#endif

#endif // IMAGE_H

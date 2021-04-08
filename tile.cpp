/*
 * Image processing with plugins
 * CSF Assignment 4
 * Connor Devlin - cdevlin4@jh.edu
 * Marc Helou - mhelou1@jh.edu
 */

#include <stdlib.h>
#include "image_plugin.h"
#include <string.h>
#include <string>
#include <stdint.h>


struct Arguments {
    // Specifies the tiling factor.
    int tiles;
};

const char* get_plugin_name(void) {
    return "tile";
}

const char* get_plugin_desc(void) {
    return "tile source image in an NxN arrangement";
}

void* parse_arguments(int num_args, char* args[]) {
    if (num_args != 1 || (atof(args[0]) < 2 || (strcmp(args[0], "0") == 0 && args[0][0] != '0'))) {
        return NULL;
    }
    struct Arguments* a = (Arguments*)calloc(1, sizeof(Arguments));
    a->tiles = std::stoi(args[0]);
    return a;
}

/* Places all possible tile dimensions in arrays */
void create_tile_arrays(uint32_t* vArray, uint32_t* hArray, uint32_t n, uint32_t extraHeight,
    uint32_t extraWidth, uint32_t tileHeight, uint32_t tileWidth) {
    // calculates tile dimensions based on passed-in modulo of dimension length and tile number
    for (uint32_t i = 0; i < n + 1; i++) {
        if (extraHeight != 0 && i > extraHeight) {
            vArray[i] = (i * tileHeight) - (i - extraHeight);
        }
        else {
            vArray[i] = i * tileHeight;
        }
        if (extraWidth != 0 && i > extraWidth) {
            hArray[i] = (i * tileWidth) - (i - extraWidth);
        }
        else {
            hArray[i] = i * tileWidth;
        }
    }
}

struct Image* transform_image(struct Image* source, void* arg_data) {
    struct Arguments* args = (Arguments*)arg_data;

    // Allocate a result Image
    struct Image* out = img_create(source->width, source->height);
    if (!out) {
        free(args);
        return NULL;
    }

    uint32_t n = args->tiles; // num of tiles
    uint32_t width = source->width; // width of source
    uint32_t height = source->height; // height of source
    uint32_t tileWidth = width / n; // base width of each tile
    uint32_t tileHeight = height / n; // base height of each tile
    uint32_t extraWidth = width % n; // extra pixels for tile widths
    uint32_t extraHeight = height % n; // extra pixels for tile heights

    if (extraHeight != 0) {
        tileHeight++;
    }
    if (extraWidth != 0) {
        tileWidth++;
    }

    uint32_t* hArray = (uint32_t*)calloc((n + 1), sizeof(uint32_t) * (n + 1)); // stores array of horizontal bounds
    uint32_t* vArray = (uint32_t*)calloc((n + 1), sizeof(uint32_t) * (n + 1)); // stores array of vertical bounds
    create_tile_arrays(vArray, hArray, n, extraHeight, extraWidth, tileHeight, tileWidth);

    /* Form smaller tiles and populate new image */
    for (uint32_t y = 0; y < n; y++) {
        for (uint32_t x = 0; x < n; x++) {
            int tempY = 0; // vertical counter for output
            for (uint32_t i = vArray[y]; i < vArray[y + 1]; i++) {
                int tempX = 0; // horizontal counter for output
                for (uint32_t j = hArray[x]; j < hArray[x + 1]; j++) {
                    out->data[i * source->width + j] = source->data[(n * (tempY * source->width)) + (n * tempX)];
                    tempX++;
                }
                tempY++;
            }
        }
    }

    // free all dynamically allocated arrays and structs
    free(vArray);
    free(hArray);
    free(args);
    return out;
}
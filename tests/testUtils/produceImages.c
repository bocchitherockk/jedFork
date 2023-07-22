#include <stdio.h>
#include <stdlib.h>
#include "../../src/jpeg.h"

int main(int argc, char **argv) {
    if(argc != 4) {
        fprintf(stderr, "ERROR: Usage: %s <input.jpg> <output.ppm> <output.bmp>", argv[0]);
        return 1;
    }
    JpegImage img = Jpeg_readImage(argv[1]);
    Block *blocks = Jpeg_getBlocks(&img);
    Jpeg_saveImageAsPpm(&img, blocks, argv[2]);
    Jpeg_saveImageAsBmp(&img, blocks, argv[3]);
    Jpeg_destroyImage(&img);
    free(blocks);
    return 0;
}
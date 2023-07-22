#include <stdio.h>
#include <stdlib.h>
#include "../../src/jpeg.h"

/*
 * return 1 when usage is wrong
 * return 2 when onr or both files not found
 * return 3 when images are different
 * return 0 when images are identical
 */
int main(int argc, char const *argv[]) {

    if(argc != 3) {
        fprintf(stderr, "ERROR: Usage: %s <input.ppm> <input.bmp>\n", argv[0]);
        return 1;
    }

    FILE *f1 = fopen(argv[1], "rb");
    FILE *f2 = fopen(argv[2], "rb");
    if(f1 == NULL) {
        fprintf(stderr, "ERROR: could not open file: %s\n", argv[1]);
        return 2;
    }
    if(f2 == NULL) {
        fprintf(stderr, "ERROR: could not open file: %s\n", argv[2]);
        return 2;
    }

    int ppmWidth, bmpWidth, ppmHeight, bmpHeight;
    Vector ppm, bmp;
    Jpeg_initVector(&ppm);
    Jpeg_initVector(&bmp);


    fgetc(f1);fgetc(f1);fgetc(f1);    // P6\n
    fscanf(f1, "%d %d", &ppmWidth, &ppmHeight);fgetc(f1);   // width height
    fgetc(f1);  // ' '
    fgetc(f1);fgetc(f1);fgetc(f1);fgetc(f1);    // 255\n


    fgetc(f2);fgetc(f2);    // BM
    fgetc(f2);fgetc(f2);fgetc(f2);fgetc(f2);    // size
    fgetc(f2);fgetc(f2);fgetc(f2);fgetc(f2);    // (int)0
    fgetc(f2);fgetc(f2);fgetc(f2);fgetc(f2);    // (int)0xa1
    fgetc(f2);fgetc(f2);fgetc(f2);fgetc(f2);    // (int)12
    bmpWidth = (fgetc(f2) << 0) + (fgetc(f2) << 8);
    bmpHeight = (fgetc(f2) << 0) + (fgetc(f2) << 8);
    fgetc(f2);fgetc(f2);    // (int)1
    fgetc(f2);fgetc(f2);    // (int)24

    if(bmpHeight != ppmHeight || bmpWidth != ppmWidth) {
        return 3;
    }


    for(int i = 0; i < ppmWidth * ppmHeight; i++) {
        byte red   = fgetc(f1);
        byte green = fgetc(f1);
        byte blue  = fgetc(f1);
        Jpeg_appendElementToVector(&ppm, red);
        Jpeg_appendElementToVector(&ppm, green);
        Jpeg_appendElementToVector(&ppm, blue);
        blue   = fgetc(f2);
        green = fgetc(f2);
        red  = fgetc(f2);
        Jpeg_appendElementToVector(&ppm, red);
        Jpeg_appendElementToVector(&ppm, green);
        Jpeg_appendElementToVector(&ppm, blue);
    }

    int i = 0;
    int j = bmp.length - 1 - bmpWidth * 3;
    for(; j >= 0; j -= bmpWidth * 3) {
        for(int k = 0; k < bmpWidth * 3; k++, i++) {
            if(ppm.data[i] != bmp.data[i + j]) {
                printf("different\n");
                return 3;
            }
        }
    }

    return 0;
}
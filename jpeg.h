#pragma once

// #ifndef MATH_H_INCLUDED
//     #error "Please include the math.h header before including jpeg.h"
// #endif


#define FFMARKER 0xFF

// Start of Frame markers, non-differential, Huffman coding
#define SOF0 0xC0 // Baseline DCT
#define SOF1 0xC1 // Extended sequential DCT
#define SOF2 0xC2 // Progressive DCT
#define SOF3 0xC3 // Lossless (sequential)
// Start of Frame markers, differential, Huffman coding
#define SOF5 0xC5 // Differential sequential DCT
#define SOF6 0xC6 // Differential progressive DCT
#define SOF7 0xC7 // Differential lossless (sequential)
// Start of Frame markers, non-differential, arithmetic coding
#define SOF9 0xC9 // Extended sequential DCT
#define SOF10 0xCA // Progressive DCT
#define SOF11 0xCB // Lossless (sequential)
// Start of Frame markers, differential, arithmetic coding
#define SOF13 0xCD // Differential sequential DCT
#define SOF14 0xCE // Differential progressive DCT
#define SOF15 0xCF // Differential lossless (sequential)
// Define Huffman Table(s)
#define DHT 0xC4
// JPEG extensions
#define JPG 0xC8
// Define Arithmetic Coding Conditioning(s)
#define DAC 0xCC
// Restart interval Markers
#define RST0 0xD0
#define RST1 0xD1
#define RST2 0xD2
#define RST3 0xD3
#define RST4 0xD4
#define RST5 0xD5
#define RST6 0xD6
#define RST7 0xD7
// Other Markers
#define SOI 0xD8 // Start of Image
#define EOI 0xD9 // End of Image
#define SOS 0xDA // Start of Scan
#define DQT 0xDB // Define Quantization Table(s)
#define DNL 0xDC // Define Number of Lines
#define DRI 0xDD // Define Restart Interval
#define DHP 0xDE // Define Hierarchical Progression
#define EXP 0xDF // Expand Reference Component(s)
// APPN Markers
#define APP0 0xE0
#define APP1 0xE1
#define APP2 0xE2
#define APP3 0xE3
#define APP4 0xE4
#define APP5 0xE5
#define APP6 0xE6
#define APP7 0xE7
#define APP8 0xE8
#define APP9 0xE9
#define APP10 0xEA
#define APP11 0xEB
#define APP12 0xEC
#define APP13 0xED
#define APP14 0xEE
#define APP15 0xEF
// Misc Markers
#define JPG0 0xF0
#define JPG1 0xF1
#define JPG2 0xF2
#define JPG3 0xF3
#define JPG4 0xF4
#define JPG5 0xF5
#define JPG6 0xF6
#define JPG7 0xF7
#define JPG8 0xF8
#define JPG9 0xF9
#define JPG10 0xFA
#define JPG11 0xFB
#define JPG12 0xFC
#define JPG13 0xFD
#define COM 0xFE
#define TEM 0x01

typedef unsigned char byte;
typedef unsigned int uint;


/************* a simple vector of type byte *************/
typedef struct Vector {
    int maxLength;
    int length;
    byte *data;
} Vector;


typedef struct QuantizationTable {
    byte id;
    byte numberOfBitsPerValue;
    uint data[64];
} QuantizationTable;

extern byte zigzag[64];


typedef struct ColorComponent {
    byte componentId;
    byte horizontalSamplingFactor;
    byte verticalSamplingFactor;
    byte quantizationTableId;
    byte dcHuffmanTableId;
    byte acHuffmanTableId;
} ColorComponent;

typedef struct HuffmanTable {
    byte id;
    byte lengths[16];
    byte *symbols[16];
    uint *codes[16];
} HuffmanTable;

typedef struct MCU {
    union {
        int luminance[64];
        int red[64];
    };
    union {
        int blueChrominance[64];
        int green[64];
    };
    union {
        int redChrominance[64];
        int blue[64];
    };
} MCU;

typedef struct MCUS {
    uint width;     // one mcu is a unit
    uint height;    // one mcu is a unit
    uint realWidth;     // one mcu is a unit
    uint realHeight;    // one mcu is a unit
    byte horizontalSamplingFactor;
    byte verticalSamplingFactor;
    MCU *mcus;
} MCUS;

typedef struct JPEGImage {
    uint quantizationTablesCount;
    QuantizationTable quantizationTables[4];
    byte numberOfBitsPerColorChannel; // aka sample precision which is usually 8
    uint imageWidth;
    uint imageHeight;
    byte colorComponentsCount;
    ColorComponent colorComponents[4];
    uint restartInterval;
    byte dcHuffmanTablesCount;
    HuffmanTable dcHuffmanTables[4];
    byte acHuffmanTablesCount;
    HuffmanTable acHuffmanTables[4];
    byte startOfSelection;
    byte endOfSelection;
    byte successiveApproximationHigh;
    byte successiveApproximationLow;
    char *comment;
    Vector huffmanCodedBitStream;
} JPEGImage;

typedef struct BitReader {
    uint nextBit;
    uint nextByte;
    Vector data;
} BitReader;

// float idctMap[64] = {
//     0.353553,  0.353553,  0.353553,  0.353553,  0.353553,  0.353553,  0.353553,  0.353553,
//     0.490393,  0.415735,  0.277785,  0.097545, -0.097545, -0.277785, -0.415735, -0.490393,
//     0.461940,  0.191342, -0.191342, -0.461940, -0.461940, -0.191342,  0.191342,  0.461940,
//     0.415735, -0.097545, -0.490393, -0.277785,  0.277785,  0.490393,  0.097545, -0.415735,
//     0.353553, -0.353553, -0.353553,  0.353553,  0.353553, -0.353553, -0.353553,  0.353553,
//     0.277785, -0.490393,  0.097545,  0.415735, -0.415735, -0.097545,  0.490393, -0.277785,
//     0.191342, -0.461940,  0.461940, -0.191342, -0.191342,  0.461940, -0.461940,  0.191342,
//     0.097545, -0.277785,  0.415735, -0.490393,  0.490393, -0.415735,  0.277785, -0.097545
// };


JPEGImage loadJpegImage(char *fileName);
void printJpeg(JPEGImage *img);
MCUS getMcus(JPEGImage *img);
void saveJpegAsPpm(JPEGImage *img, MCUS *mcus, char *fileName);
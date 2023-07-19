#ifndef JPEG_H
#define JPEG_H

// #ifndef MATH_H_INCLUDED
//     #error Please include the math.h header before including jpeg.h
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

extern byte zigzag[64];

/************* a simple vector of type byte *************/
typedef struct Vector {
    int maxLength;
    int length;
    byte *data;
} Vector;
void Jpeg_initVector(Vector *v);
void Jpeg_appendElementToVector(Vector *v, byte element);


typedef struct QuantizationTable {
    byte id;
    byte numberOfBitsPerValue;
    uint data[64];
} QuantizationTable;


typedef struct HuffmanTable {
    byte id;
    byte lengths[16];
    byte *symbols[16];
    uint *codes[16];
} HuffmanTable;
int Jpeg_huffmanTableSymbolsCount(HuffmanTable *ht);
void Jpeg_generateHuffmanTableCodes(HuffmanTable *ht);


typedef struct ColorComponent {
    byte componentId;
    byte horizontalSamplingFactor;
    byte verticalSamplingFactor;
    byte quantizationTableId;
    byte dcHuffmanTableId;
    byte acHuffmanTableId;
} ColorComponent;


typedef struct Block {  // 8*8 block of data
    union {int luminance[64];int red[64];};
    union {int blueChrominance[64];int green[64];};
    union {int redChrominance[64];int blue[64];};
} Block;
int *Jpeg_getComponentDataByComponentId(Block *block, int id);


typedef struct BitReader {
    uint nextBit;
    uint nextByte;
    Vector data;
} BitReader;
void Jpeg_initBitReader(BitReader *bitReader, Vector vec);
int Jpeg_getBit(BitReader *bitReader);
int Jpeg_getNBits(BitReader *bitReader, int n);
byte Jpeg_getNextSymbol(BitReader *bitReader, HuffmanTable ht, int *errorFlag);


typedef struct JpegImage {
    byte quantizationTablesCount;
    QuantizationTable quantizationTables[4];
    byte numberOfBitsPerColorChannel; // aka sample precision which is usually 8
    uint imageWidth;
    uint imageHeight;
    byte colorComponentsCount;
    ColorComponent colorComponents[3];
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

    uint blocksHeight;
    uint blocksWidth;
    uint blocksHeightReal;
    uint blocksWidthReal;

    byte horizontalSamplingFactor;
    byte verticalSamplingFactor;
} JpegImage;
void Jpeg_initImage(JpegImage *img);
void Jpeg_destroyImage(JpegImage *img);


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

HuffmanTable Jpeg_getDCHuffmanTableById(JpegImage *img, int dcHuffmanTableId);
HuffmanTable Jpeg_getACHuffmanTableById(JpegImage *img, int acHuffmanTableId);
QuantizationTable Jpeg_getQuantizationTableById(JpegImage *img, int quantizationTableId);
ColorComponent Jpeg_getColorComponentById(JpegImage *img, int componentId);
void Jpeg_generateHuffmanTablesCodes(JpegImage *img);

void Jpeg_decodeBlockComponent(BitReader *bitReader, int* component, int *previousDC, HuffmanTable dcTable, HuffmanTable acTable);
Block* Jpeg_decodeBlocks(JpegImage* img, Block *blocks);
void Jpeg_dequantizeBlockComponent(QuantizationTable qTable, int *component);
void Jpeg_dequantizeBlocks(JpegImage *img, Block *blocks);
void Jpeg_inverseDctBlockComponent(int *component);
void Jpeg_inverseDctBlocks(JpegImage *img, Block *blocks);
void Jpeg_YCbCrToRGBBlock(JpegImage *img, Block *luminanceBlock, Block *cbcrBlock, int v, int h);
void Jpeg_YCbCrToRGBBlocks(JpegImage *img, Block *blocks);
Block *Jpeg_getBlocks(JpegImage *img);

JpegImage Jpeg_readImage(char *fileName);
void Jpeg_printImage(JpegImage *img);
void Jpeg_saveImageAsPpm(JpegImage *img, Block *blocks, char *fileName);
void Jpeg_saveImageAsBmp(JpegImage *img, Block *blocks, char *filename);

#endif

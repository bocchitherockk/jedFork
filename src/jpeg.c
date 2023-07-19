#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jpeg.h"

// IDCT scaling factor
float m0 = 2.0 * cos(1.0 / 16.0 * 2.0 * M_PI);
float m1 = 2.0 * cos(2.0 / 16.0 * 2.0 * M_PI);
float m3 = 2.0 * cos(2.0 / 16.0 * 2.0 * M_PI);
float m5 = 2.0 * cos(3.0 / 16.0 * 2.0 * M_PI);
float m2 = 2.0 * cos(1.0 / 16.0 * 2.0 * M_PI) - 2.0 * cos(3.0 / 16.0 * 2.0 * M_PI);
float m4 = 2.0 * cos(1.0 / 16.0 * 2.0 * M_PI) + 2.0 * cos(3.0 / 16.0 * 2.0 * M_PI);

float s0 = cos(0.0 / 16.0 * M_PI) / sqrt(8);
float s1 = cos(1.0 / 16.0 * M_PI) / 2.0;
float s2 = cos(2.0 / 16.0 * M_PI) / 2.0;
float s3 = cos(3.0 / 16.0 * M_PI) / 2.0;
float s4 = cos(4.0 / 16.0 * M_PI) / 2.0;
float s5 = cos(5.0 / 16.0 * M_PI) / 2.0;
float s6 = cos(6.0 / 16.0 * M_PI) / 2.0;
float s7 = cos(7.0 / 16.0 * M_PI) / 2.0;

byte zigzag[64] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/************* helper functions *************/
/*
 * print binary format of a number to a certain length
 * decToBinary(7, 2) -> ERROR
 * decToBinary(7, 3) -> 111
 * decToBinary(7, 4) -> 0111
 */
void decToBinary(int number, int length) {
    if(number >= (1 << length)) { // (1 << length) = pow(2, length)
        fprintf(stderr, "length %d is not enough to convert %d to binary\n", length, number);
        exit(1);
    }
    int binaryNum[32];
  
    int i = 0;
    while (number > 0) {
        binaryNum[i] = number % 2;
        number = number / 2;
        i++;
    }

    for(int j = length - 1; j > i - 1; j--) {
        printf("0");
    }
    for (int j = i - 1; j >= 0; j--) {
        printf("%d", binaryNum[j]);
    }
}

/*
 * return the dc huffman table with matching id
 */
HuffmanTable Jpeg_getDCHuffmanTableById(JpegImage *img, int dcHuffmanTableId) {
    for(int i = 0; i < img->dcHuffmanTablesCount; i++) {
        if(img->dcHuffmanTables[i].id == dcHuffmanTableId) {
            return img->dcHuffmanTables[i];
        }
    }
}

/*
 * return the ac huffman table with matching id
 */
HuffmanTable Jpeg_getACHuffmanTableById(JpegImage *img, int acHuffmanTableId) {
    for(int i = 0; i < img->acHuffmanTablesCount; i++) {
        if(img->acHuffmanTables[i].id == acHuffmanTableId) {
            return img->acHuffmanTables[i];
        }
    }
}

/*
 * return the quantization table with matching id
 */
QuantizationTable Jpeg_getQuantizationTableById(JpegImage *img, int quantizationTableId) {
    for(int i = 0; i < img->quantizationTablesCount; i++) {
        if(img->quantizationTables[i].id == quantizationTableId) {
            return img->quantizationTables[i];
        }
    }
}

/*
 * return the color component with matching id
 */
ColorComponent Jpeg_getColorComponentById(JpegImage *img, int componentId) {
    for(int i = 0; i < img->colorComponentsCount; i++) {
        if(img->colorComponents[i].componentId == componentId) {
            return img->colorComponents[i];
        }
    }
}

/*
 * return the color component with matching id
 * id == 1 -> luminance
 * id == 2 -> blue chrominance
 * id == 3 -> red chrominance
 */
int *Jpeg_getComponentDataByComponentId(Block *block, int id) {
    if(id == 1) {
        return block->luminance;
    } else if(id == 2) {
        return block->blueChrominance;
    } else if(id == 3) {
        return block->redChrominance;
    }
}


void Jpeg_initVector(Vector *v) {
    v->maxLength = 10;
    v->length = 0;
    v->data = malloc(sizeof(byte) * v->maxLength);
}

void Jpeg_appendElementToVector(Vector *v, byte element) {
    if(v->length == v->maxLength) {
        v->maxLength += 10;
        v->data = realloc(v->data, sizeof(byte) * v->maxLength);
    }
    v->data[v->length] = element;
    v->length++;
}

/*
 * return the total number of symbols for a huffman table
 */
int Jpeg_huffmanTableSymbolsCount(HuffmanTable *ht) {
    int result = 0;
    for(int i = 0; i < 16; i++) {
        result += ht->lengths[i];
    }
    return result;
}


void Jpeg_initImage(JpegImage *img) {
    img->quantizationTablesCount = 0;
    img->numberOfBitsPerColorChannel = 0;
    img->imageWidth = 0;
    img->imageHeight = 0;
    img->colorComponentsCount = 0;
    img->restartInterval = 0;
    img->dcHuffmanTablesCount = 0;
    img->acHuffmanTablesCount = 0;
    img->comment = NULL;
    Jpeg_initVector(&img->huffmanCodedBitStream);
}

void Jpeg_destroyImage(JpegImage *img) {
    for(int i = 0; i < img->dcHuffmanTablesCount; i++) {
        for(int j = 0; j < 16; j++) {
            free(img->dcHuffmanTables[i].symbols[j]);
            free(img->dcHuffmanTables[i].codes[j]);
        }
    }
    for(int i = 0; i < img->acHuffmanTablesCount; i++) {
        for(int j = 0; j < 16; j++) {
            free(img->acHuffmanTables[i].symbols[j]);
            free(img->acHuffmanTables[i].codes[j]);
        }
    }
    free(img->comment);
}


void Jpeg_initBitReader(BitReader *bitReader, Vector vec) {
    bitReader->data = vec;
    bitReader->nextBit = 0;
    bitReader->nextByte = 0;
}

int Jpeg_getBit(BitReader *bitReader) {
    if(bitReader->nextByte >= bitReader->data.length) {
        return -1;
    }
    int bit = (bitReader->data.data[bitReader->nextByte] >> (7 - bitReader->nextBit)) & 1;
    bitReader->nextBit++;
    if(bitReader->nextBit == 8) {
        bitReader->nextBit = 0;
        bitReader->nextByte++;
    }
    return bit;
}

int Jpeg_getNBits(BitReader *bitReader, int n) {
    int bits = 0;
    for(int i = 0; i < n; i++) {
        int bit = Jpeg_getBit(bitReader);
        if(bit == -1) {
            return -1;
        }
        bits = (bits << 1) | bit;
    }
    return bits;
}

byte Jpeg_getNextSymbol(BitReader *bitReader, HuffmanTable ht, int *errorFlag) {
    uint currentCode = 0;
    for(int i = 0; i < 16; i++) {
        int bit = Jpeg_getBit(bitReader);
        if(bit == -1) {
            *errorFlag = -1;
            return -1;
        }
        currentCode = (currentCode << 1) | bit;
        for(int j = 0; j < ht.lengths[i]; j++) {
            if(currentCode == ht.codes[i][j]) {
                return ht.symbols[i][j];
            }
        }
    }
    *errorFlag = -1;
    return -1;
}

void Jpeg_printImage(JpegImage *img) {

    printf("=============DQT=============\n");
    for(int i = 0; i < img->quantizationTablesCount; i++) {
        printf("id: %d\n", img->quantizationTables[i].id);
        printf("number of bits per value: %d\n", img->quantizationTables[i].numberOfBitsPerValue);
        for(int y = 0; y < 8; y++) {
            for(int x = 0; x < 8; x++) {
                printf("%d ", img->quantizationTables[i].data[y*8 + x]);
            }
            printf("\n");
        }
    }

    printf("=============SOF=============\n");
    printf("width: %d\n", img->imageWidth);
    printf("heigth: %d\n", img->imageHeight);
    printf("number of bits per color channel: %d\n", img->numberOfBitsPerColorChannel);
    printf("number of color components: %d\n", img->colorComponentsCount);

    printf("=============DRI=============\n");
    printf("restart interval: %d\n", img->restartInterval);

    printf("=============DHT=============\n");
    printf("number of dc Huffman Tables: %d\n", img->dcHuffmanTablesCount);
    for(int i = 0; i < img->dcHuffmanTablesCount; i++) {
        printf("id: %d\n", img->dcHuffmanTables[i].id);
        for(int j = 0; j < 16; j++) {
            printf("codes of length %d\n    number of codes: %d\n", j + 1, img->dcHuffmanTables[i].lengths[j]);
            for(int k = 0; k < img->dcHuffmanTables[i].lengths[j]; k++) {
                printf("    symbol: %02x   code: ", img->dcHuffmanTables[i].symbols[j][k]);
                decToBinary(img->dcHuffmanTables[i].codes[j][k], j + 1);
                printf("\n");
            }
        }
    }

    printf("number of ac Huffman Tables: %d\n", img->acHuffmanTablesCount);
    for(int i = 0; i < img->acHuffmanTablesCount; i++) {
        printf("id: %d\n", img->acHuffmanTables[i].id);
        for(int j = 0; j < 16; j++) {
            printf("codes of length %d\n    number of codes: %d\n", j + 1, img->acHuffmanTables[i].lengths[j]);
            for(int k = 0; k < img->acHuffmanTables[i].lengths[j]; k++) {
                printf("    symbol: %02x   code: ", img->acHuffmanTables[i].symbols[j][k]);
                decToBinary(img->acHuffmanTables[i].codes[j][k], j + 1);
                printf("\n");
            }
        }
    }

    if(img->comment != NULL) {
        printf("=============COM=============\n");
        printf("%s\n", img->comment);
    }

    printf("=============SOS=============\n");
    printf("start of selection: %d\n", img->startOfSelection);
    printf("end of selection: %d\n", img->endOfSelection);
    printf("successive approximation high: %d\n", img->successiveApproximationHigh);
    printf("successive approximation low: %d\n", img->successiveApproximationLow);
    printf("color components:\n");
    for(int i = 0; i < img->colorComponentsCount; i++) {
        ColorComponent aux = img->colorComponents[i];
        printf("component id: %d\n", aux.componentId);
        printf("dc huffman table id: %d\n", aux.dcHuffmanTableId);
        printf("ac huffman table id: %d\n", aux.acHuffmanTableId);
        printf("related quantization table id: %d\n", aux.quantizationTableId);
        printf("vertical sampling factor: %d\n", aux.verticalSamplingFactor);
        printf("horizontal sampling factor: %d\n", aux.horizontalSamplingFactor);
    }
    printf("huffman coded bitStream:\n");
    printf("length: %d\n", img->huffmanCodedBitStream.length);
/*    printf("bytes:\n");
    for(int i = 0; i < img->huffmanCodedBitStream.length; i++) {
        printf("%02x ", img->huffmanCodedBitStream.data[i]);
    }
    fflush(stdout);
    printf("\n");*/
/*    printf("binary:\n");
    for(int i = 0; i < img->huffmanCodedBitStream.length; i++) {
        decToBinary(img->huffmanCodedBitStream.data[i], 8);
    }
    printf("\n");
    fflush(stdout);*/
}

void Jpeg_readAPPNSegments(FILE *f) { 
    int length = (fgetc(f) << 8) + fgetc(f);
    for(int i = 0; i < length - 2; i++) {
        fgetc(f);
    }
}

void Jpeg_readQuantizationTablesSegment(FILE *f, JpegImage *img) {
    int length = (fgetc(f) << 8) + fgetc(f);
    length -= 2;
    while(length > 0) {
        byte tableInfo = fgetc(f);
        length--;
        byte id = tableInfo & 0xf;
        if(id > 3) {
            fprintf(stderr, "ERROR: invalid quantization table id: %d\n", id);
            fclose(f);
            exit(1);
        }
        img->quantizationTables[img->quantizationTablesCount].id = id;

        byte numberOfBitsPerValue = tableInfo >> 4;
        if(numberOfBitsPerValue == 0) {
            img->quantizationTables[img->quantizationTablesCount].numberOfBitsPerValue = 8;
        } else if(numberOfBitsPerValue == 1) {
            img->quantizationTables[img->quantizationTablesCount].numberOfBitsPerValue = 16;
        } else {
            fprintf(stderr, "ERROR: invalid number of bits per quantization value\n");
            fclose(f);
            exit(1);
        }

        for(int y = 0; y < 8; y++) {
            for(int x = 0; x < 8; x++) {
                byte firstByte = fgetc(f);
                uint data = firstByte;
                if(numberOfBitsPerValue == 1) {
                    byte secondByte = fgetc(f);
                    data = (data << 8) + secondByte;
                }
                img->quantizationTables[img->quantizationTablesCount].data[zigzag[y * 8 + x]] = data;
            }
        }
        length -= 64 * numberOfBitsPerValue + 64;
        img->quantizationTablesCount++;
    }

    if(length < 0) {
        fprintf(stderr, "ERROR: DQT marker length is incorrect\n");
        fclose(f);
        exit(1);
    }
}

void Jpeg_readStartOfFrameSegment(FILE *f, JpegImage *img) {

    if(img->colorComponentsCount != 0) {
        fprintf(stderr, "ERROR: multiple SOF segments detected\n");
        fclose(f);
        exit(1);
    }

    int length = (fgetc(f) << 8) + fgetc(f);

    img->numberOfBitsPerColorChannel = fgetc(f);
    if(img->numberOfBitsPerColorChannel != 8) {
        fprintf(stderr, "ERROR: invalid or unsupported precision encountered: %d\n", img->numberOfBitsPerColorChannel);
        fclose(f);
        exit(1);
    }

    img->imageHeight = (fgetc(f) << 8) + fgetc(f);
    img->imageWidth = (fgetc(f) << 8) + fgetc(f);
    if(img->imageHeight == 0 || img->imageWidth == 0) {
        fprintf(stderr, "ERROR: invalid dimentions encountered (width: %d, height: %d)\n", img->imageWidth, img->imageHeight);
        fclose(f);
        exit(1);
    }

    img->blocksHeight = (img->imageHeight + 7) / 8;
    img->blocksWidth = (img->imageWidth + 7) / 8;
    img->blocksHeightReal = img->blocksHeight;
    img->blocksWidthReal = img->blocksWidth;

    img->colorComponentsCount = fgetc(f);
    if(img->colorComponentsCount == 4) {
        fprintf(stderr, "ERROR: CMYK color mode is not supported\n");
        fclose(f);
        exit(1);
    }
    if(img->colorComponentsCount == 0) {
        fprintf(stderr, "ERROR: number of components should not be 0\n");
        fclose(f);
        exit(1);
    }

    for(int i = 0; i < img->colorComponentsCount; i++) {
        byte ComponentId = fgetc(f);
        if(ComponentId == 4 || ComponentId == 5) {
            fprintf(stderr, "ERROR: YIQ color mode is not supported\n");
            fclose(f);
            exit(1);
        }
        if(ComponentId > 3) {
            fprintf(stderr, "ERROR: invalid component id: %d\n", ComponentId);
            fclose(f);
            exit(1);
        }
        img->colorComponents[i].componentId = ComponentId;

        byte samplingFactor = fgetc(f);
        img->colorComponents[i].verticalSamplingFactor = samplingFactor & 0xf;
        img->colorComponents[i].horizontalSamplingFactor = samplingFactor >> 4;
        if(img->colorComponents[i].componentId == 1) {
            if(samplingFactor != 0x11 && samplingFactor != 0x12 && samplingFactor != 0x21 && samplingFactor != 0x22) {
                fprintf(stderr, "ERROR: sampling factor other than one is unsupported, vertical: %d, horizontal: %d\n", samplingFactor & 0xf, samplingFactor >> 4);
                fclose(f);
                exit(1);
            }
            if (img->colorComponents[i].horizontalSamplingFactor == 2 && img->blocksWidth % 2 == 1) {
                img->blocksWidthReal += 1;
            }
            if (img->colorComponents[i].verticalSamplingFactor == 2 && img->blocksHeight % 2 == 1) {
                img->blocksHeightReal += 1;
            }
            img->horizontalSamplingFactor = img->colorComponents[i].horizontalSamplingFactor;
            img->verticalSamplingFactor = img->colorComponents[i].verticalSamplingFactor;
        } else {
            if(samplingFactor != 0x11) {
                fprintf(stderr, "ERROR: sampling factors are unsupported, vertical: %d, horizontal: %d\n", samplingFactor & 0xf, samplingFactor >> 4);
                fclose(f);
                exit(1);
            }
        }


        byte quantizationTableId = fgetc(f);
        img->colorComponents[i].quantizationTableId = quantizationTableId;
        if(quantizationTableId > 3) {
            fprintf(stderr, "ERROR: invalid quantization table id (%d)assigned to component %d\n", quantizationTableId, ComponentId);
            fclose(f);
            exit(1);
        }
    }

    if(length - 8 - 3 * img->colorComponentsCount != 0) {
        fprintf(stderr, "ERROR: invalid length of SOF marker\n");
        fclose(f);
        exit(1);
    }

    // the component id 1 means it is luminance component, 2 is Cb, 3 is Cr
    // but in some images you will find 0 for chrominance, 1 for Cb, 2 for Cr
    // so i'll take care of that to be all standard 
    if(img->colorComponents[0].componentId == 0) {
        img->colorComponents[0].componentId++;
        img->colorComponents[1].componentId++;
        img->colorComponents[2].componentId++;
    }
}

void Jpeg_readRestartIntervalSegment(FILE *f, JpegImage *img) {

    int length = (fgetc(f) << 8) + fgetc(f);
    img->restartInterval = (fgetc(f) << 8) + fgetc(f);
    if(length - 4 != 0) {
        fprintf(stderr, "ERROR: invalid DRI(define restrat interval)");
        fclose(f);
        exit(1);
    }
}

void Jpeg_readHuffmanTablesSegment(FILE *f, JpegImage *img) {

    int length = (fgetc(f) << 8) + fgetc(f);
    length -= 2;
    while(length > 0) {
        byte tableInfo = fgetc(f);
        length--;

        byte id = tableInfo & 0xf;
        if(id > 3) {
            fprintf(stderr, "ERROR: invalid huffman table id: %d\n", id);
            fclose(f);
            exit(1);
        }

        byte type = tableInfo >> 4;
        HuffmanTable *ht = &img->dcHuffmanTables[img->dcHuffmanTablesCount];
        if(type == 1) {
            ht = &img->acHuffmanTables[img->acHuffmanTablesCount];
        }
        ht->id = id;

        // read the lengths, allocate memory for symbols and codes
        for(int i = 0; i < 16; i++) {
            ht->lengths[i] = fgetc(f);
            if(ht->lengths[i] == 0) {
                ht->symbols[i] = NULL;
                ht->codes[i] = NULL;
            } else {
                ht->symbols[i] = malloc(sizeof(byte) * ht->lengths[i]);
                ht->codes[i] = malloc(sizeof(uint) * ht->lengths[i]);
            }
        }
        length -= 16;

        // read the symbols
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < ht->lengths[i]; j++) {
                ht->symbols[i][j] = fgetc(f);
            }
        }
        length -= Jpeg_huffmanTableSymbolsCount(ht);

        if(type == 0) {
            img->dcHuffmanTablesCount++;
        } else {
            img->acHuffmanTablesCount++;
        }
    }

    if(length != 0) {
        fprintf(stderr, "ERROR: invalid length of DHT(define huffman table)\n");
        fclose(f);
        exit(1);
    }
}

void Jpeg_readStartOfScanSegment(FILE *f, JpegImage *img) {

    if(img->colorComponentsCount == 0) {
        fprintf(stderr, "ERROR: encountered SOS(start of scan) before SOF(start of frame)\n");
        fclose(f);
        exit(1);
    }

    uint length = (fgetc(f) << 8) + fgetc(f);

    byte componentsCount = fgetc(f);
    if(componentsCount != img->colorComponentsCount) {
        fprintf(stderr, "ERROR: number of components indicated in SOS does not match that in SOF\n");
        fclose(f);
        exit(1);
    }

    for(int i = 0; i < componentsCount; i++) {
        byte componentId = fgetc(f);
        if(img->colorComponents[i].componentId != componentId) {
            fprintf(stderr, "ERROR: component id indicated in SOS does not match that in SOF\n");
            fclose(f);
            exit(1);
        }

        byte huffmanTableIds = fgetc(f);
        img->colorComponents[i].dcHuffmanTableId = huffmanTableIds >> 4;
        img->colorComponents[i].acHuffmanTableId = huffmanTableIds & 0xf;
    }

    img->startOfSelection = fgetc(f);
    img->endOfSelection = fgetc(f);
    if(img->startOfSelection != 0 || img->endOfSelection != 63) {
        fprintf(stderr, "ERROR: invalid spectral selection\n");
        fclose(f);
        exit(1);
    }
    byte successiveApproximation = fgetc(f);
    if(successiveApproximation != 0x00) {
        fprintf(stderr, "ERROR: invalid successive approximation\n");
        fclose(f);
        exit(1);
    }
    img->successiveApproximationHigh = successiveApproximation >> 4;
    img->successiveApproximationLow = successiveApproximation & 0xf;

    if(length - 6 - 2 * componentsCount != 0) {
        fprintf(stderr, "ERROR: invalid length of SOS\n");
        fclose(f);
        exit(1);
    }
}

void Jpeg_readCommentSegment(FILE *f, JpegImage *img) {

    uint length = (fgetc(f) << 8) + fgetc(f);
    length -= 2;
    img->comment = malloc((length + 1) * sizeof(char));
    for(int i = 0; i < length; i++) {
        img->comment[i] = fgetc(f);
    }
    img->comment[length] = '\0';
}

void Jpeg_readJpegSegments(FILE *f, JpegImage *img) {
    byte previous = fgetc(f);
    byte current = fgetc(f);

    // reading start of image marker
    if(previous != FFMARKER || current != SOI) {
        fprintf(stderr, "ERROR: could not read 0xffd8 marker(start of image (SOI)), file is not a valid jpeg image\n");
        fclose(f);
        exit(1);
    }

    do {
        previous = fgetc(f);
        current = fgetc(f);
        if(previous != FFMARKER) {
            fprintf(stderr, "ERROR: expected a 0xff marker\n");
            fclose(f);
            exit(1);
        }

        if(current >= APP0 && current <= APP15) {
            Jpeg_readAPPNSegments(f);
        } else if(current == DQT) {
            Jpeg_readQuantizationTablesSegment(f, img);
        } else if(current == SOF0) {
            Jpeg_readStartOfFrameSegment(f, img);
        } else if(current == DRI) {
            Jpeg_readRestartIntervalSegment(f, img);
        } else if(current == DHT) {
            Jpeg_readHuffmanTablesSegment(f, img);
        } else if(current == SOS) {
            Jpeg_readStartOfScanSegment(f, img);
            break;
        } else if(current == COM) {
            Jpeg_readCommentSegment(f, img);
        } else if(current == 0xff) {
            current = fgetc(f);
            continue;
        } else if(current == SOI) {
            fprintf(stderr, "ERROR: embedded jpegs is unsupported\n");
            fclose(f);
            exit(1);
        } else if(current == EOI) {
            fprintf(stderr, "ERROR: EOI detected before SOS\n");
            fclose(f);
            exit(1);
        } else if(current == DAC) {
            fprintf(stderr, "ERROR: arithmetic coding mode is unsupported\n");
            fclose(f);
            exit(1);
        } else if(current >= SOF1 && current <= SOF15) {
            // Jpeg_readStartOfFrameSegment(f, img);
            fprintf(stderr, "ERROR: SOF marker is unsupported\n");
            fclose(f);
            exit(1);
        } else if(current >= RST0 && current <= RST7) {
            fprintf(stderr, "ERROR: RSTN detected before SOS\n");
            fclose(f);
            exit(1);
        } else {
            fprintf(stderr, "ERROR: unknow marker: 0x%x\n", current);
            fclose(f);
            exit(1);
        }
    } while(current != SOS);
}

void Jpeg_readHuffmanCodedBitStream(FILE *f, JpegImage *img) {
    byte previous, current = fgetc(f);
    while(1) {
        previous = current;
        current = fgetc(f);

        if(previous == FFMARKER) {
            if(current == EOI) {
                break;
            } else if(current == 0x00) {
                Jpeg_appendElementToVector(&img->huffmanCodedBitStream, previous);
                current = fgetc(f);
            } else if(current >= RST0 && current <= RST7) {
                current = fgetc(f);
            } else if(current == FFMARKER) {
                continue;
            } else {
                fprintf(stderr, "ERROR: invalid marker encountered: %02x%02x\n", previous, current);
                fclose(f);
                exit(1);
            }
        } else {
            Jpeg_appendElementToVector(&img->huffmanCodedBitStream, previous);
        }
    }
}


void Jpeg_generateHuffmanTableCodes(HuffmanTable *ht) {

    uint code = 0;
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < ht->lengths[i]; j++) {
            ht->codes[i][j] = code;
            code++;
        }
        code <<= 1;
    }
}

void Jpeg_generateHuffmanTablesCodes(JpegImage *img) {
    for(int i = 0; i < img->dcHuffmanTablesCount; i++) {
        Jpeg_generateHuffmanTableCodes(&img->dcHuffmanTables[i]);
    }
    for(int i = 0; i < img->acHuffmanTablesCount; i++) {
        Jpeg_generateHuffmanTableCodes(&img->acHuffmanTables[i]);
    }
}

// fill the coefficients of an MCU component based on Huffman codes
//   read from the BitReader
void Jpeg_decodeBlockComponent(BitReader *bitReader, int* component, int *previousDC, HuffmanTable dcTable, HuffmanTable acTable) {
    int errorFlag = 0;
    // get the DC value for this MCU component
    byte length = Jpeg_getNextSymbol(bitReader, dcTable, &errorFlag);
    if (errorFlag == -1) {
        fprintf(stderr, "ERROR: Invalid DC value\n");
        exit(1);
    }
    if (length > 11) {
        fprintf(stderr, "ERROR: DC coefficient length greater than 11\n");
        exit(1);
    }
    int coeff = Jpeg_getNBits(bitReader, length);
    if (coeff == -1) {
        fprintf(stderr, "ERROR: Invalid DC value\n");
        exit(1);
    }
    if (length != 0 && coeff < (1 << (length - 1))) {
        coeff -= (1 << length) - 1;
    }
    component[0] = coeff + *previousDC;
    *previousDC = component[0];
    // get the AC values for this MCU component
    uint i = 1;
    while (i < 64) {
        byte symbol = Jpeg_getNextSymbol(bitReader, acTable, &errorFlag);
        if (errorFlag == -1) {
            fprintf(stderr, "ERROR: Invalid AC value\n");
            exit(1);
        }
        // symbol 0x00 means fill remainder of component with 0
        if (symbol == 0x00) {
            for (; i < 64; ++i) {
                component[zigzag[i]] = 0;
            }
            return;
        }
        // otherwise, read next component coefficient
        byte numZeroes = symbol >> 4;
        byte coeffLength = symbol & 0x0F;
        coeff = 0;
        // symbol 0xF0 means skip 16 0's
        if (symbol == 0xF0) {
            numZeroes = 16;
        }
        if (i + numZeroes >= 64) {
            fprintf(stderr, "ERROR: Zero run-length exceeded MCU\n");
            exit(1);
        }
        for (uint j = 0; j < numZeroes; ++j, ++i) {
            component[zigzag[i]] = 0;
        }
        if (coeffLength > 10) {
            fprintf(stderr, "ERROR: AC coefficient length greater than 10\n");
            exit(1);
        }
        if (coeffLength != 0) {
            coeff = Jpeg_getNBits(bitReader, coeffLength);
            if (coeff == -1) {
                fprintf(stderr, "ERROR: Invalid AC value\n");
                exit(1);
            }
            if (coeff < (1 << (coeffLength - 1))) {
                coeff -= (1 << coeffLength) - 1;
            }
            component[zigzag[i]] = coeff;
            i += 1;
        }
    }
}

// decode all the Huffman data and fill all MCUs
Block* Jpeg_decodeBlocks(JpegImage* img, Block *blocks) {
    BitReader BitReader;
    Jpeg_initBitReader(&BitReader, img->huffmanCodedBitStream);
    int previousDCs[3] = { 0 };
    uint restartInterval = img->restartInterval * img->horizontalSamplingFactor * img->verticalSamplingFactor;

    for (uint y = 0; y < img->blocksHeight; y += img->verticalSamplingFactor) {
        for (uint x = 0; x < img->blocksWidth; x += img->horizontalSamplingFactor) {
            if (restartInterval != 0 && (y * img->blocksWidthReal + x) % restartInterval == 0) {
                previousDCs[0] = 0;
                previousDCs[1] = 0;
                previousDCs[2] = 0;
                if(BitReader.nextByte < BitReader.data.length && BitReader.nextBit != 0) {
                    BitReader.nextBit = 0;
                    BitReader.nextByte++;
                }
            }
            for (uint i = 0; i < img->colorComponentsCount; ++i) {
                HuffmanTable selectedDcHuffmanTable = Jpeg_getDCHuffmanTableById(img, img->colorComponents[i].dcHuffmanTableId);
                HuffmanTable selectedAcHuffmanTable = Jpeg_getACHuffmanTableById(img, img->colorComponents[i].acHuffmanTableId);
                for (uint v = 0; v < img->colorComponents[i].verticalSamplingFactor; ++v) {
                    for (uint h = 0; h < img->colorComponents[i].horizontalSamplingFactor; ++h) {
                        int *selectedColorComponent = Jpeg_getComponentDataByComponentId(&blocks[(y + v) * img->blocksWidthReal + (x + h)], img->colorComponents[i].componentId);
                        Jpeg_decodeBlockComponent(&BitReader, selectedColorComponent, &previousDCs[i], selectedDcHuffmanTable, selectedAcHuffmanTable);
                    }
                }
            }
        }
    }
    return blocks;
}

// dequantize an MCU component based on a quantization table
void Jpeg_dequantizeBlockComponent(QuantizationTable qTable, int *component) {
    for (uint i = 0; i < 64; ++i) {
        component[i] *= qTable.data[i];
    }
}

// dequantize all MCUs
void Jpeg_dequantizeBlocks(JpegImage *img, Block *blocks) {
    for (uint y = 0; y < img->blocksHeight; y += img->verticalSamplingFactor) {
        for (uint x = 0; x < img->blocksWidth; x += img->horizontalSamplingFactor) {
            for (uint i = 0; i < img->colorComponentsCount; ++i) {
                QuantizationTable selectedQuantizationTable = Jpeg_getQuantizationTableById(img, img->colorComponents[i].quantizationTableId);
                for (uint v = 0; v < img->colorComponents[i].verticalSamplingFactor; ++v) {
                    for (uint h = 0; h < img->colorComponents[i].horizontalSamplingFactor; ++h) {
                        int *selectedColorComponent = Jpeg_getComponentDataByComponentId(&blocks[(y + v) * img->blocksWidthReal + (x + h)], img->colorComponents[i].componentId);
                        Jpeg_dequantizeBlockComponent(selectedQuantizationTable, selectedColorComponent);
                    }
                }
            }
        }
    }
}

// perform 1-D IDCT on all columns and rows of an MCU component
//   resulting in 2-D IDCT
void Jpeg_inverseDctBlockComponent(int *component) {
    for (uint i = 0; i < 8; ++i) {
        float g0 = component[0 * 8 + i] * s0;
        float g1 = component[4 * 8 + i] * s4;
        float g2 = component[2 * 8 + i] * s2;
        float g3 = component[6 * 8 + i] * s6;
        float g4 = component[5 * 8 + i] * s5;
        float g5 = component[1 * 8 + i] * s1;
        float g6 = component[7 * 8 + i] * s7;
        float g7 = component[3 * 8 + i] * s3;
        float f0 = g0;
        float f1 = g1;
        float f2 = g2;
        float f3 = g3;
        float f4 = g4 - g7;
        float f5 = g5 + g6;
        float f6 = g5 - g6;
        float f7 = g4 + g7;
        float e0 = f0;
        float e1 = f1;
        float e2 = f2 - f3;
        float e3 = f2 + f3;
        float e4 = f4;
        float e5 = f5 - f7;
        float e6 = f6;
        float e7 = f5 + f7;
        float e8 = f4 + f6;
        float d0 = e0;
        float d1 = e1;
        float d2 = e2 * m1;
        float d3 = e3;
        float d4 = e4 * m2;
        float d5 = e5 * m3;
        float d6 = e6 * m4;
        float d7 = e7;
        float d8 = e8 * m5;
        float c0 = d0 + d1;
        float c1 = d0 - d1;
        float c2 = d2 - d3;
        float c3 = d3;
        float c4 = d4 + d8;
        float c5 = d5 + d7;
        float c6 = d6 - d8;
        float c7 = d7;
        float c8 = c5 - c6;
        float b0 = c0 + c3;
        float b1 = c1 + c2;
        float b2 = c1 - c2;
        float b3 = c0 - c3;
        float b4 = c4 - c8;
        float b5 = c8;
        float b6 = c6 - c7;
        float b7 = c7;
        component[0 * 8 + i] = b0 + b7;
        component[1 * 8 + i] = b1 + b6;
        component[2 * 8 + i] = b2 + b5;
        component[3 * 8 + i] = b3 + b4;
        component[4 * 8 + i] = b3 - b4;
        component[5 * 8 + i] = b2 - b5;
        component[6 * 8 + i] = b1 - b6;
        component[7 * 8 + i] = b0 - b7;
    }
    for (uint i = 0; i < 8; ++i) {
        float g0 = component[i * 8 + 0] * s0;
        float g1 = component[i * 8 + 4] * s4;
        float g2 = component[i * 8 + 2] * s2;
        float g3 = component[i * 8 + 6] * s6;
        float g4 = component[i * 8 + 5] * s5;
        float g5 = component[i * 8 + 1] * s1;
        float g6 = component[i * 8 + 7] * s7;
        float g7 = component[i * 8 + 3] * s3;
        float f0 = g0;
        float f1 = g1;
        float f2 = g2;
        float f3 = g3;
        float f4 = g4 - g7;
        float f5 = g5 + g6;
        float f6 = g5 - g6;
        float f7 = g4 + g7;
        float e0 = f0;
        float e1 = f1;
        float e2 = f2 - f3;
        float e3 = f2 + f3;
        float e4 = f4;
        float e5 = f5 - f7;
        float e6 = f6;
        float e7 = f5 + f7;
        float e8 = f4 + f6;
        float d0 = e0;
        float d1 = e1;
        float d2 = e2 * m1;
        float d3 = e3;
        float d4 = e4 * m2;
        float d5 = e5 * m3;
        float d6 = e6 * m4;
        float d7 = e7;
        float d8 = e8 * m5;
        float c0 = d0 + d1;
        float c1 = d0 - d1;
        float c2 = d2 - d3;
        float c3 = d3;
        float c4 = d4 + d8;
        float c5 = d5 + d7;
        float c6 = d6 - d8;
        float c7 = d7;
        float c8 = c5 - c6;
        float b0 = c0 + c3;
        float b1 = c1 + c2;
        float b2 = c1 - c2;
        float b3 = c0 - c3;
        float b4 = c4 - c8;
        float b5 = c8;
        float b6 = c6 - c7;
        float b7 = c7;
        component[i * 8 + 0] = b0 + b7;
        component[i * 8 + 1] = b1 + b6;
        component[i * 8 + 2] = b2 + b5;
        component[i * 8 + 3] = b3 + b4;
        component[i * 8 + 4] = b3 - b4;
        component[i * 8 + 5] = b2 - b5;
        component[i * 8 + 6] = b1 - b6;
        component[i * 8 + 7] = b0 - b7;
    }
}

// perform IDCT on all MCUs
void Jpeg_inverseDctBlocks(JpegImage *img, Block *blocks) {
    for (uint y = 0; y < img->blocksHeight; y += img->verticalSamplingFactor) {
        for (uint x = 0; x < img->blocksWidth; x += img->horizontalSamplingFactor) {
            for (uint i = 0; i < img->colorComponentsCount; ++i) {
                for (uint v = 0; v < img->colorComponents[i].verticalSamplingFactor; ++v) {
                    for (uint h = 0; h < img->colorComponents[i].horizontalSamplingFactor; ++h) {
                        int *selectedColorComponent = Jpeg_getComponentDataByComponentId(&blocks[(y + v) * img->blocksWidthReal + (x + h)], img->colorComponents[i].componentId);
                        Jpeg_inverseDctBlockComponent(selectedColorComponent);
                    }
                }
            }
        }
    }
}

/*// convert a pixel from YCbCr color space to RGB
void YCbCrToRGBPixel(int& y, int& cb, int& cr) {
    int r = y + 1.402 * cr + 128;
    int g = (y - (0.114 * (y + 1.772 * cb)) - 0.299 * (y + 1.402 * cr)) / 0.587 + 128;
    int b = y + 1.772 * cb + 128;
    if (r < 0)   r = 0;
    if (r > 255) r = 255;
    if (g < 0)   g = 0;
    if (g > 255) g = 255;
    if (b < 0)   b = 0;
    if (b > 255) b = 255;
    y  = r;
    cb = g;
    cr = b;
}*/
// convert all pixels in an MCU from YCbCr color space to RGB
void Jpeg_YCbCrToRGBBlock(JpegImage *img, Block *luminanceBlock, Block *cbcrBlock, int v, int h) {
    for (uint y = 7; y < 8; --y) { // one mcu element
        for (uint x = 7; x < 8; --x) {
            const uint pixel = y * 8 + x;
            const uint cbcrPixelRow = y / img->verticalSamplingFactor + 4 * v;
            const uint cbcrPixelColumn = x / img->horizontalSamplingFactor + 4 * h;
            const uint cbcrPixel = cbcrPixelRow * 8 + cbcrPixelColumn;
            int r = luminanceBlock->luminance[pixel]                               + 1.402f * cbcrBlock->redChrominance[cbcrPixel] + 128;
            int g = luminanceBlock->luminance[pixel] - 0.344f * cbcrBlock->blueChrominance[cbcrPixel] - 0.714f * cbcrBlock->redChrominance[cbcrPixel] + 128;
            int b = luminanceBlock->luminance[pixel] + 1.772f * cbcrBlock->blueChrominance[cbcrPixel]                               + 128;
            if (r < 0)   r = 0;
            if (r > 255) r = 255;
            if (g < 0)   g = 0;
            if (g > 255) g = 255;
            if (b < 0)   b = 0;
            if (b > 255) b = 255;
            luminanceBlock->red[pixel] = r;
            luminanceBlock->green[pixel] = g;
            luminanceBlock->blue[pixel] = b;
        }
    }
}

// convert all pixels from YCbCr color space to RGB
void Jpeg_YCbCrToRGBBlocks(JpegImage *img, Block *blocks) {
    for (uint y = 0; y < img->blocksHeight; y += img->verticalSamplingFactor) {
        for (uint x = 0; x < img->blocksWidth; x += img->horizontalSamplingFactor) {
            Block *cbcrBlock = &blocks[y * img->blocksWidthReal + x];
            for (uint v = img->verticalSamplingFactor - 1; v < img->verticalSamplingFactor; --v) {
                for (uint h = img->horizontalSamplingFactor - 1; h < img->horizontalSamplingFactor; --h) {
                    Block *luminanceBlock = &blocks[(y + v) * img->blocksWidthReal + (x + h)];
                    Jpeg_YCbCrToRGBBlock(img, luminanceBlock, cbcrBlock, v, h);
                }
            }
        }
    }
}

Block *Jpeg_getBlocks(JpegImage *img) {

    Block *blocks = malloc(sizeof(Block) * img->blocksHeightReal * img->blocksWidthReal);
    if(blocks == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for blocks\n");
        exit(1);
    }
    Jpeg_decodeBlocks(img, blocks);
    Jpeg_dequantizeBlocks(img, blocks);
    Jpeg_inverseDctBlocks(img, blocks);
    Jpeg_YCbCrToRGBBlocks(img, blocks);
    return blocks;
}

JpegImage Jpeg_readImage(char *fileName) {
    
    FILE *f = fopen(fileName, "rb");
    if(f == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", fileName);
        exit(1);
    }

    JpegImage img;
    Jpeg_initImage(&img);
    Jpeg_readJpegSegments(f, &img);
    Jpeg_generateHuffmanTablesCodes(&img);
    Jpeg_readHuffmanCodedBitStream(f, &img);
    fclose(f);
    return img;
}

void Jpeg_saveImageAsPpm(JpegImage *img, Block *blocks, char *fileName) {

    FILE *f = fopen(fileName, "wb");
    if(f == NULL) {
        fprintf(stderr, "ERROR: unable to open file: %s\n", fileName);
        exit(1);
    }

    fputc('P', f);
    fputc('6', f);
    fputc('\n', f);
    fprintf(f, "%d %d\n255\n", img->imageWidth, img->imageHeight);
    for(int y = 0; y < img->imageHeight; y++) {
        int mcuRow = y / 8;
        int pixelRow = y % 8;
        for(int x = 0; x < img->imageWidth; x++) {
            int mcuColumn = x / 8;
            int pixelColumn = x % 8;
            int mcuIndex = mcuRow * img->blocksWidthReal + mcuColumn;
            int pixelIndex = pixelRow * 8 + pixelColumn;
            fputc(blocks[mcuIndex].red[pixelIndex], f);
            fputc(blocks[mcuIndex].green[pixelIndex], f);
            fputc(blocks[mcuIndex].blue[pixelIndex], f);
        }
    }

    fclose(f);
}

void putInt(FILE *f, int a) {
    fputc((a >> 0)  & 0xff, f);
    fputc((a >> 8)  & 0xff, f);
    fputc((a >> 16) & 0xff, f);
    fputc((a >> 24) & 0xff, f);
}
void putShort(FILE *f, int a) {
    fputc((a >> 0)  & 0xff, f);
    fputc((a >> 8)  & 0xff, f);
}

// write all the pixels in the MCUs to a BMP file
void Jpeg_saveImageAsBmp(JpegImage *img, Block *blocks, char *filename) {
    // open file
    FILE *f = fopen(filename, "wb");

    const uint paddingSize = img->imageWidth % 4;
    const uint size = 14 + 12 + img->imageHeight * img->imageWidth * 3 + paddingSize * img->imageHeight;

    fputc('B', f);
    fputc('M', f);

    putInt(f, size);
    putInt(f, 0);
    putInt(f, 0x1A);
    putInt(f, 12);
    putShort(f, img->imageWidth);
    putShort(f, img->imageHeight);
    putShort(f, 1);
    putShort(f, 24);
    for (uint y = img->imageHeight - 1; y < img->imageHeight; --y) {
        const uint mcuRow = y / 8;
        const uint pixelRow = y % 8;
        for (uint x = 0; x < img->imageWidth; ++x) {
            const uint mcuColumn = x / 8;
            const uint pixelColumn = x % 8;
            const uint mcuIndex = mcuRow * img->blocksWidthReal + mcuColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;
            fputc(blocks[mcuIndex].blue [pixelIndex], f);
            fputc(blocks[mcuIndex].green[pixelIndex], f);
            fputc(blocks[mcuIndex].red  [pixelIndex], f);
        }
        for (uint i = 0; i < paddingSize; ++i) {
            fputc(0, f);
        }
    }
    fclose(f);
}

void fromPpmToBitmap(char *ppmFileName, char *bitmapFileName) {
    FILE *fppm = fopen("/home/yassine/Desktop/cProjects/olivec/edgeDetection/flowers.ppm", "rb");
    FILE *fbitmap = fopen("/home/yassine/Desktop/cProjects/olivec/jpg/tempjpg/flowers.bmp", "wb");
    if(fppm == NULL) {
        fprintf(stderr, "could not open file square1.ppm\n");
        exit(1);
    }
    if(fbitmap == NULL) {
        fprintf(stderr, "could not open file square1.bmp\n");
        exit(1);
    }

    fgetc(fppm);fgetc(fppm);fgetc(fppm);    // P6\n
    int width, height;
    fscanf(fppm, "%d %d", &width, &height);
    fgetc(fppm);fgetc(fppm);fgetc(fppm);fgetc(fppm);fgetc(fppm);   // \n255\n

    int size = 14 + 12 + height * width;
    fprintf(fbitmap, "BM");
    fwrite(&size, sizeof(size), 1, fbitmap);

    int a = 0;
    fwrite(&a, sizeof(int), 1, fbitmap);
    a = 0x1a;
    fwrite(&a, sizeof(int), 1, fbitmap);
    a = 12;
    fwrite(&a, sizeof(int), 1, fbitmap);

    fputc(width & 0xff, fbitmap);
    fputc((width >> 8) & 0xff, fbitmap);
    fputc(height & 0xff, fbitmap);
    fputc((height >> 8) & 0xff, fbitmap);

    fputc(1 & 0xff, fbitmap);
    fputc((1 >> 8) & 0xff, fbitmap);

    fputc(24 & 0xff, fbitmap);
    fputc((24 >> 8) & 0xff, fbitmap);

    Vector vec;
    Jpeg_initVector(&vec);

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            byte red = fgetc(fppm);
            byte green = fgetc(fppm);
            byte blue = fgetc(fppm);
            Jpeg_appendElementToVector(&vec, green);
            Jpeg_appendElementToVector(&vec, red);
            Jpeg_appendElementToVector(&vec, blue);
        }
    }

    for(int i = vec.length - 1 - width*3; i >= 0; i -= width*3) {
        for(int j = 0; j < width * 3; j += 3) {
            fputc(vec.data[i + j], fbitmap);
            fputc(vec.data[i + j + 1], fbitmap);
            fputc(vec.data[i + j + 2], fbitmap);
        }
    }

    for(int i = 0; i < vec.length; i++) {
        fputc(vec.data[i], fbitmap);
    }


    fclose(fbitmap);
    fclose(fppm);
}







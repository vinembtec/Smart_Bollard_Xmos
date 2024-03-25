
#ifndef JPEG_C_LIB_TEST
#define JPEG_C_LIB_TEST

/* Library headers */
#include "fs_support.h"

extern int imageCaptureCount;
/*
extern uint8_t jpegFullSizeImageCount;
extern uint8_t jpegNumberPlateSizeImageCount;
extern uint8_t previousJpegFullSizeImageCount;
extern uint8_t previousJpegNumberPlateSizeImageCount;

extern uint32_t jpegFullSizeImageNumbers;
extern uint32_t jpegNumberPlateSizeImageNumbers;
extern uint32_t previousJpegFullSizeImageNumbers;
extern uint32_t previousJpegNumberPlateSizeImageNumbers;

extern uint8_t executeBrighterDataImageTask;
extern uint32_t currentBrighterDataImages;
extern uint32_t previousBrighterDataImages;
extern uint32_t brighterDataImages;
extern int iEnableBrighterDataImage;

extern uint16_t previousTotalJpegImages;
extern uint16_t totalCapturedNumImages;*/

//#include "jpeg-6b/jpeglib.h"

/*#define FULL_IMAGE_SIZE (1088 * 1928)
#define NUM_IMAGE_BLOCKS 32
#define IMAGE_BLOCK_SIZE (FULL_IMAGE_SIZE / NUM_IMAGE_BLOCKS)
#define NUM_ROWS_PER_IMAGE_BLOCK (IMAGE_BLOCK_SIZE / 1928)*/

typedef unsigned char BYTE;
typedef unsigned short WORD;

/*typedef struct {
    unsigned long startRow;
    unsigned long startColumn;
    unsigned long width;
    unsigned long height;
} RegionOfInterest_t;*/

//void JpegColorImage_create(char *filename, int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height);
//void JpegColorImage_create(int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height);
void JpegCompress_blockWise(int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height, uint8_t currentSession);
void RawFile_create(int imgBlock, unsigned short *rawBuffer);

extern uint8_t currentSession_global;

#endif	//JPEG_C_LIB_TEST
// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef CAMERA_MAIN_H_
#define CAMERA_MAIN_H_

#define MAX_COMPRESSED_IMAGES_IN_DDR 20	//TODO:: to be modified based on memory available in LPDDR.
#define MAX_COMPRESSED_IMAGE_SIZE ((FRAME_SIZE * 2) / MAX_COMPRESSED_IMAGES_IN_DDR) 	//TODO:: to be modified based on memory available in LPDDR.

#define SENSOR_IMAGE_HEIGHT (1088)
#define SENSOR_IMAGE_WIDTH  (1928)
#define IMG_ROWS (1088)
#define IMG_COLS (1928)
#define PIX_BYTES (2)

#define STRIDE (IMG_COLS*PIX_BYTES+48)
#if (((STRIDE)%64)!=0)
#error Stride must be a multiple of 64
#endif 

#define FRAME_SIZE (STRIDE*IMG_ROWS)
#if (((FRAME_SIZE)%64)!=0)
#error Frame size must be a multiple of 64
#endif 

#define FRAMES_IN_DDR (3)	//TBD //TODO:: to be modified based on memory available in LPDDR.
//#if ON_TILE(1)
/*typedef struct frame_buffer 
{
    uint8_t frames[FRAMES_IN_DDR][FRAME_SIZE];//n.b the second length must be a multiple of 64 bytes (this happens to be without any padding)
    int serial;
} decoupler_buffer_t;
*/
typedef union ImageData {
		uint8_t frames[FRAMES_IN_DDR][FRAME_SIZE];//n.b the second length must be a multiple of 64 bytes (this happens to be without any padding)
		unsigned short frameWord[FRAMES_IN_DDR][FRAME_SIZE/2];//n.b the second length must be a multiple of 64 bytes (this happens to be without any padding)
} ImageData_t;
	
typedef struct frame_buffer 
{
	ImageData_t image;
    int serial;
} decoupler_buffer_t;

/*typedef struct Compressed_Image_Data
{
	uint8_t number;
	char fileName[200];
	uint32_t fileSize;
    uint8_t data[(MAX_COMPRESSED_IMAGE_SIZE - 205)];
} CompressedImageData_t;

typedef struct Compressed_Image
{
    CompressedImageData_t image[MAX_COMPRESSED_IMAGES_IN_DDR];	//n.b the second length must be a multiple of 64 bytes (this happens to be without any padding)
    int serial;
} CompressedImage_t;*/
//#endif

#endif /* CAMERA_MAIN_H_ */

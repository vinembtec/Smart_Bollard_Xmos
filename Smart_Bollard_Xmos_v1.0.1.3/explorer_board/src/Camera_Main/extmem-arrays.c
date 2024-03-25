#include <stdint.h>
/* System headers */
#include <platform.h>
#include <xs1.h>

#include "jpeg-6b/jmorecfg.h"

#if ON_TILE(1)

/*#define MAX_IMG_ROWS (1088)
#define MAX_IMG_COLS (1928)
#define FULL_IMAGE_SIZE (MAX_IMG_ROWS * MAX_IMG_COLS)
#define NUM_IMAGE_BLOCKS 136
#define IMAGE_BLOCK_SIZE (FULL_IMAGE_SIZE / NUM_IMAGE_BLOCKS)
#define NUM_ROWS_PER_IMAGE_BLOCK (IMAGE_BLOCK_SIZE / MAX_IMG_COLS)*/

//__attribute__((section(".ExtMem_data"))) int x[2];	//added on 24_03_2022 
//JSAMPLE imageBuffer[IMAGE_BLOCK_SIZE * 3];	/* Points to large array of R,G,B-order data */
#endif

#include <xcore/channel.h>
/*
 * example.c
 *
 * This file illustrates how to use the IJG code as a subroutine library
 * to read or write JPEG image files.  You should look at this code in
 * conjunction with the documentation file libjpeg.doc.
 *
 * This code will not do anything useful as-is, but it may be helpful as a
 * skeleton for constructing routines that call the JPEG library.  
 *
 * We present these routines in the same coding style used in the JPEG code
 * (ANSI function definitions, etc); but you are of course free to code your
 * routines in a different style if you prefer.
 */

#include <stdio.h>

/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "jpeg-6b/jpeglib.h"

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>

#include "assert.h"

#include <string.h>
#include <print.h>

#include "jpeg_c_lib_test.h"
#include "host_mcu/host_mcu_types.h"

//#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define MAX_IMG_ROWS (1088)
#define MAX_IMG_COLS (1928)
#define FULL_IMAGE_SIZE (MAX_IMG_ROWS * MAX_IMG_COLS)
//#define NUM_IMAGE_BLOCKS 136
#define NUM_IMAGE_BLOCKS 544
#define IMAGE_BLOCK_SIZE (FULL_IMAGE_SIZE / NUM_IMAGE_BLOCKS)
#define NUM_ROWS_PER_IMAGE_BLOCK (IMAGE_BLOCK_SIZE / MAX_IMG_COLS)

extern void rtos_fatfs_init_tast_call(void);

	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	extern void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 );
	extern void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 );
	extern void Debug_Display( int8_t minimumDebugLevel, const char * pszText );
	//////////////////////////////////
	extern uint8_t capture_sequence_number;
//////////SHARE FLASH////////////////
extern char SF_Filename[200];
extern int sf_write(uint8_t* FTPImagFileBufferptr, uint8_t* SF_Filename, uint32_t uImageSize, uint8_t capture_sequence_number,uint8_t capture_number);
extern uint8_t capture_sequence_number;
//////////////////////////	
/******************** JPEG COMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to feed data into the JPEG compressor.
 * We present a minimal version that does not worry about refinements such
 * as error recovery (the JPEG code will just exit() if it gets an error).
 */


/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 * For this example, we'll assume that this data structure matches the way
 * our application has stored the image in memory, so we can just pass a
 * pointer to our image buffer.  In particular, let's say that the image is
 * RGB color and is described by:
 */

JSAMPLE imageBuffer[((MAX_IMG_ROWS/8) * (MAX_IMG_COLS/8)) * 3];	/* Points to large array of R,G,B-order data */
char FTPBaseFilename[200];
//extern JSAMPLE imageBuffer[IMAGE_BLOCK_SIZE * 3];	/* Points to large array of R,G,B-order data */
char newFileName[200];
extern char fName[200];
//char filename[200] = {"IMG_FL_8044_32065_YYYYMNDD_HHMMSS_00.jpeg"};
//char rawFileName[200] = {"IMG_FL_8044_32065_YYYYMNDD_HHMMSS_00.raw"};

/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */
struct jpeg_compress_struct cinfo;
FIL outfile;		/* target file */
//FILE *rawfile;		/* target file */
FIL fp;
unsigned int rawDataCount = 0;	
extern ImageInfo_t imageCapture;

unsigned char jo_write_jpg(int imgBlock, const char *filename, const void *data, int width, int height, int comp, int quality);

#if 0
//void RawFile_create(int imgBlock, char *rawFileName, unsigned short *rawBuffer)
void RawFile_create(int imgBlock, unsigned short *rawBuffer)
{
	if (0 == imgBlock)
	{
		if ((rawfile = fopen(rawFileName, "wb")) == NULL) 
		{		
			printf("\r\n'%s' Failed to Open the File.", rawFileName);
			//exit(1);
			return;
		}
		
		printf("\r\n\r\n'%s' File Opened.", rawFileName); 
		rawDataCount = 0;
	}
	
	/* Write Data to File */	
	/*while (rawDataCount < FULL_IMAGE_SIZE)
	{
		fwrite(rawBuffer, sizeof(WORD), IMAGE_BLOCK_SIZE, rawfile);
		rawDataCount += IMAGE_BLOCK_SIZE;
		printf("\r\nWrite Block %d in File.", (imgBlock));
		printf("\r\nImage Size: %d.", (rawDataCount));
		
		if (imgBlock >= (NUM_IMAGE_BLOCKS - 1))
		{
			printf("\r\nBlocks %d Written in File.", (imgBlock + 1));
		}
		else
		{
			return;
		}	
	}*/
	
	while (rawDataCount < FULL_IMAGE_SIZE)
	{
		fwrite(rawBuffer, sizeof(WORD), MAX_IMG_COLS, rawfile);
		rawDataCount += MAX_IMG_COLS;
		//printf("\r\nWrite Block %d in File.", (imgBlock));
		//printf("\r\nImage Size: %d.", (rawDataCount));
		printstr("\r\nWrite Block: ");
		printintln(imgBlock);   
		printstr("\r\nImage Size: ");
		printintln(rawDataCount);   
	
		if (imgBlock >= (MAX_IMG_ROWS - 1))
		{
			//printf("\r\nBlocks %d Written in File.", (imgBlock + 1));
			printstr("\r\nTotal Blocks: ");
			printintln((imgBlock + 1));   
		}
		else
		{
			return;
		}	
	}
	
	/* Closing the file using fclose() */
	fclose(rawfile);
	  
	printf("\r\n'%s' File Created Success.\n", rawFileName);
}
#endif

void cleanup_flash()
{
	FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;
	Debug_TextOut(0,"Cleaning flash");
	fr = f_findfirst(&dj, &fno, "", "*");
	while (fr == FR_OK && fno.fname[0]) 
	{
		if ( (strncmp(fno.fname,"IMG_NP", 6) == 0) || (strncmp(fno.fname,"IMG_FL", 6) == 0))
		{
			Debug_Output1(0, "Cleaning Flash IMG file %s", fno.fname);
			f_unlink(fno.fname);
		}
		else
		{
			Debug_Output1(0, "Flash JNK file %s", fno.fname);
			char logString[100];
			sprintf(logString, "Size: %lu\n", fno.fsize);
			Debug_TextOut(0, logString);
        	sprintf(logString, "Timestamp: %u-%02u-%02u, %02u:%02u\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
			Debug_TextOut(0, logString);
        	sprintf(logString, "Attributes: %c%c%c%c%c\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
			Debug_TextOut(0, logString);
		}
		fr = f_findnext(&dj, &fno); 
	}
	f_closedir(&dj);
	fr = f_findfirst(&dj, &fno, "", "*");
	while (fr == FR_OK && fno.fname[0])
	{
		Debug_Output1(0, "Cleaning Flash JNK file %s", fno.fname);
		f_unlink(fno.fname);
		fr = f_findnext(&dj, &fno); 
	}
	f_closedir(&dj);
}

void write_JPEG_file (char *filename, int imgBlock, int quality)
//void write_JPEG_file (char * filename, int quality, JSAMPLE *image_buffer)
//void write_JPEG_file (int quality, JSAMPLE *image_buffer)
{
  //FIL fil;
  //JSAMPLE image_buffer[((MAX_IMG_ROWS/8) * (MAX_IMG_COLS/8)) * 3];	/* Points to large array of R,G,B-order data */
  
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  //struct jpeg_compress_struct cinfo;
  
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  //FILE * outfile;		/* target file */
    
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  //printf("\r\nsizeof cinfo: %d", sizeof(cinfo));
  
  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  if (0 == imgBlock)
  {
	  cinfo.err = jpeg_std_error(&jerr);
	  
	  /* Now we can initialize the JPEG compression object. */
	  jpeg_create_compress(&cinfo);		//commented on 08_04_2022 
	  //rtos_printf("\r\n jpeg_create_compress: after \n");
	  //printf("\nsizeof(BEFORE struct jpeg_compress_struct): %lu\n", sizeof(struct jpeg_compress_struct));
	  //jpeg_CreateCompress((cinfo), JPEG_LIB_VERSION, (size_t) sizeof(struct jpeg_compress_struct));

	  /* Step 2: specify data destination (eg, a file) */
	  /* Note: steps 2 and 3 can be done in either order. */

	  /* Here we use the library-supplied code to send compressed data to a
	   * stdio stream.  You can also write your own code to do something else.
	   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	   * requires it in order to write binary files.
	   */ //fr = f_open(&fil, "message.txt", FA_READ)

    FRESULT fr;     /* Return value */
	   
#if 0	   
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;


    /* Get volume information and free clusters of drive 1 */
    fr = f_getfree("1:", &fre_clust, &fs);
    //if (fr) die(fr);

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2, fre_sect / 2);
#endif	

#if 0
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */	   
			rtos_printf("TILE = %d\n", 1);			
			fr = f_findfirst(&dj, &fno, "", "*.*"); /* Start to search for photo files */

			while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
				rtos_printf("TILE0_fno.fname = %s\n", fno.fname);                /* Print the object name */
				fr = f_findnext(&dj, &fno);               /* Search for next item */
			}
			rtos_printf("TILE0_last fr = %d\n", fr); 
			rtos_printf("TILE0_last fno.fname = %s\n", fno.fname); 
#endif			
	  //rtos_printf("\r\n file name : %s \n",filename);
	  rtos_fatfs_init_tast_call();
	  
	  if ((fr = f_open(&outfile, filename, FA_CREATE_ALWAYS | FA_WRITE |FA_READ)/*fopen(filename, "wb")*/) != NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		Debug_TextOut(0,"can't open/create file(Memory Full):");
		Debug_TextOut(0,filename);
		cleanup_flash();
		//exit(1);
		if ((fr = f_open(&outfile, filename, FA_CREATE_ALWAYS | FA_WRITE |FA_READ)/*fopen(filename, "wb")*/) != NULL)
		{
			Debug_TextOut(0,"Cleanup failed");
			return;
		}
		
	  }
	  
	  //printf("\r\nJC: first outfile: %ld\n", outfile);
	  rtos_printf("\r\n file open success: after = %d\n",fr);
	  Debug_Output1(0,"file open success: after = %d",fr);
  
	  jpeg_stdio_dest(&cinfo, &outfile);
	  //printf("\r\n jpeg_stdio_dest: after \n");
	  /* Step 3: set parameters for compression */

	  /* First we supply a description of the input image.
	   * Four fields of the cinfo struct must be filled in:
	   */
	  cinfo.image_width = MAX_IMG_COLS / 2; 	/* image width and height, in pixels */
	  cinfo.image_height = MAX_IMG_ROWS / 2;
	  //cinfo.image_height = 10;
	  cinfo.input_components = 3;		/* # of color components per pixel */
	  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	  
	  /* Now use the library's routine to set default compression parameters.
	   * (You must set at least cinfo.in_color_space before calling this,
	   * since the defaults depend on the source color space.)
	   */
	  jpeg_set_defaults(&cinfo);
	  //printf("\r\n jpeg_set_defaults: after \n");
	  /* Now you can set any non-default parameters you wish to.
	   * Here we just illustrate the use of quality (quantization table) scaling:
	   */
	  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	  //printf("\r\n jpeg_set_quality: after \n");
	  /* Step 4: Start compressor */

	  /* TRUE ensures that we will write a complete interchange-JPEG file.
	   * Pass TRUE unless you are very sure of what you're doing.
	   */
	  jpeg_start_compress(&cinfo, TRUE);
	  //printf("\r\n jpeg_start_compress: after \n");
	  cinfo.next_scanline = 0;  
  }
  
  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
   
  row_stride = (MAX_IMG_COLS / 2) * 3;	/* JSAMPLEs per row in image_buffer */

  //printf("\ncinfo.next_scanline(BEFORE jpeg_write_scanlines): %lu\n", cinfo.next_scanline);
  //printf("\ncinfo.image_height(BEFORE jpeg_write_scanlines): %lu\n", cinfo.image_height);
  int nextScanline = 0;
  
  while (cinfo.next_scanline < cinfo.image_height)
  {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
	 
    //row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
	//row_pointer[0] = & imageBuffer[cinfo.next_scanline * row_stride];
	//printf("\r\n imageBuffer[nextScanline * row_stride]: %ld", strlen(imageBuffer[nextScanline * row_stride]));
	//printf("\r\n next_scanline: %d", nextScanline);
	//printf("\r\n row_stride: %d", row_stride);
	row_pointer[0] = & imageBuffer[nextScanline * row_stride];
	//printf("\r\nbefore cinfo.next_scanline: %d", cinfo.next_scanline);	
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);	
	//printf("\r\nafter cinfo.next_scanline: %d", cinfo.next_scanline);
	//printf("\r\ncinfo.next_scanline: %d", cinfo.next_scanline);
	//printf("\r\nnextScanline: %d", nextScanline);
	
	//if (cinfo.next_scanline >= NUM_ROWS_PER_IMAGE_BLOCK)
	if (++nextScanline >= (NUM_ROWS_PER_IMAGE_BLOCK / 2))
	{
		//cinfo.next_scanline = 0;
		//printf("\r\nBlock(%d) Compressed.", imgBlock);
		
		if (imgBlock >= (NUM_IMAGE_BLOCKS - 1))
		{
			//printf("\r\ncinfo.next_scanline: %d", (int)cinfo.next_scanline);
			//printf("\r\nBlocks %d Compressed.", (imgBlock + 1));
			//printf("\r\nJC: Jpeg Compression Completed. %d", (imgBlock + 1));
			//break;
		}
		else
		{
			//printf("\r\n return ");
			return;
		}		
	}
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
   
  //----------------------------------------------
  //TBD //only for testing //--to be Removed
  //fseek(outfile, 0L, SEEK_END);  
  long int jpegFileSize = f_size(&outfile);//36;//ftell(outfile);  	// calculating the size of the file
  printf("\r\nJC: Jpeg File Size: %lu", jpegFileSize);
  Debug_Output1(0,"JC: Jpeg File Size: %lu", jpegFileSize);
  //----------------------------------------------		 
  //printf("\r\nJC: end outfile: %ld\n", outfile);
  /* After finish_compress, we can close the output file. */
  f_close(&outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

//Section to save main memory image to share flash/fs/IMGP
#if 0
{
    FIL test_file;
	uint8_t *file_contents_buf = NULL;
	uint32_t uImageSize = -1;
	uint32_t bytes_read = 0;
    FRESULT result;
	
	char fname[100];	
    char tempString[200];
	uint8_t capture_number;
	char tempPrintString[300];
	uint32_t i = 0;
	
	rtos_printf("filename to read for saving to share flash %s\n", filename);
    result = f_open(&test_file, filename, FA_READ);
    if (result == FR_OK)
    {
        rtos_printf("Found file %s\n", filename);
        uImageSize = f_size(&test_file);
		rtos_printf("file_size %ld\n", uImageSize);
        file_contents_buf = pvPortMalloc(sizeof(uint8_t)*uImageSize);
        configASSERT(file_contents_buf != NULL);

        result = f_read(&test_file,
                        (uint8_t*)file_contents_buf,
                        uImageSize,
                        (unsigned int*)&bytes_read);
        configASSERT(bytes_read == uImageSize);
    } else {
        rtos_printf("Failed to open file %s\n", filename);
    }
			printf("\r\n 2,file_contents_buf = ");
			for(int i=0; i<900; i++)
			
			 {	 printf("%02X ",(uint8_t)file_contents_buf[i]);}
			 
    if (uImageSize != -1)
    {
        rtos_printf("\r\nLoaded file %s\n", filename);
        f_close(&test_file);
    }
	
		////////////  Get Seq Number from File Name ////////////////////////
	sprintf(tempString,"%s",filename);	
	printf("\nFile name at fopen is ===================================>> %s\n",tempString);		
	if(strlen(filename)>43){
					sprintf(fname,"%s",filename);// IMG_FL_00S8028_32018_20230220_080835_01.jpeg
					sprintf(SF_Filename,"%s",filename);
					printf("\r\nFile name before parsing is ===================================>> %s\n",tempString);
					char * token = strtok(fname, "_");
				   // printf( " %s\n", token );
				    token = strtok(NULL, "_");
				   // printf( " %s\n", token );
				    token = strtok(NULL, "S");
				    printf( " %s\n Token for seq number", token );
				    capture_sequence_number = atoi(token);
				    Debug_Output1(0,"\r\nSequence Number is  %d\n", capture_sequence_number);
				   printf("\r\nSequence Number is  %d\n", capture_sequence_number);
					
					/////////////////////End////////////////////////////////////////////
					
					
					//////////  Get Capture number /////////////////////////////////////
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, ".");
					printf( "File name parsing for capture num %s\n", token );
					capture_number = atoi(token);
					Debug_Output1(0,"capture_number Number is  %d\n", capture_number);
                   }
   else
				{
					capture_number = 1;
					capture_sequence_number = 1;								
				}

printf("\nFile name after parsing %s   Capture sequence number is %d capture number is %d\n\n",filename,capture_sequence_number,capture_number);
	
            /////////////////////SF/////////////////////////////////////
            //capture_sequence_number = get_seq_num();
             printf("In fopen after f_read()\n");
             printf("\n	Shared flash write function enter \n");
             Debug_TextOut(0,"\nShared flash write function enter\n");
             sprintf(tempPrintString, "\nJPEG: Before Shared flash func Image siz %lu  Capture Seq Number ==>%d<===\n",uImageSize,capture_sequence_number);
			 Debug_TextOut(0, tempPrintString);           
             printf("\nJPEG: Before Shared flash func Image siz %lu  Capture Seq Number ==>%d<===\n",uImageSize,capture_sequence_number);  
			 printf("\n\nValue of image buffer\n");
			 
			 sprintf(tempPrintString, "\nValue of image buffer\n");
			 printf("\nFirst 900 bytes of Image data after fread\n");
			// int32_t i;
			 //ps:27022023 
			//for(i=0;i<uImageSize;i++)
			for(i=0; i<900; i++)
			
			 {	 printf("%02X ",(uint8_t)file_contents_buf[i]);}
			  Debug_TextOut(0, tempPrintString);
             //FTPImagFileBufferptr_sf = file_contents_buf;
			 
			 printf("file_contents_buf = %lu \n",file_contents_buf);			 
             
			 sf_write(file_contents_buf,SF_Filename,uImageSize,capture_sequence_number,capture_number);
             //  sf_write_test();
            // shared_flash_write();
             printf("\n	Shared flash write function exit \n");
             Debug_TextOut(0,"\nShared flash write function exit \n");
             printf("Memory successfully allocated using malloc. end = %d = bytes %d\n",result,bytes_read);
             ///////////////////////////////////////////////////////////	
	
	vPortFree(file_contents_buf);	
	///////////////////End//////////////////////////////////////////////
}
#endif	
//End of Share Flash section
	
  /* And we're done! */
}


/*
 * SOME FINE POINTS:
 *
 * In the above loop, we ignored the return value of jpeg_write_scanlines,
 * which is the number of scanlines actually written.  We could get away
 * with this because we were only relying on the value of cinfo.next_scanline,
 * which will be incremented correctly.  If you maintain additional loop
 * variables then you should be careful to increment them properly.
 * Actually, for output to a stdio stream you needn't worry, because
 * then jpeg_write_scanlines will write all the lines passed (or else exit
 * with a fatal error).  Partial writes can only occur if you use a data
 * destination module that can demand suspension of the compressor.
 * (If you don't know what that's for, you don't need it.)
 *
 * If the compressor requires full-image buffers (for entropy-coding
 * optimization or a multi-scan JPEG file), it will create temporary
 * files for anything that doesn't fit within the maximum-memory setting.
 * (Note that temp files are NOT needed if you use the default parameters.)
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 *
 * Scanlines MUST be supplied in top-to-bottom order if you want your JPEG
 * files to be compatible with everyone else's.  If you cannot readily read
 * your data in that order, you'll need an intermediate array to hold the
 * image.  See rdtarga.c or rdbmp.c for examples of handling bottom-to-top
 * source data using the JPEG code's internal virtual-array mechanisms.
 */



/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */


/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */

#if 0
GLOBAL(int)
read_JPEG_file (char * filename)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  //while (cinfo.output_scanline < cinfo.output_height)  	
  for (; cinfo.output_scanline < cinfo.output_height; )  	
  {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    //put_scanline_someplace(buffer[0], row_stride);	
  }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 1;
}
#endif

/*
 * SOME FINE POINTS:
 *
 * In the above code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.doc for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.doc for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 */


typedef enum {
	FULL_SIZE_IMAGAE,
	NUMBER_PLATE_IMAGAE,
} ImageType_t;

//TODO:: imageType to be passed this value from JpegColorImage_create() in Camera_Main.xc ..
uint8_t imageType = FULL_SIZE_IMAGAE;


//void JpegColorImage_create(char *filename, int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height)
//void JpegColorImage_create(int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height)
#pragma stackfunction 1000
void JpegCompress_blockWise(int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height, uint8_t currentSession)
{
    unsigned int row;
    unsigned int col;
    unsigned int jpegrow = 0;
    unsigned int jpegcol = 0;
    //unsigned long endRow = 0;
    //unsigned long endColumn = 0;
    int redColorIndex = 0;
    int greenRColorIndex = 0;
    int greenBColorIndex = 0;
    int blueColorIndex = 0;
	
	/* Sanity Check */
	if ((startRow < 0) || (startRow > MAX_IMG_ROWS) ||  
		(startColumn < 0) || (startColumn > MAX_IMG_COLS) || 
		(width < 0) || (width > MAX_IMG_COLS) ||  
		(height < 0) || (height > MAX_IMG_ROWS))
	{
		startRow = 0;
		startColumn = 0;
		width = MAX_IMG_COLS;
		height = MAX_IMG_ROWS;
	}

	if (0 == imgBlock)
	{
		rtos_printf("\r\nJC: ROI(R,C,W,H): %d,%d,%d,%d", startRow, startColumn, width, (height * NUM_IMAGE_BLOCKS));
		Debug_Output6(0,"\r\nJC: ROI(R,C,W,H): %d,%d,%d,%d", startRow, startColumn, width, (height * NUM_IMAGE_BLOCKS),0,0);
	}
	
	redColorIndex = 0;
	greenRColorIndex = 0;
	greenBColorIndex = 0;
	blueColorIndex = 0;
	
	memset(imageBuffer, 0, sizeof(imageBuffer));
	//printf("\r\nimageBuffer: %d ", imageBuffer);
	
	for (row = startRow, jpegrow = 0; row < (startRow + height); row+=2, jpegrow++)
	{
		redColorIndex = 1 + startColumn + (row * width);
		greenRColorIndex = redColorIndex - 1;
		greenBColorIndex = redColorIndex + width;
		blueColorIndex = greenBColorIndex - 1;
		//printf("\r\nstartRow: %d ", startRow);	
		for (col = startColumn, jpegcol = 0; col < (startColumn + width); col+=2, jpegcol+=3)
		{
			imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol] = rawBuffer[redColorIndex];
			imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol + 1] = (rawBuffer[greenRColorIndex] + rawBuffer[greenBColorIndex]) / 2;
			imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol + 2] = rawBuffer[blueColorIndex];

			//if(imgBlock == 1)
			//	printf("\r\n redColorIndex: %ld,greenRColorIndex: %ld,blueColorIndex: %ld ", rawBuffer[redColorIndex],
			//																				 (rawBuffer[greenRColorIndex] + rawBuffer[greenBColorIndex]) / 2, 
			//																				 rawBuffer[blueColorIndex]);	
																							 
			/*if(imgBlock > 57)
				printf("\r\n redColorIndex: %ld,greenRColorIndex: %ld,blueColorIndex: %ld ", imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol],
																							 imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol + 1], 
																							 imageBuffer[(jpegrow * (width / 2) * 3) + jpegcol + 2]);*/																							 
			redColorIndex += 2;
			greenRColorIndex += 2;
			greenBColorIndex += 2;
			blueColorIndex += 2;
					
		}
	}
	//printf("\r\nimgBlock: %d ", imgBlock);
	if (0 == imgBlock)
	{
		sprintf(newFileName, "IMG_FL_%02dS%s", capture_sequence_number,(fName + strlen("IMG_")));
		strcpy(FTPBaseFilename, newFileName);
		//sprintf(newFileName, "/flash/fs/IMGP.JPG");
		rtos_printf("\r\nIncludeing Sequence number in file name %d.", capture_sequence_number);
		rtos_printf("\r\nJC: %s Create.", newFileName);
		Debug_TextOut(0, "JC: %s Create.");		
		Debug_TextOut(0, newFileName);
	}
	
	//if(imgBlock > 55) vTaskDelay(pdMS_TO_TICKS(1000));
	//rtos_printf("\r\n write_JPEG_file: %d ", imgBlock);	
	write_JPEG_file(newFileName, imgBlock, 75);//75);	//TBD //commented on 26_05_2022 //--to be UNcommented 
	//jo_write_jpg(imgBlock, ( const char *)newFileName, (const void *)imageBuffer, width, height, 1, 75);		//TBD //added on 26_05_2022 //--to be Removed 
	//jo_write_jpg(imgBlock, newFileName, imageBuffer, width, height, 1, 75);		//TBD //added on 26_05_2022 //--to be Removed 
	
	if (0 == imgBlock)
	{
		if (FULL_SIZE_IMAGAE == imageType)
		{	
			/* Full Size Image as per ROI */
			if (currentSession)
			{
				imageCapture.vars.jpegFullSizeImageCount++;
				imageCapture.vars.jpegFullSizeImageNumbers |= (1 << imageCaptureCount);
			}
			else
			{
				imageCapture.vars.previousJpegFullSizeImageCount++;
				imageCapture.vars.previousJpegFullSizeImageNumbers |= (1 << imageCaptureCount);
			}
		}
		/* Number Plate Image as per ANPR ROI */
		else if (NUMBER_PLATE_IMAGAE == imageType)
		{		
			if (currentSession)
			{
				imageCapture.vars.jpegNumberPlateSizeImageCount++;
				imageCapture.vars.jpegNumberPlateSizeImageNumbers |= (1 << imageCaptureCount);
			}
			else
			{
				imageCapture.vars.previousJpegNumberPlateSizeImageCount++;
				imageCapture.vars.previousJpegNumberPlateSizeImageNumbers |= (1 << imageCaptureCount);
			}
		}
	}
}

#if 0
static const unsigned char s_jo_ZigZag[] = { 0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,42,3,8,12,17,25,30,41,43,9,11,18,24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,33,38,46,51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63 };

//static void jo_writeBits(FILE *fp, int bitBuf, int bitCnt, const unsigned short *bs) {
static void jo_writeBits(FIL *fp, int bitBuf, int bitCnt, const unsigned short *bs) {
	bitCnt += bs[1];
	bitBuf |= bs[0] << (24 - bitCnt);
	while(bitCnt >= 8) {
		UINT bw;
		unsigned char c = (bitBuf >> 16) & 255;
		//putc(c, fp);
		//FRESULT fr = f_write(fp, 1, sizeof(c), &c);
		FRESULT fr = f_write(&fp, &c, sizeof(c), &bw);
		if(c == 255) {
			//putc(0, fp);
			unsigned char data = 0;
			//fr = f_write(fp, 1, sizeof(data), &data);
			fr = f_write(&fp, &data, sizeof(data), &bw);
		}
		bitBuf <<= 8;
		bitCnt -= 8;
	}
}

static void jo_DCT(float d0, float d1, float d2, float d3, float d4, float d5, float d6, float d7) {
	float tmp0 = d0 + d7;
	float tmp7 = d0 - d7;
	float tmp1 = d1 + d6;
	float tmp6 = d1 - d6;
	float tmp2 = d2 + d5;
	float tmp5 = d2 - d5;
	float tmp3 = d3 + d4;
	float tmp4 = d3 - d4;

	// Even part
	float tmp10 = tmp0 + tmp3;	// phase 2
	float tmp13 = tmp0 - tmp3;
	float tmp11 = tmp1 + tmp2;
	float tmp12 = tmp1 - tmp2;

	d0 = tmp10 + tmp11; 		// phase 3
	d4 = tmp10 - tmp11;

	float z1 = (tmp12 + tmp13) * 0.707106781f; // c4
	d2 = tmp13 + z1; 		// phase 5
	d6 = tmp13 - z1;

	// Odd part
	tmp10 = tmp4 + tmp5; 		// phase 2
	tmp11 = tmp5 + tmp6;
	tmp12 = tmp6 + tmp7;

	// The rotator is modified from fig 4-8 to avoid extra negations.
	float z5 = (tmp10 - tmp12) * 0.382683433f; // c6
	float z2 = tmp10 * 0.541196100f + z5; // c2-c6
	float z4 = tmp12 * 1.306562965f + z5; // c2+c6
	float z3 = tmp11 * 0.707106781f; // c4

	float z11 = tmp7 + z3;		// phase 5
	float z13 = tmp7 - z3;

	d5 = z13 + z2;			// phase 6
	d3 = z13 - z2;
	d1 = z11 + z4;
	d7 = z11 - z4;
} 

static void jo_calcBits(int val, unsigned short bits[2]) {
	int tmp1 = val < 0 ? -val : val;
	val = val < 0 ? val-1 : val;
	bits[1] = 1;
	while(tmp1 >>= 1) {
		++bits[1];
	}
	bits[0] = val & ((1<<bits[1])-1);
}

//static int jo_processDU(FILE *fp, int bitBuf, int bitCnt, float *CDU, int du_stride, float *fdtbl, int DC, const unsigned short HTDC[256][2], const unsigned short HTAC[256][2]) {
static int jo_processDU(FIL *fp, int bitBuf, int bitCnt, float *CDU, int du_stride, float *fdtbl, int DC, const unsigned short HTDC[256][2], const unsigned short HTAC[256][2]) {	
	const unsigned short EOB[2] = { HTAC[0x00][0], HTAC[0x00][1] };
	const unsigned short M16zeroes[2] = { HTAC[0xF0][0], HTAC[0xF0][1] };

	// DCT rows
	for(int i=0; i<du_stride*8; i+=du_stride) {
		jo_DCT(CDU[i], CDU[i+1], CDU[i+2], CDU[i+3], CDU[i+4], CDU[i+5], CDU[i+6], CDU[i+7]);
	}
	// DCT columns
	for(int i=0; i<8; ++i) {
		jo_DCT(CDU[i], CDU[i+du_stride], CDU[i+du_stride*2], CDU[i+du_stride*3], CDU[i+du_stride*4], CDU[i+du_stride*5], CDU[i+du_stride*6], CDU[i+du_stride*7]);
	}
	// Quantize/descale/zigzag the coefficients
	int DU[64];
	for(int y = 0, j=0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x,++j) {
			int i = y*du_stride+x;
			float v = CDU[i]*fdtbl[j];
			DU[s_jo_ZigZag[j]] = (int)(v < 0 ? ceilf(v - 0.5f) : floorf(v + 0.5f));
		}
	}

	// Encode DC
	int diff = DU[0] - DC; 
	if (diff == 0) {
		jo_writeBits(fp, bitBuf, bitCnt, HTDC[0]);
	} else {
		unsigned short bits[2];
		jo_calcBits(diff, bits);
		jo_writeBits(fp, bitBuf, bitCnt, HTDC[bits[1]]);
		jo_writeBits(fp, bitBuf, bitCnt, bits);
	}
	// Encode ACs
	int end0pos = 63;
	for(; (end0pos>0)&&(DU[end0pos]==0); --end0pos) {
	}
	// end0pos = first element in reverse order !=0
	if(end0pos == 0) {
		jo_writeBits(fp, bitBuf, bitCnt, EOB);
		return DU[0];
	}
	for(int i = 1; i <= end0pos; ++i) {
		int startpos = i;
		for (; DU[i]==0 && i<=end0pos; ++i) {
		}
		int nrzeroes = i-startpos;
		if ( nrzeroes >= 16 ) {
			int lng = nrzeroes>>4;
			for (int nrmarker=1; nrmarker <= lng; ++nrmarker)
				jo_writeBits(fp, bitBuf, bitCnt, M16zeroes);
			nrzeroes &= 15;
		}
		unsigned short bits[2];
		jo_calcBits(DU[i], bits);
		jo_writeBits(fp, bitBuf, bitCnt, HTAC[(nrzeroes<<4)+bits[1]]);
		jo_writeBits(fp, bitBuf, bitCnt, bits);
	}
	if(end0pos != 63) {
		jo_writeBits(fp, bitBuf, bitCnt, EOB);
	}
	return DU[0];
}

//bool jo_write_jpg(const char *filename, const void *data, int width, int height, int comp, int quality) {
unsigned char jo_write_jpg(int imgBlock, const char *filename, const void *data, int width, int height, int comp, int quality) {	
	// Constants that don't pollute global namespace
	static const unsigned char std_dc_luminance_nrcodes[] = {0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
	static const unsigned char std_dc_luminance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
	static const unsigned char std_ac_luminance_nrcodes[] = {0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d};
	static const unsigned char std_ac_luminance_values[] = {
		0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,
		0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,
		0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
		0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
		0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,
		0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,
		0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
	};
	static const unsigned char std_dc_chrominance_nrcodes[] = {0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
	static const unsigned char std_dc_chrominance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
	static const unsigned char std_ac_chrominance_nrcodes[] = {0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
	static const unsigned char std_ac_chrominance_values[] = {
		0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
		0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,
		0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,
		0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,
		0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,
		0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
		0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
	};
	// Huffman tables
	static const unsigned short YDC_HT[256][2] = { {0,2},{2,3},{3,3},{4,3},{5,3},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9}};
	static const unsigned short UVDC_HT[256][2] = { {0,2},{1,2},{2,2},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9},{1022,10},{2046,11}};
	static const unsigned short YAC_HT[256][2] = { 
		{10,4},{0,2},{1,2},{4,3},{11,4},{26,5},{120,7},{248,8},{1014,10},{65410,16},{65411,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{12,4},{27,5},{121,7},{502,9},{2038,11},{65412,16},{65413,16},{65414,16},{65415,16},{65416,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{28,5},{249,8},{1015,10},{4084,12},{65417,16},{65418,16},{65419,16},{65420,16},{65421,16},{65422,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{58,6},{503,9},{4085,12},{65423,16},{65424,16},{65425,16},{65426,16},{65427,16},{65428,16},{65429,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{59,6},{1016,10},{65430,16},{65431,16},{65432,16},{65433,16},{65434,16},{65435,16},{65436,16},{65437,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{122,7},{2039,11},{65438,16},{65439,16},{65440,16},{65441,16},{65442,16},{65443,16},{65444,16},{65445,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{123,7},{4086,12},{65446,16},{65447,16},{65448,16},{65449,16},{65450,16},{65451,16},{65452,16},{65453,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{250,8},{4087,12},{65454,16},{65455,16},{65456,16},{65457,16},{65458,16},{65459,16},{65460,16},{65461,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{504,9},{32704,15},{65462,16},{65463,16},{65464,16},{65465,16},{65466,16},{65467,16},{65468,16},{65469,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{505,9},{65470,16},{65471,16},{65472,16},{65473,16},{65474,16},{65475,16},{65476,16},{65477,16},{65478,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{506,9},{65479,16},{65480,16},{65481,16},{65482,16},{65483,16},{65484,16},{65485,16},{65486,16},{65487,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{1017,10},{65488,16},{65489,16},{65490,16},{65491,16},{65492,16},{65493,16},{65494,16},{65495,16},{65496,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{1018,10},{65497,16},{65498,16},{65499,16},{65500,16},{65501,16},{65502,16},{65503,16},{65504,16},{65505,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{2040,11},{65506,16},{65507,16},{65508,16},{65509,16},{65510,16},{65511,16},{65512,16},{65513,16},{65514,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{65515,16},{65516,16},{65517,16},{65518,16},{65519,16},{65520,16},{65521,16},{65522,16},{65523,16},{65524,16},{0,0},{0,0},{0,0},{0,0},{0,0},
		{2041,11},{65525,16},{65526,16},{65527,16},{65528,16},{65529,16},{65530,16},{65531,16},{65532,16},{65533,16},{65534,16},{0,0},{0,0},{0,0},{0,0},{0,0}
	};
	static const unsigned short UVAC_HT[256][2] = { 
		{0,2},{1,2},{4,3},{10,4},{24,5},{25,5},{56,6},{120,7},{500,9},{1014,10},{4084,12},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{11,4},{57,6},{246,8},{501,9},{2038,11},{4085,12},{65416,16},{65417,16},{65418,16},{65419,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{26,5},{247,8},{1015,10},{4086,12},{32706,15},{65420,16},{65421,16},{65422,16},{65423,16},{65424,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{27,5},{248,8},{1016,10},{4087,12},{65425,16},{65426,16},{65427,16},{65428,16},{65429,16},{65430,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{58,6},{502,9},{65431,16},{65432,16},{65433,16},{65434,16},{65435,16},{65436,16},{65437,16},{65438,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{59,6},{1017,10},{65439,16},{65440,16},{65441,16},{65442,16},{65443,16},{65444,16},{65445,16},{65446,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{121,7},{2039,11},{65447,16},{65448,16},{65449,16},{65450,16},{65451,16},{65452,16},{65453,16},{65454,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{122,7},{2040,11},{65455,16},{65456,16},{65457,16},{65458,16},{65459,16},{65460,16},{65461,16},{65462,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{249,8},{65463,16},{65464,16},{65465,16},{65466,16},{65467,16},{65468,16},{65469,16},{65470,16},{65471,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{503,9},{65472,16},{65473,16},{65474,16},{65475,16},{65476,16},{65477,16},{65478,16},{65479,16},{65480,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{504,9},{65481,16},{65482,16},{65483,16},{65484,16},{65485,16},{65486,16},{65487,16},{65488,16},{65489,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{505,9},{65490,16},{65491,16},{65492,16},{65493,16},{65494,16},{65495,16},{65496,16},{65497,16},{65498,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{506,9},{65499,16},{65500,16},{65501,16},{65502,16},{65503,16},{65504,16},{65505,16},{65506,16},{65507,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{2041,11},{65508,16},{65509,16},{65510,16},{65511,16},{65512,16},{65513,16},{65514,16},{65515,16},{65516,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{16352,14},{65517,16},{65518,16},{65519,16},{65520,16},{65521,16},{65522,16},{65523,16},{65524,16},{65525,16},{0,0},{0,0},{0,0},{0,0},{0,0},
		{1018,10},{32707,15},{65526,16},{65527,16},{65528,16},{65529,16},{65530,16},{65531,16},{65532,16},{65533,16},{65534,16},{0,0},{0,0},{0,0},{0,0},{0,0}
	};
	static const int YQT[] = {16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99};
	static const int UVQT[] = {17,18,24,47,99,99,99,99,18,21,26,66,99,99,99,99,24,26,56,99,99,99,99,99,47,66,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99};
	static const float aasf[] = { 1.0f * 2.828427125f, 1.387039845f * 2.828427125f, 1.306562965f * 2.828427125f, 1.175875602f * 2.828427125f, 1.0f * 2.828427125f, 0.785694958f * 2.828427125f, 0.541196100f * 2.828427125f, 0.275899379f * 2.828427125f };

	if(!data || !filename || !width || !height || comp > 4 || comp < 1 || comp == 2) {
		//return false;
		return 0;
	}

	
	/*FILE *fp = fopen(filename, "wb");
	if(!fp) {
		//return false;
		return 0;
	}*/
	FRESULT fr;     /* Return value */
	
	if (0 == imgBlock)
	{
	#if 0
		rtos_printf("\r\nJC: ROI(R,C,W,H): %d,%d,%d,%d", 0, 0, width, (height * NUM_IMAGE_BLOCKS));
		//FRESULT fr;     /* Return value */
		
		sprintf(newFileName, "IMG_FL_%s", (fName + strlen("IMG_")));
		strcpy(FTPBaseFilename, newFileName);
		//sprintf(newFileName, "/flash/fs/IMGP.JPG");
		rtos_printf("\r\nJC: %s Create.", newFileName);	
	#endif	
	
		rtos_fatfs_init_tast_call();
	
	#if 0
		if ((fr = f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE |FA_READ)/*fopen(filename, "wb")*/) != NULL) {	
			fprintf(stderr, "can't open %s\n", filename);
			//exit(1);
			return 0;
		}
		
		rtos_printf("\r\nJC: File '%s' Open Success.", filename);
	#endif	
		if ((fr = f_open(&fp, newFileName, FA_CREATE_ALWAYS | FA_WRITE |FA_READ)/*fopen(newFileName, "wb")*/) != NULL) {	
			fprintf(stderr, "can't open %s\n", newFileName);
			//exit(1);
			return 0;
		}
		
		rtos_printf("\r\nJC: File '%s' Open Success.", newFileName);
	}
	
	quality = quality ? quality : 90;
	int subsample = quality <= 90 ? 1 : 0;
	quality = quality < 1 ? 1 : quality > 100 ? 100 : quality;
	quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

	unsigned char YTable[64], UVTable[64];
	for(int i = 0; i < 64; ++i) {
		int yti = (YQT[i]*quality+50)/100;
		YTable[s_jo_ZigZag[i]] = yti < 1 ? 1 : yti > 255 ? 255 : yti;
		int uvti  = (UVQT[i]*quality+50)/100;
		UVTable[s_jo_ZigZag[i]] = uvti < 1 ? 1 : uvti > 255 ? 255 : uvti;
	}

	float fdtbl_Y[64], fdtbl_UV[64];
	for(int row = 0, k = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col, ++k) {
			fdtbl_Y[k]  = 1 / (YTable [s_jo_ZigZag[k]] * aasf[row] * aasf[col]);
			fdtbl_UV[k] = 1 / (UVTable[s_jo_ZigZag[k]] * aasf[row] * aasf[col]);
		}
	}

	// Write Headers
	static const unsigned char head0[] = { 0xFF,0xD8,  0xFF,0xE0,0,0x10,'J','F','I','F',0,1,1,0,0,1,0,1,0,0,0xFF,  0xDB,0,0x84,0 };
	UINT bw;
	
	unsigned char dataByte;
	
	if (0 == imgBlock)
	{
		//rtos_printf("\r\nJC: Write Headers.");
		//fwrite(head0, sizeof(head0), 1, fp);
		//fr = f_write(&fp, 1, sizeof(head0), head0);	
		fr = f_write(&fp, head0, sizeof(head0), &bw);	
		//rtos_printf("\r\nJC: Write YTable.");
		//fwrite(YTable, sizeof(YTable), 1, fp);
		//fr = f_write(&fp, 1, sizeof(YTable), YTable);
		fr = f_write(&fp, YTable, sizeof(YTable), &bw);	
		//putc(1, fp);
		//rtos_printf("\r\nJC: Write dataByte.");
		//unsigned char dataByte = 1;
		dataByte = 1;
		//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
		fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);	

		//rtos_printf("\r\nJC: Write UVTable.");
		//fwrite(UVTable, sizeof(UVTable), 1, fp);
		//fr = f_write(&fp, 1, sizeof(UVTable), UVTable);
		fr = f_write(&fp, UVTable, sizeof(UVTable), &bw);
		const unsigned char head1[] = { 0xFF,0xC0,0,0x11,8,(unsigned char)(height>>8),(unsigned char)(height&0xFF),(unsigned char)(width>>8),(unsigned char)(width&0xFF),3,1,(unsigned char)(subsample?0x22:0x11),0,2,0x11,1,3,0x11,1,0xFF,0xC4,0x01,0xA2,0 };
		//fwrite(head1, sizeof(head1), 1, fp);
		//fr = f_write(&fp, 1, sizeof(head1), head1);	
		fr = f_write(&fp, head1, sizeof(head1), &bw);
		//fwrite(std_dc_luminance_nrcodes+1, sizeof(std_dc_luminance_nrcodes)-1, 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_dc_luminance_nrcodes)-1, std_dc_luminance_nrcodes+1);	
		fr = f_write(&fp, std_dc_luminance_nrcodes+1, sizeof(std_dc_luminance_nrcodes)-1, &bw);
		//fwrite(std_dc_luminance_values, sizeof(std_dc_luminance_values), 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_dc_luminance_values), std_dc_luminance_values);	
		fr = f_write(&fp, std_dc_luminance_values, sizeof(std_dc_luminance_values), &bw);
		//putc(0x10, fp); // HTYACinfo
		dataByte = 0x10;
		//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
		fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);
		//rtos_printf("\r\nJC: Write HTYACinfo.");

		//fwrite(std_ac_luminance_nrcodes+1, sizeof(std_ac_luminance_nrcodes)-1, 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_ac_luminance_nrcodes)-1, std_ac_luminance_nrcodes+1);	
		fr = f_write(&fp, std_ac_luminance_nrcodes+1, sizeof(std_ac_luminance_nrcodes)-1, &bw);
		//fwrite(std_ac_luminance_values, sizeof(std_ac_luminance_values), 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_ac_luminance_values), std_ac_luminance_values);	
		fr = f_write(&fp, std_ac_luminance_values, sizeof(std_ac_luminance_values), &bw);
		//putc(1, fp); // HTUDCinfo
		dataByte = 1;
		//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
		fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);
		//rtos_printf("\r\nJC: Write HTUDCinfo.");
		
		//fwrite(std_dc_chrominance_nrcodes+1, sizeof(std_dc_chrominance_nrcodes)-1, 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_dc_chrominance_nrcodes)-1, std_dc_chrominance_nrcodes+1);	
		fr = f_write(&fp, std_dc_chrominance_nrcodes+1, sizeof(std_dc_chrominance_nrcodes)-1, &bw);
		//fwrite(std_dc_chrominance_values, sizeof(std_dc_chrominance_values), 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_dc_chrominance_values), std_dc_chrominance_values);	
		fr = f_write(&fp, std_dc_chrominance_values, sizeof(std_dc_chrominance_values), &bw);
		//putc(0x11, fp); // HTUACinfo
		dataByte = 0x11;
		//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
		fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);

		//fwrite(std_ac_chrominance_nrcodes+1, sizeof(std_ac_chrominance_nrcodes)-1, 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_ac_chrominance_nrcodes)-1, std_ac_chrominance_nrcodes+1);	
		fr = f_write(&fp, std_ac_chrominance_nrcodes+1, sizeof(std_ac_chrominance_nrcodes)-1, &bw);
		//fwrite(std_ac_chrominance_values, sizeof(std_ac_chrominance_values), 1, fp);
		//fr = f_write(&fp, 1, sizeof(std_ac_chrominance_values), std_ac_chrominance_values);	
		fr = f_write(&fp, std_ac_chrominance_values, sizeof(std_ac_chrominance_values), &bw);
		static const unsigned char head2[] = { 0xFF,0xDA,0,0xC,3,1,0,2,0x11,3,0x11,0,0x3F,0 };
		//fwrite(head2, sizeof(head2), 1, fp);
		//fr = f_write(&fp, 1, sizeof(head2), head2);	
		fr = f_write(&fp, head2, sizeof(head2), &bw);
		//rtos_printf("\r\nJC: Write Head2.");
	}
	
	// Encode 8x8 macroblocks
	int ofsG = comp > 1 ? 1 : 0, ofsB = comp > 1 ? 2 : 0;
	const unsigned char *dataR = (const unsigned char *)data;
	const unsigned char *dataG = dataR + ofsG;
	const unsigned char *dataB = dataR + ofsB;
	int DCY=0, DCU=0, DCV=0;
	int bitBuf=0, bitCnt=0;
	
	//rtos_printf("\r\nJC: Encode 8x8 macroblocks.");
	if(subsample) {
		for(int y = 0; y < height; y += 16) {
			for(int x = 0; x < width; x += 16) {
				float Y[256], U[256], V[256];
				for(int row = y, pos = 0; row < y+16; ++row) {
					for(int col = x; col < x+16; ++col, ++pos) {
						int prow = row >= height ? height-1 : row;
						int pcol = col >= width ? width-1 : col;
						int p = prow*width*comp + pcol*comp;
						float r = dataR[p], g = dataG[p], b = dataB[p];
						Y[pos]=+0.29900f*r+0.58700f*g+0.11400f*b-128;
						U[pos]=-0.16874f*r-0.33126f*g+0.50000f*b;
						V[pos]=+0.50000f*r-0.41869f*g-0.08131f*b;
					}
				}
				DCY = jo_processDU(&fp, bitBuf, bitCnt, Y+0, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
				DCY = jo_processDU(&fp, bitBuf, bitCnt, Y+8, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
				DCY = jo_processDU(&fp, bitBuf, bitCnt, Y+128, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
				DCY = jo_processDU(&fp, bitBuf, bitCnt, Y+136, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
				// subsample U,V
				float subU[64], subV[64];
				for(int yy = 0, pos = 0; yy < 8; ++yy) {
					for(int xx = 0; xx < 8; ++xx, ++pos) {
						int j = yy*32+xx*2;
						subU[pos] = (U[j+0] + U[j+1] + U[j+16] + U[j+17]) * 0.25f;
						subV[pos] = (V[j+0] + V[j+1] + V[j+16] + V[j+17]) * 0.25f;
					}
				}
				DCU = jo_processDU(&fp, bitBuf, bitCnt, subU, 8, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
				DCV = jo_processDU(&fp, bitBuf, bitCnt, subV, 8, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
			}
		}
	} else {
		for(int y = 0; y < height; y += 8) {
			for(int x = 0; x < width; x += 8) {
				float Y[64], U[64], V[64];
				for(int row = y, pos = 0; row < y+8; ++row) {
					for(int col = x; col < x+8; ++col, ++pos) {
						int prow = row >= height ? height-1 : row;
						int pcol = col >= width ? width-1 : col;
						int p = prow*width*comp + pcol*comp;
						float r = dataR[p], g = dataG[p], b = dataB[p];
						Y[pos]=+0.29900f*r+0.58700f*g+0.11400f*b-128;
						U[pos]=-0.16874f*r-0.33126f*g+0.50000f*b;
						V[pos]=+0.50000f*r-0.41869f*g-0.08131f*b;
					}
				}
				DCY = jo_processDU(&fp, bitBuf, bitCnt, Y, 8, fdtbl_Y, DCY, YDC_HT, YAC_HT);
				DCU = jo_processDU(&fp, bitBuf, bitCnt, U, 8, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
				DCV = jo_processDU(&fp, bitBuf, bitCnt, V, 8, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
			}
		}
	}
	
	// Do the bit alignment of the EOI marker
	/*static const unsigned short fillBits[] = {0x7F, 7};
	jo_writeBits(fp, bitBuf, bitCnt, fillBits);
	putc(0xFF, fp);
	putc(0xD9, fp);	
	fclose(fp);*/
	
	if (imgBlock >= (NUM_IMAGE_BLOCKS - 1))
	{
		printf("\r\nJC: Jpeg Compression Completed. %d", (imgBlock + 1));
	}
	else
	{
		//rtos_printf("\r\nJC: Jpeg Compression In Progress. %d", (imgBlock + 1));
		return 0;
	}		
/*
	//----------------------------------------------
	//TBD //only for testing //--to be Removed
	//fseek(outfile, 0L, SEEK_END);  
	long int jpegFileSize = f_size(&outfile);//36;//ftell(outfile);  	// calculating the size of the file
	printf("\r\nJC: Jpeg File Size: %lu", jpegFileSize);	
	//----------------------------------------------
*/	
	//printf("\r\nJC: end outfile: %ld\n", outfile);

	/* After finish_compress, we can close the output file. */
	static const unsigned short fillBits[] = {0x7F, 7};
	jo_writeBits(&fp, bitBuf, bitCnt, fillBits);
	//putc(0xFF, fp);
	dataByte = 0xFF;
	//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
	fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);
	//putc(0xD9, fp);
	dataByte = 0xD9;
	//fr = f_write(&fp, 1, sizeof(dataByte), &dataByte);
	fr = f_write(&fp, &dataByte, sizeof(dataByte), &bw);

	//----------------------------------------------
	//TBD //only for testing //--to be Removed
	//fseek(outfile, 0L, SEEK_END);  
	long int jpegFileSize = f_size(&outfile);//36;//ftell(outfile);  	// calculating the size of the file
	printf("\r\nJC: Jpeg File Size: %lu", jpegFileSize);
	//----------------------------------------------

	f_close(&fp);	
	
	return 1;
}

#endif

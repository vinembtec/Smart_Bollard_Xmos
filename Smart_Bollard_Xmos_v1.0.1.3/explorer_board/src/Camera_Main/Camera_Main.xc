// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <syscall.h>
#include <xs1.h>
#include <xs3a_registers.h>
#include <print.h>
#include <stdio.h>
#include <stdint.h>
#include <platform.h>
#include <math.h>
#include "mipi.h"
#include <stdlib.h>
#include <string.h>
#include <xscope.h>
#include <xclib.h>
//#include "leds.h"
#include "optMemCpy.h"
//#include "host_mcu_uart.h"
#include "Camera_Main.h"
//#include "jpeg_c_lib_test.h"
#include "host_mcu/host_mcu_interface.h"
//#include "protocol_frame_host_mcu.h"
#include "host_mcu/host_mcu_types.h"

#include "xmos_config/xmos_config.h"
//#include "host_mcu/protocol_data_host_mcu.h"
//#include <xcore/port.h>
//#include <rtos_gpio.h>
//#include "../image_deletion/image_deletion.h"
//#include "image_deletion//image_deletion.h"

//#include "intertile_comm/intertile_comm_tile0.h"
//#include "intertile_comm/intertile_comm_tile1.h"

#define JPEG_SEPERATE_CORE TRUE
#define SINGLE_CAPTURE	   TRUE

#define TRUE 1
#define FALSE 0

#define IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH  101

#define INSERT_DELAY_BETWEEN_IMAGES (0)
#define PRINT_DEBUG_MIPI_LPDDR_SHIFTED_DATA (1)
#define CONTINUOUS_IMAGE_CAPTURE (0)

#define MAX_IMAGES (20)	

#define MAX_IMAGES_PER_CAMERA (FRAMES_IN_DDR -1)
#define SRAM_BUF_ROWS (8)

#define MIPI_OUTPUT_ENABLE 0x00000040	
#define MIPI_SW_SELECT     0x00000080

    #define MIPI_CLK_DIV (1)//MIPI DEMUX CLK = (SYSCLK / 2) / (x+1) -- 1 for 600MHz 
    #define MIPI_CFG_CLK_DIV (2)		//MIPI PHY CLK = 100MHz	    -- 2 for 600MHz 
#if 0	
#ifdef CLK_600
    #define MIPI_CLK_DIV (1)//MIPI DEMUX CLK = (SYSCLK / 2) / (x+1) -- 1 for 600MHz 
    #define MIPI_CFG_CLK_DIV (2)		//MIPI PHY CLK = 100MHz	    -- 2 for 600MHz 
#elif CLK_800
    #define MIPI_CLK_DIV (1)//MIPI DEMUX CLK = (SYSCLK / 2) / (x+1)   -- safe: 2, 3, 4 
    #define MIPI_CFG_CLK_DIV (3)		//MIPI PHY CLK = 100MHz		  -- safe: 3, 4    
#else
    #error "only 600 and 800 MHz clocks supported"
#endif
#endif

#define DEMUX_DATATYPE 0
#define DEMUX_MODE     0x80     // bias
#define DEMUX_EN       0

#define TEST_DEMUX_DATATYPE MIPI_RAW12	//RAW12  
#define TEST_DEMUX_MODE     XMIPI_DEMUXMODE_12TO16//XMIPI_DEMUXMODE_12TO16 //RAW12 Mode 
#define TEST_DEMUX_EN       (1)  
#define DELAY_MIPI_CLK      (1)
#define LANES (2)

#ifndef MIPI_TILE
    #define MIPI_TILE (1)
	//#define MIPI_TILE (0)
#endif

#define DDR_START_ADDR_CAM1 (0x10000000 + ((SENSOR_IMAGE_HEIGHT) * (STRIDE) ))
#if (((DDR_START_ADDR_CAM1)%64)!=0)
#error DDR CAM1 start address must be a multiple of 64
#endif 

#define DDR_START_ADDR_CAM2 ((DDR_START_ADDR_CAM1) + ((FRAME_SIZE) * (FRAMES_IN_DDR)) + 64)
#if (((DDR_START_ADDR_CAM2)%64)!=0)
#error DDR CAM2 start address must be a multiple of 64
#endif 

#define DDR_START_ADDR_JPEG_CAM1 ((DDR_START_ADDR_CAM2) + ((FRAME_SIZE) * (FRAMES_IN_DDR)) + 64)
#if (((DDR_START_ADDR_JPEG_CAM1)%64)!=0)
#error DDR CAM2 start address must be a multiple of 64
#endif 

#define DDR_START_ADDR_JPEG_CAM2 ((DDR_START_ADDR_JPEG_CAM1) + ((FRAME_SIZE) * (2)) + 64)
#if (((DDR_START_ADDR_JPEG_CAM2)%64)!=0)
#error DDR CAM2 start address must be a multiple of 64
#endif 

//#if INSERT_DELAY_BETWEEN_IMAGES
unsigned char vehicleOccupancyStatusIndication = VEHICLE_OCCUPANCY_IDLE_STATE;
int delayBetweenImagesVehicleEntry = 1500;
int delayBetweenImagesVehicleExit = 1500;
//#endif //INSERT_DELAY_BETWEEN_IMAGES

on tile[MIPI_TILE]:buffered in port:32 p_mipi_clk = XS1_PORT_1O;
on tile[MIPI_TILE]:in port p_mipi_rxa = XS1_PORT_1E;
on tile[MIPI_TILE]:in port p_mipi_rxv = XS1_PORT_1I;
on tile[MIPI_TILE]:buffered in port:32 p_mipi_rxd = XS1_PORT_8A;
on tile[MIPI_TILE]:in port p_mipi_rxs = XS1_PORT_1J;
on tile[MIPI_TILE]:clock clk_mipi = XS1_CLKBLK_1;
//on tile[MIPI_TILE]:out port imageCaptureTimePin = XS1_PORT_4C; //declaration
on tile[MIPI_TILE]: port p_reset_camera = XS1_PORT_1P;	

//on tile[MIPI_TILE]:out port MIPI_SWITCH_CONTROL_PINS = XS1_PORT_8A; //MIPI_SW_OE_8A6_O X1D08   //MIPI_SW_SEL_8A7_O X1D09
on tile[MIPI_TILE]:out port MIPI_SW_OE_PIN  = XS1_PORT_1D;// = XS1_PORT_1B; //MIPI_SW_OE X1D01
on tile[MIPI_TILE]:out port MIPI_SW_SEL_PIN = XS1_PORT_1B;// = XS1_PORT_1D; //MIPI_SW_OE X1D10on tile[MIPI_TILE]:out port MIPI_SW_SEL_PIN = XS1_PORT_1B;
on tile[MIPI_TILE]:out port GPIO_DEBUG_PIN1 = XS1_PORT_1F;
on tile[MIPI_TILE]:out port GPIO_DEBUG_PIN2 = XS1_PORT_1G;

//on tile[MIPI_TILE]:out port LEFT_CAM_TRIGGER_PIN = XS1_PORT_1E; //MIPI_SW_OE X1D12
////on tile[MIPI_TILE]:out port RIGHT_CAM_TRIGGER_PIN = XS1_PORT_1H; //MIPI_SW_OE X1D23

////extern port UART_4BIT_TXD;
extern port PORT_LEDS_X1D4C;//on tile[MIPI_TILE]: port PORT_LEDS_X1D4C = XS1_PORT_4C;

unsafe{
decoupler_buffer_t * unsafe decoupler_r_cam1 = (decoupler_buffer_t *) DDR_START_ADDR_CAM1;
decoupler_buffer_t * unsafe decoupler_r_cam2 = (decoupler_buffer_t *) DDR_START_ADDR_CAM2;
}

/*unsafe{
	CompressedImage_t * unsafe compressedImageCam1 = (CompressedImage_t *) DDR_START_ADDR_JPEG_CAM1;
	CompressedImage_t * unsafe compressedImageCam2 = (CompressedImage_t *) DDR_START_ADDR_JPEG_CAM2;
}*/

uint8_t imageTriggerPinState = 0; //added on 14_07_2022

uint16_t totalNumCompressedImagesDdrCam1 = 0;
uint16_t totalNumCompressedImagesDdrCam2 = 0;
//extern unsigned long outfileCount;

extern char newFileName[200];
char TmpStr[50];

void exit(int);

extern int x[];	// this ensures extmem gets initialized

//void send_array(chanend c, uint32_t * unsafe array, unsigned size);

uint32_t lineCountError = 0; 
uint32_t pixelCountError = 0;
uint64_t pixelCounter = 0;
uint8_t my_SRAM_buffer[SRAM_BUF_ROWS][STRIDE];	//12 d-words added to make 64bit aligned //added on 11_03_2022 AC /4 added MAR18


char fName[200];

//-----------------------------------------------------------------------
//TODO:: Camera Number to be taken from RTC Packet when receives from MSP.
//int iSelectedCamera;
int iSelectedCamera = 1;
//int iSelectedCamera = 2;
//-----------------------------------------------------------------------
uint8_t sameCameraDetected = 0;
unsigned int cameraCount = 0;
uint32_t mipiSwitchControlPins = 0;

uint16_t First_Raw_Data[10][50];
uint16_t Second_Raw_Data[10][50];

/*
uint8_t exitLoopUponAnotherCameraDetected = 0;
uint8_t executeBrighterDataImageTask = 0;

uint8_t jpegFullSizeImageCount = 0;
uint8_t jpegNumberPlateSizeImageCount = 0;
uint8_t previousJpegFullSizeImageCount = 0;
uint8_t previousJpegNumberPlateSizeImageCount = 0;

uint32_t jpegFullSizeImageNumbers = 0;
uint32_t jpegNumberPlateSizeImageNumbers = 0;
uint32_t previousJpegFullSizeImageNumbers = 0;
uint32_t previousJpegNumberPlateSizeImageNumbers = 0;

uint16_t previousCompletedAnprNumImages = 0;
uint16_t previousResidualAnprNumImages = 0;
uint16_t previousCompletedJpegNumImages = 0;
uint16_t previousResidualJpegNumImages = 0;
uint16_t previousTotalJpegImages = 0;
uint16_t totalCapturedNumImages = 0;

uint32_t currentBrighterDataImages = 0;
uint32_t previousBrighterDataImages = 0;
uint32_t brighterDataImages = 0;

char currentFileNameBase[200];
char previousFileNameBase[200];
*/

ImageInfo_t imageCapture;
ImageInfo_t imageCaptureTile0;

ImageInfo_t imageCaptureTemp;

int createFileName = 1;
int imageCaptureCount = 0;

//-----------------------------------------------------------------------
//TODO:: Camera Number to be taken from RTC Packet when receives from MSP.
int year = 2022;
int month = 1;
int date = 1;
int hour = 10;
int minute = 0;
int second = 0;
uint8_t capture_sequence_number;
//-----------------------------------------------------------------------

int writeColorJpeg;
int iAnprEnable = 0;
int iCustomerId;
int iLeftMID;
int iRightMID;
int iEnableBrighterDataImageConfigPara;

extern "C" {
	int ImageDeletion_processVehicleOccupancyStatusInfo(unsigned char *buffer, unsigned short length);
	//int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(void);
	//int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(chanend_t txCmd);
}

extern "C" {
	
	void JpegCompress_blockWise(int imgBlock, unsigned short *rawBuffer, int startRow, int startColumn, int width, int height, uint8_t currentSession);
	void RawFile_create(int imgBlock, unsigned short *rawBuffer);
	//unsigned char jo_write_jpg(const char *filename, const void *data, int width, int height, int comp, int quality);
	unsigned char jo_write_jpg(int imgBlock, const char *filename, const void *data, int width, int height, int comp, int quality);
	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	extern void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 );
	extern void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 );
	extern void Debug_Display( int8_t minimumDebugLevel, const char * pszText );
	//////////////////////////////////	
	
	#define PORT_LEDS_X1D4C0 0
	#define PORT_LEDS_X1D4C1 1
	#define PORT_LEDS_X1D4C2 2
	#define PORT_LEDS_X1D4C3 3

	extern void setGpio(uint8_t position);
	extern void resetGpio(uint8_t position);
	extern int readGpio(void);
	
	//extern void HostMcuInterface_sendDataPacket(unsigned char packetType);
}

extern void start_FreeRTOS_Task(void);
extern void HostMcuUart_ReceiveData(chanend c_decoupler, chanend interTileInstance, chanend txd_rxd_channel,chanend seq_c);
extern void HostMcuUart_txData(chanend tile0TxCmd, chanend tile1TxCmd, chanend interTileInstance, chanend JPEG_Status_Send, chanend JPEGinterTileInstance, chanend txd_rxd_channel,chanend interTileInstance_new);

#define MAX_IMG_ROWS (1088)
#define MAX_IMG_COLS (1928)
#define FULL_IMAGE_SIZE (MAX_IMG_ROWS * MAX_IMG_COLS)
//#define NUM_IMAGE_BLOCKS 136
#define NUM_IMAGE_BLOCKS 544
#define IMAGE_BLOCK_SIZE (FULL_IMAGE_SIZE / NUM_IMAGE_BLOCKS)
#define NUM_ROWS_PER_IMAGE_BLOCK (IMAGE_BLOCK_SIZE / MAX_IMG_COLS)

unsigned short rawBuffer[IMAGE_BLOCK_SIZE * 3];
extern unsigned char txCommand_to_tile0;
uint8_t currentSession_global = 0;
extern uint8_t bootUpFlag;

void camera_main(chanend txCmd, chanend instance[]);
void capture_thread(chanend instance[]);
//void HostMcuUart_testIntertile(chanend txCmd);
void InterTileCommTile0_sendPacketToHostMcuViaTile1(chanend txCmd, unsigned char packetType);
void InterTileCommTile0_waitResponseFromHostMcuViaTile1(chanend txCmd, unsigned char packetType, unsigned char rxStatus);
void InterTileCommTile1_sendResponse(chanend txCmd, unsigned char packetType, unsigned char status, unsigned char *data);
//void InterTileCommTile1_txInstance(chanend txInstance, unsigned char instance);
void InterTileCommTile1_txInstance(chanend txInstance, ImageInfo_t * unsafe imageCap, unsigned char instance);
//void InterTileCommTile0_rxInstance(chanend sensorTriggerInstance, chanend imgCaptureDone, chanend jpegDone, chanend txCmd);
void InterTileCommTile0_rxInstance(chanend instance[]);
//int InterTileCommTile0_rxInstance_seq_num(chanend c[]);
//void InterTileCommTile1_txInstance_seq_num(chanend c[], int seq_num);
//uint8_t get_seq_num();

//VehicleOccupancyPacket_t tile0VehicleOccupancyPacket;

//extern VehicleOccupancyPacket_t vehicleOccupancyPacket;
//#if ON_TILE(0)
//extern int ImageDeletion_processVehicleOccupancyStatusInfo(uint8_t *buffer, uint16_t length);
//extern int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(void);
//extern int ImageDeletion_processVehicleOccupancyStatusInfo(unsigned char *buffer, unsigned short length);
//extern int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(chanend txCmd);

//#endif 

extern void rtos_fatfs_init_tast_call(void);

uint32_t frame_time = 0, line_time = 0;

// Start user code

#pragma unsafe arrays
void MipiDecoupler(chanend c, chanend c_kill, chanend c_line)
{	
    unsigned tailSize, ourWordCount, mipiHeader;
    int line = 0; // TODO changing this affects the "row 2 issue"

	uint8_t set_First = 0;
	uint8_t linesSaved = 0;
	uint8_t imageCount = 0;

	set_core_high_priority_on();
	//Debug_Output2(0,"entered to saving data dest = 0X%lX , pt = 0X%lX  ",DDR_START_ADDR_CAM1,DDR_START_ADDR_CAM2);
	unsafe
	{
    while(1)
    {
        // Send out a buffer pointer to receiver thread 
        //unsafe{
            uint8_t * unsafe pt = my_SRAM_buffer[line];
            outuint(c, (unsigned) pt);
        //}
        /* Packet receive notification - header */
        mipiHeader = inuint(c);
#if 0		
		int headerTemp = mipiHeader & 0x3f;
        if (MIPI_FRAMESTART == headerTemp) // End of frame
             {
				set_First = 0;
				//line = 0;
				//printstr("End data from MipiDecoupler");
			 }		
        else if (MIPI_FRAMEEND == headerTemp) // End of frame
             {
				printstr("\r\n");
				printstr("First data MIPIDECOUPLER");
				printstr("\r\n");	
				printint(line);
				printstr("\r\n");				
				for(uint8_t v=0;v<50;v++)
				{
					printhex(Second_Raw_Data[imageCount][v]); 	 	
					printstr(" ");					
				}
				printstr("\r\n");
				
				imageCount++;
				set_First = 0;
				linesSaved = 0;
				//line = 0;
				//printstr("End data from MipiDecoupler");
			 }
		else if(MIPI_RAW12 == headerTemp)
			{
				//if(imageCount == (endImageIndex-1))
				{
				 if(set_First == 0)Second_Raw_Data[(imageCount)][linesSaved] = *pt;
				 if(linesSaved > 49)set_First = 1;
				 linesSaved++;
				}			
			}
#endif			
        outuchar(c_line, mipiHeader);
        line = (line + 1) & (SRAM_BUF_ROWS-1);			 
        /* Long packet */
        if(mipiHeader & 0x30) {
            ourWordCount = inuint(c);
            tailSize = inuint(c);
            if (ourWordCount != (SENSOR_IMAGE_WIDTH/2) && tailSize != 0)
            {
                #if TEST_DEMUX_EN
                    printintln(ourWordCount);
                    printintln(tailSize);
                #else
                    #warning DEMUX is not enabled!
                #endif
                pixelCountError++;
            }
        }
    }
	}
}

unsigned char selectedCamera = 1;
unsigned char totCapImages = FRAMES_IN_DDR; //added on 05_07_2022

// Port SET PAD CTRL Control Word definition
// Bit - Meaning
// 23 - Enable schmitt trigger when set to 1.
// 22 - Slew rate limiting enabled when set to 1.
// [21:20] - Sets Drive Strength - 00 - 2mA, 01 - 4mA, 10 - 8mA, 11 - 12mA.
// [19:18] - Sets the pull resistor options. 00 - none, 01 - pull up enabled, 10 - pull down enabled, 11 buskeeper.
// 17 - REN - Receiver enable. Set to 1 to enable receiver. Might set this to 0 to save power for floating IOs or just in general.
// 16 - Not used.
// [15:0] - Mode Bits - 0x0006 - This defines a SETPADCTRL write

#define PORT_PAD_CTL_PULLDOWN_4mA 0x001A0006
#define PORT_PAD_CTL_PULLUP_4mA   0x00160006
#define PORT_PAD_CTL_PULLNONE_4mA 0x00120006

#define PORT_PAD_CTL_PULLDOWN_12mA 0x003A0006
#define PORT_PAD_CTL_PULLUP_12mA   0x00360006
//#define PORT_PAD_CTL_PULLNONE_12mA 0x00320006
#define PORT_PAD_CTL_PULLNONE_12mA 0x00B20006



uint8_t set_First = 0;
uint8_t set_Second = 0;	

#pragma unsafe arrays
void MipiImager(chanend c_line, chanend c_decoupler, chanend txCmd/*chanend ?c_decoupler2 chanend c_l0*/, chanend ?c_imgCount)
{
	int line = 0;
    int lineCount = 0;
    int linesSaved = 0;
    int grabbing = 0;
    uint8_t new_grabbing = 0;
    int width = SENSOR_IMAGE_WIDTH;
    int imageCount = 0;
	uint8_t generateCaptureImageTrigger = 1;
	int start_x = 0;
	//int end_x = 1928;
	int start_y = 0;
	int end_y = SENSOR_IMAGE_HEIGHT;	
	unsigned char imgTriggerType = 0;
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	unsigned captureValid = 0;
	
	int last_sof = 0, last_line = 0, now = 0;

	unsigned int header_count = 0;
	unsigned int header_count_acutal = 0;
	
	uint8_t properImage = 0;
	uint8_t ErrorImageCount = 0;

	uint8_t gpio_debug_toggle = 0;
	timer t;
	int time;
	uint8_t timerSet = 0;
	uint8_t allImagesCaptured = 0;

	set_core_high_priority_on();
	//port_enable(XS1_PORT_1H);
	//port_write_control_word(MODE_SETPADCTRL, 0x00300000);
	//write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_DDR_CLK_DIVIDER_NUM, 0);	//LPDDR Clock Divisor = 0
	
	printf("\r\nMipiImager.");		
	//printf("\r\nMI: X: (%d,%d) Y: (%d,%d)", start_x, (SENSOR_IMAGE_WIDTH), start_y, end_y);
	//printf("\r\nMI: Ngrab");
	
	//asm volatile ("setc res[%0], %1" :: "r" (RIGHT_CAM_TRIGGER_PIN), "r" (PORT_PAD_CTL_PULLNONE_4mA));	
	////asm volatile ("setc res[%0], %1" :: "r" (RIGHT_CAM_TRIGGER_PIN), "r" (PORT_PAD_CTL_PULLNONE_12mA)); //this was selected
	//asm volatile ("setc res[%0], %1" :: "r" (RIGHT_CAM_TRIGGER_PIN), "r" (PORT_PAD_CTL_PULLDOWN_12mA));
	//asm volatile ("setc res[%0], %1" :: "r" (RIGHT_CAM_TRIGGER_PIN), "r" (PORT_PAD_CTL_PULLUP_12mA));
	//outuchar(txCmd, HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);

		//delay_milliseconds(1000);
		//HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
		//outuchar(txCmd, HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
		//delay_milliseconds(1000);	
	unsafe{	
	        ////uint8_t * unsafe pt = my_SRAM_buffer[line];
			uint8_t * unsafe camPtr = &selectedCamera;
			uint8_t * unsafe totCapImagesPtr = &totCapImages; //added on 05_07_2022
            ////uint8_t header_byte;

			uint16_t startImageIndex = 1;
			//---------------------------------------------------------------
			//TODO:: Number of Images to be captured to be taken from config.
			uint16_t endImageIndex = FRAMES_IN_DDR;	
			endImageIndex = *totCapImagesPtr; //added on 05_07_2022
			//---------------------------------------------------------------	
			//int header;
		
	    while (1)
        {
	        uint8_t * unsafe pt = my_SRAM_buffer[line];
			uint8_t header_byte;
            
		    /*if (1 == generateCaptureImageTrigger)
		    {
			    generateCaptureImageTrigger = 0;
			    outuchar(c_imgCount, 1);
		    }*/
			
			/*
			if (new_grabbing)		//TBD //--to be commented //added 08_06_2022
			{
				outuchar(c_decoupler, new_grabbing);
			}*/

#if SINGLE_CAPTURE
			if((captureValid == 1)/*&&(properImage == 1)*/)
			{
				captureValid = 0;
#if 0				
			if(set_First)
			{			
				printstr("\r\n");
				printstr("First data");
				printstr("\r\n");
				printint(line);
				printstr("\r\n");	
				for(uint8_t v=0;v<50;v++)
				{
					printhex(First_Raw_Data[(imageCount-2)][v]); 	 	
					printstr(" ");					
				}
				printstr("\r\n");
			}
#endif
#if 1									
								if (imageCount == endImageIndex){

#if 1
									/* Disable MIPI SW OE, MIPI SW SEL, Image Captured Completed */
									//if (imageCount == FRAMES_IN_DDR)
									if (imageCount == endImageIndex)
									{
										MIPI_SW_OE_PIN <: 1;	//Set 
										MIPI_SW_SEL_PIN <: 0;	//Clear																	
										/* 
										Left Cam trigger pin needed to remap 
										but current XMOS board 1.8V GPIO is not available on TILE[1], 
										so shorted Right and Left Cam trigger pins, 
										so that Camera streaming ON/OFF of both camera's can be 
										controlled using only one GPIO because one camera will be 
										ON/Active at a time.
										*/
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{											
											//LEFT_CAM_TRIGGER_PIN <: 0; //Clear
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear
											resetGpio(PORT_LEDS_X1D4C2);
										}
									}	
#endif
									#if 1 //CONTINUOUS_IMAGE_CAPTURE
										/*printf("\r\nMI: decoupler_r_cam1: \n");
										for(int frame=0; frame < FRAMES_IN_DDR-1; frame++){
											for (int row = 0; row < 20; row++)
											{
												printf("\r\nMI: F[%02d].R[%02d]: ", frame, row);
												for (int column = 0; column < 50; column+=2)
												{									
													unsafe{
														printf("%02X", decoupler_r_cam1->frames[frame][row*STRIDE+column+1]);
														printf("%02X ", decoupler_r_cam1->frames[frame][row*STRIDE+column]);
													}
												}
											}
										}*/
										imageCount = 0;
										new_grabbing = 0;	//TBD //added on 21_06_2022
									#endif //CONTINUOUS_IMAGE_CAPTURE

								
									if(properImage == 1)HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
									Debug_Output1(0," Total header Counter = %d",header_count_acutal);
									allImagesCaptured = 1;
									
									outuchar(txCmd, HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE);	//added on 20_06_2022
                                    #if PRINT_DEBUG_MIPI_LPDDR_SHIFTED_DATA
										printstr("\r\nMI: Triggering Jpeg Create Task. End");
										outuchar(c_imgCount, 0x5A);
                                    #endif
									header_count = 0;
									header_count_acutal = 0;
									//return; //commented on 20_06_2022
                                }
								else if(imageCount > startImageIndex)
								{
																	
					//This is to Turn Off Camera Power for battery consumption				
					#if 0				
					if(properImage == 1)
					{
						Debug_TextOut(0, "Triggering off camera");				
						HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
					}
					#endif									
									#if PRINT_DEBUG_MIPI_LPDDR_SHIFTED_DATA
										//if(properImage == 1)printstr("\r\nMI: Triggering Jpeg Create Task. Start");
                                        //if(properImage == 1)outuchar(c_imgCount, 0x5A);
                                    #endif
#if 1
if(properImage == 1)
{
										if ((delayBetweenImagesVehicleEntry > 0) && (VEHICLE_OCCUPANCY_ENTRY_STATE == vehicleOccupancyStatusIndication))
										{
											delay_milliseconds(delayBetweenImagesVehicleEntry);
										}
										else if ((delayBetweenImagesVehicleExit > 0) && (VEHICLE_OCCUPANCY_EXIT_STATE == vehicleOccupancyStatusIndication))
										{
											delay_milliseconds(delayBetweenImagesVehicleExit);
										}
}										
#endif
									
					//This is to Turn On Camera Power for battery consumption				
					#if 0				
					if(properImage == 1)
					{
						Debug_TextOut(0, "Triggering on camera");				
						HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE);
					}
					#endif					

					properImage = 0;
#if 0					
					/* select MIPI_SW_SEL and set MIPI_SW_OE as per camera trigger	*/
					/* Disable MIPI SW OE, MIPI SW SEL */
					MIPI_SW_OE_PIN <: 1;	//Set 
					MIPI_SW_SEL_PIN <: 0;	//Clear

					if (1 == *camPtr)
					{
						/* Enable MIPI SW OE, MIPI SW SEL=1 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 1;	//Set						
					}
					else if(2 == *camPtr)
					{
						/* Enable MIPI SW OE, MIPI SW SEL=0 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 0;	//Clear
					}
#endif					
					////delay_milliseconds(10);
					
									if (imageCount < endImageIndex)
									{
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{
											//LEFT_CAM_TRIGGER_PIN <: 1; //Set
											////RIGHT_CAM_TRIGGER_PIN <: 1; //Set
											setGpio(PORT_LEDS_X1D4C2);
											
											delay_microseconds(1);
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear */
											resetGpio(PORT_LEDS_X1D4C2);
										}
									}																		
								}
								else if ((imageCount < endImageIndex)&&(header_count > 0))
								{
#if 0
					/* select MIPI_SW_SEL and set MIPI_SW_OE as per camera trigger	*/
					/* Disable MIPI SW OE, MIPI SW SEL */
					MIPI_SW_OE_PIN <: 1;	//Set 
					MIPI_SW_SEL_PIN <: 0;	//Clear
					
					if (1 == *camPtr)
					{
						/* Enable MIPI SW OE, MIPI SW SEL=1 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 1;	//Set						
					}
					else if(2 == *camPtr)
					{
						/* Enable MIPI SW OE, MIPI SW SEL=0 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 0;	//Clear
					}
#endif					
					////delay_milliseconds(10);
					
										////printstr("\r\nMI: Triggering Jpeg Create Task. Temp");
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{
											//LEFT_CAM_TRIGGER_PIN <: 1; //Set
											////RIGHT_CAM_TRIGGER_PIN <: 1; //Set 
											setGpio(PORT_LEDS_X1D4C2);
											
											delay_microseconds(1);
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear */
											resetGpio(PORT_LEDS_X1D4C2);
										}
								}
#endif				
			}				

#endif
            if(timerSet == 0)
			{
				Debug_TextOut(0, "Before select block, starting timer");
				timerSet = 1;
				t :> time;
				time += 3000*1000*100; // 3000 ms

			}
            select {
                case inuchar_byref(c_line, header_byte):
				    int header = header_byte & 0x3f;	
				    line = (line + 1) & (SRAM_BUF_ROWS-1);
				    
                    if (MIPI_FRAMESTART == header)      // Start of frame
                    {
						GPIO_DEBUG_PIN1 <: 0;
						GPIO_DEBUG_PIN2 <: 0;
						////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear */
						resetGpio(PORT_LEDS_X1D4C2);
						
						header_count++;
						header_count_acutal++;
						////setGpio(PORT_LEDS_X1D4C0);	
						////PORT_LEDS_X1D4C/*XS1_PORT_4C*/ <: 0x09;
                        lineCount = 0;
						set_First = 0;
                        grabbing = new_grabbing;
                        new_grabbing = 0;
						ErrorImageCount = 0;

						if((header_count % 3) == 0)properImage = 1;
						////Debug_TextOut(0,"S");
						////printstr("S");
                    } 
                    else if (MIPI_FRAMEEND == header) // End of frame
                    {
						////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear */
						GPIO_DEBUG_PIN1 <: 1;
						GPIO_DEBUG_PIN2 <: 1;

						resetGpio(PORT_LEDS_X1D4C2);
						
						////resetGpio(PORT_LEDS_X1D4C0);
						captureValid = 1;
						
						new_grabbing = 1;
						grabbing = 0;
						////line = 0;
						
						////PORT_LEDS_X1D4C/*XS1_PORT_4C*/ <: 0x00;						
                        if (lineCount != SENSOR_IMAGE_HEIGHT)
                        {
                            lineCountError++;
							//printstr("LErr");							
                            printintln(lineCountError);
                            printintln(lineCount);
                        }
						////Debug_TextOut(0,"E");
						////printstr("E");
					    pixelCounter += (SENSOR_IMAGE_WIDTH*SENSOR_IMAGE_HEIGHT);
                    } 
				    else if (MIPI_RAW12 == header) // RAW12 BIT	
                    {   
						////Debug_TextOut(0,"R");
						//printstr("MIPI_RAW12");
						if(gpio_debug_toggle == 0)
						{
							GPIO_DEBUG_PIN1 <: 1;
							GPIO_DEBUG_PIN2 <: 0;
							gpio_debug_toggle = 1;
						}
						else
						{
							GPIO_DEBUG_PIN1 <: 0;
							GPIO_DEBUG_PIN2 <: 1;
							gpio_debug_toggle = 0;
						}					

					    if ((grabbing == TRUE) &&
                            lineCount >= start_y &&
                            lineCount < end_y) 
                        {
							uint32_t dest;							
							////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear */
							resetGpio(PORT_LEDS_X1D4C2);
							//properImage = 1;
							
							/*if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)	//TBD	//added on 15_07_2022 //--only for testing //--to be Removed
							{
								startImageIndex = 0;
							}*/
													
                            /* Skip First and Last Image as these are bad images */
							//if ((imageCount >= 1) || (imageCount == FRAMES_IN_DDR)){
							if ((imageCount >= startImageIndex) || (imageCount == endImageIndex))		//TBD //commented on 12_07_2022 //--only for testing //--to be UNcommented
							//if ((imageCount >= 0) || (imageCount == endImageIndex))	//TBD	//added on 12_07_2022 //--only for testing //--to be Removed
							{	
								
								//uint32_t dest;
								
								//if (1 == iSelectedCamera)
								//if (1 == selectedCamera)																		
								if (1 == *camPtr)	
								{
									dest = ((uint32_t) decoupler_r_cam1) + ( (linesSaved) * STRIDE + (imageCount-1)*FRAME_SIZE); 
								}
								//else if (2 == iSelectedCamera)
								//else if (2 == selectedCamera)	
								else if (2 == *camPtr)
								{
									dest = ((uint32_t) decoupler_r_cam2) + ( (linesSaved) * STRIDE + (imageCount-1)*FRAME_SIZE); 
								}
#if 1								
								//if(imageCount == (endImageIndex-1))
								{
								if((set_First == 0)&&(*pt == 0x10)) ErrorImageCount++;	
                                //if(set_First == 0)First_Raw_Data[(imageCount-1)][linesSaved] = *pt;
								if(linesSaved > 49)set_First = 1;
								}
#endif								
								//Debug_Output2(0,"entered to saving data dest = %ld , pt = %ld  ",dest,*pt);
    						    unsafe{OptMemCpy_1line_no_prefetch(pt, (void*)dest, STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32
						    }
						    linesSaved++;

                            if (linesSaved == (end_y - start_y))
                            { 
/*								if ((imageCount >= startImageIndex) || (imageCount == endImageIndex))
								Debug_Output2(0,"entered to saving data dest = %ld , pt = %ld  ",dest,*pt);						
*/						        
								//if (imageCount < FRAMES_IN_DDR)
								if (imageCount < endImageIndex)	
						        {
									if((ErrorImageCount < 40)||(header_count_acutal > ((endImageIndex*3)+3))) 
									{
										header_count = 3;
										properImage = 1;
										ErrorImageCount = 0;
									}
									else
									{
										if(((header_count % 3) == 0)&&(header_count > 0))
										{
											Debug_Output2(0,"ReCapture For Good Image ,ErrorImageCount = %d ,header_count=%d",ErrorImageCount,header_count);
											header_count -= 1;
											ErrorImageCount = 0;
											properImage = 0;
										}
										else
										{
											Debug_Output2(0,"ReCapture For Good Image,ErrorImageCount=%d ,header_count=%d",ErrorImageCount,header_count);
											ErrorImageCount = 0;
											properImage = 0;
										}
									}		
									
							        if(((header_count % 3) == 0)&&(header_count > 0))imageCount++;
									//printintln(imageCount);
#if 0									
									if ((imageCount > startImageIndex)&&(IMAGE_TRIGGER_CAPTURE_MULTIPLE_IMAGES == imgTriggerType))
									{
										/* Delay Between Images Based on Vehicle ENTRY OR EXIT State */
										//#if INSERT_DELAY_BETWEEN_IMAGES
#if 0
										if ((delayBetweenImagesVehicleEntry > 0) && (VEHICLE_OCCUPANCY_ENTRY_STATE == vehicleOccupancyStatusIndication))
										{
											delay_milliseconds(delayBetweenImagesVehicleEntry);
										}
										else if ((delayBetweenImagesVehicleExit > 0) && (VEHICLE_OCCUPANCY_EXIT_STATE == vehicleOccupancyStatusIndication))
										{
											delay_milliseconds(delayBetweenImagesVehicleExit);
										}
#endif										
										//#endif //INSERT_DELAY_BETWEEN_IMAGES										
									}
#endif									
							        new_grabbing = 1;
									
#if 0
									/* Disable MIPI SW OE, MIPI SW SEL, Image Captured Completed */
									//if (imageCount == FRAMES_IN_DDR)
									if (imageCount == endImageIndex)
									{
										MIPI_SW_OE_PIN <: 1;	//Set 
										MIPI_SW_SEL_PIN <: 0;	//Clear																	
										/* 
										Left Cam trigger pin needed to remap 
										but current XMOS board 1.8V GPIO is not available on TILE[1], 
										so shorted Right and Left Cam trigger pins, 
										so that Camera streaming ON/OFF of both camera's can be 
										controlled using only one GPIO because one camera will be 
										ON/Active at a time.
										*/
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{											
											//LEFT_CAM_TRIGGER_PIN <: 0; //Clear
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear
											
											resetGpio(PORT_LEDS_X1D4C2);
										}
									}	
#endif									
									//added on 11_07_2022
									/*else if (imageCount < endImageIndex)
									{
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{
											//LEFT_CAM_TRIGGER_PIN <: 1; //Set
											////RIGHT_CAM_TRIGGER_PIN <: 1; //Set 
											setGpio(PORT_LEDS_X1D4C2);
											delay_microseconds(1);
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear 
											resetGpio(PORT_LEDS_X1D4C2);
										}
									}*/									
						        }

								//printf("\r\n	MI: CAM(%d) Image(%d) Captured.", iSelectedCamera, imageCount);
								//printf("\r\n	MI: CAM(%d) Image(%d) Captured.", selectedCamera, imageCount);	
								//printf("\r\n	MI: CAM(%d) Image(%d) Captured.", *camPtr, imageCount);								
								/*printstrln(" ");
								printstr(" 	MI: CAM");
								printint(*camPtr);
								printstr(" Image");
								printint(imageCount);
								printstr(" Captured.");*/
								
								char byteString[50];					
								sprintf(byteString, "\r\n	MI: CAM(%d) Image(%d) Captured.", *camPtr, imageCount);
								//printstr(byteString);
								Debug_TextOut(0,byteString);
								
								//if ((imageCount == endImageIndex) && (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType))
								/*if (imageCount == endImageIndex)
								{
									//char byteString[50];					
									asm volatile("gettime %0" : "=r" (currentTimer));
									sprintf(byteString, "\r\n	MI: Image Capture Total Time: %dms.", (((currentTimer - startTimer) / 100) / 1000));
									printstr(byteString);
									
									startTimer = 0;
									currentTimer = 0;
									asm volatile("gettime %0" : "=r" (startTimer));
								}*/

						        linesSaved = 0;
                                grabbing = 0;			
                                //if (imageCount == FRAMES_IN_DDR){
#if 0									
								if (imageCount == endImageIndex){	
									#if 1 //CONTINUOUS_IMAGE_CAPTURE
										/*printf("\r\nMI: decoupler_r_cam1: \n");
										for(int frame=0; frame < FRAMES_IN_DDR-1; frame++){
											for (int row = 0; row < 20; row++)
											{
												printf("\r\nMI: F[%02d].R[%02d]: ", frame, row);
												for (int column = 0; column < 50; column+=2)
												{									
													unsafe{
														printf("%02X", decoupler_r_cam1->frames[frame][row*STRIDE+column+1]);
														printf("%02X ", decoupler_r_cam1->frames[frame][row*STRIDE+column]);
													}
												}
											}
										}*/
										imageCount = 0;
										new_grabbing = 0;	//TBD //added on 21_06_2022
										line = 0;
									#endif //CONTINUOUS_IMAGE_CAPTURE
									
									Debug_Output1(0," Total header Counter = %d",header_count_acutal);
									
									outuchar(txCmd, HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE);	//added on 20_06_2022
                                    #if PRINT_DEBUG_MIPI_LPDDR_SHIFTED_DATA
										printstr("\r\nMI: Triggering Jpeg Create Task. End");
                                        outuchar(c_imgCount, 0x5A);
                                    #endif
									header_count = 0;
									header_count_acutal = 0;
									//return; //commented on 20_06_2022
                                }
								else if(imageCount > startImageIndex)
								{
									#if PRINT_DEBUG_MIPI_LPDDR_SHIFTED_DATA
										printstr("\r\nMI: Triggering Jpeg Create Task. Start");
                                        outuchar(c_imgCount, 0x5A);
                                    #endif
									
									if (imageCount < endImageIndex)
									{
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{
											//LEFT_CAM_TRIGGER_PIN <: 1; //Set
											////RIGHT_CAM_TRIGGER_PIN <: 1; //Set 
											setGpio(PORT_LEDS_X1D4C2);
											delay_microseconds(1);
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear 
											resetGpio(PORT_LEDS_X1D4C2);
										}
									}									
								}
								else if (imageCount < endImageIndex)
								{
										if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
										{
											//LEFT_CAM_TRIGGER_PIN <: 1; //Set
											////RIGHT_CAM_TRIGGER_PIN <: 1; //Set 
											setGpio(PORT_LEDS_X1D4C2);
											delay_microseconds(1);
											////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear 
											resetGpio(PORT_LEDS_X1D4C2);
										}
								}
#endif
                            }
                        }
                        lineCount++;	
                    }
					else
					{
						////Debug_TextOut(0,"M");
						//Debug_Output1(0,"header: = %ld",header);						
					}	
                    break;				
				/* This case/Loop will be triggered when MSP sends RTC Info Packet. */
                case inuchar_byref(c_decoupler, new_grabbing):			
					*camPtr = inuchar(c_decoupler);	//TBD //added on 21_06_2022
					imgTriggerType = inuchar(c_decoupler); //added on 05_07_2022

					imgTriggerType =  IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE;
					
					if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
					{
						*totCapImagesPtr = FRAMES_IN_DDR;//2;
					}
					else
					{
						//TBD //TODO:: add here ..load totCapImagesPtr from config parameters...
						*totCapImagesPtr = FRAMES_IN_DDR;
					}
					
					////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear
					resetGpio(PORT_LEDS_X1D4C2);
					
					char byteString[50];
					sprintf(byteString, "\r\nMI: Capture CAM%d Images.", *camPtr);
					printstr(byteString);
	
					/* select MIPI_SW_SEL and set MIPI_SW_OE as per camera trigger	*/
					/* Disable MIPI SW OE, MIPI SW SEL */
					MIPI_SW_OE_PIN <: 1;	//Set 
					MIPI_SW_SEL_PIN <: 0;	//Clear
	
					/*startTimer = 0;
					currentTimer = 0;
					asm volatile("gettime %0" : "=r" (startTimer));*/

					/* 
					Left Cam trigger pin needed to remap 
					but current XMOS board 1.8V GPIO is not available on TILE[1], 
					so shorted Right and Left Cam trigger pins, 
					so that Camera streaming ON/OFF of both camera's can be 
					controlled using only one GPIO because one camera will be 
					ON/Active at a time.
					*/					
					
					if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
					{
						/*startTimer = 0;
						currentTimer = 0;
						asm volatile("gettime %0" : "=r" (startTimer));*/						
					
						//LEFT_CAM_TRIGGER_PIN <: 0; //Clear
						////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear
						
						resetGpio(PORT_LEDS_X1D4C2);
					}
					
					//if (1 == selectedCamera)
					if (1 == *camPtr)
					{
						new_grabbing = 1;
						
						/* Enable MIPI SW OE, MIPI SW SEL=1 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 1;	//Set
						/* 
						Left Cam trigger pin needed to remap 
						but current XMOS board 1.8V GPIO is not available on TILE[1], 
						so shorted Right and Left Cam trigger pins, 
						so that Camera streaming ON/OFF of both camera's can be 
						controlled using only one GPIO because one camera will be 
						ON/Active at a time.
						*/
						/*if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
						{						
							//LEFT_CAM_TRIGGER_PIN <: 1; //Set
							////RIGHT_CAM_TRIGGER_PIN <: 1; //Set //added on 19_07_2022
							setGpio(PORT_LEDS_X1D4C2);
							delay_microseconds(1);
							////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear //added on 19_07_2022
							resetGpio(PORT_LEDS_X1D4C2);
						}*/
						
						header_count++;
#if SINGLE_CAPTURE
						captureValid = 1;
#endif
					}
					//else if (2 == selectedCamera)
					else if (2 == *camPtr)	
					{						
						new_grabbing = 1;
						
						/* Enable MIPI SW OE, MIPI SW SEL=0 */
						MIPI_SW_OE_PIN <: 0;	//Clear 
						MIPI_SW_SEL_PIN <: 0;	//Clear
						
						//delay_milliseconds(1);
						
						/*if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imgTriggerType)
						{
							////RIGHT_CAM_TRIGGER_PIN <: 1; //Set //added on 19_07_2022
							setGpio(PORT_LEDS_X1D4C2);
							delay_microseconds(1);
							////RIGHT_CAM_TRIGGER_PIN <: 0; //Clear //added on 19_07_2022
							resetGpio(PORT_LEDS_X1D4C2);
						}*/
						
						header_count++;
#if SINGLE_CAPTURE
						captureValid = 1;
#endif						
					}
					else
					{
						//printf("MI: NO Camera detected..\n");	
						//printstrln(" ");
						printstr("\r\nMI: NO Camera detected..");
						new_grabbing = 0;
					}
					////resetGpio(PORT_LEDS_X1D4C0);
					//PORT_LEDS_X1D4C/*XS1_PORT_4C*/ <: 0;				
                    break;
				
				case t when timerafter(time) :> void:
					Debug_Output1(0, "Image capture timer expired, allImagesCaptured(%d)", allImagesCaptured);
					timerSet = 0;
					break;
            }
        }
    }
}

unsigned char selectedImageSensor = 0;

void FileName_create(unsigned char imageSensor, int imageNumber, char *extension, uint8_t currentSession)
{		
	//printf("\r\nimageSensor: %d\n", imageSensor);	
	 sprintf(TmpStr, "\n\nIn File name create \n");
			
	     Debug_TextOut(0, TmpStr);	
	
	if (1 == createFileName)
    {
        createFileName = 0;
		
        //if (1 == iSelectedCamera)
		if (1 == imageSensor)
		{
            sprintf(imageCapture.vars.currentFileNameBase, "IMG_%04d_%05d_%04d%02d%02d_%02d%02d%02d", iCustomerId, iLeftMID, year, month, date, hour, minute, second);
        }
		//else if (2 == iSelectedCamera)
		else if (2 == imageSensor)	
		{
			sprintf(imageCapture.vars.currentFileNameBase, "IMG_%04d_%05d_%04d%02d%02d_%02d%02d%02d", iCustomerId, iRightMID, year, month, date, hour, minute, second);
		}		
		
		//printf("\r\ncurrentFileNameBase: %s", imageCapture.vars.currentFileNameBase);	
	}
	
	if (!currentSession)
	{
		strcpy(imageCapture.vars.currentFileNameBase, imageCapture.vars.previousFileNameBase);
	}
	
    sprintf(fName, "%s_%02d%s", imageCapture.vars.currentFileNameBase, imageNumber, extension);
	
	currentSession_global = currentSession;
	//printf("\r\ncurrentSession_global: %d", currentSession_global);
	//printf("\r\nfName: %s", fName);

	Debug_TextOut(0,fName);	
}

/* Find how many bits to be shifted Right */
#define BIN_COMPARE_VALUE 5000

unsigned long rbin8 = 0;
unsigned long rbin9 = 0;
unsigned long rbin10 = 0;
unsigned long rbin11 = 0;

unsigned long gbin8 = 0;
unsigned long gbin9 = 0;
unsigned long gbin10 = 0;
unsigned long gbin11 = 0;

unsigned long bbin8 = 0;
unsigned long bbin9 = 0;
unsigned long bbin10 = 0;
unsigned long bbin11 = 0;

unsigned short rshift = 3;
unsigned short gshift = 3;
unsigned short bshift = 3;

unsigned long binaryCompareValue = BIN_COMPARE_VALUE;

void RawImage_resetVars(void)
{
    rbin8 = 0;
    rbin9 = 0;
    rbin10 = 0;
    rbin11 = 0;

    gbin8 = 0;
    gbin9 = 0;
    gbin10 = 0;
    gbin11 = 0;

    bbin8 = 0;
    bbin9 = 0;
    bbin10 = 0;
    bbin11 = 0;

    rshift = 3;
    gshift = 3;
    bshift = 3;
}

void RawImage_findBitCountOfHigherByteData(unsigned short pixelData, unsigned char color)
{
    if (0 == color)
    {
        if (pixelData > 0x00FF)
        {
            rbin8++;
        }

        if (pixelData > 0x01FF)
        {
            rbin9++;
        }

        if (pixelData > 0x03FF)
        {
            rbin10++;
        }

        if (pixelData > 0x07FF)
        {
            rbin11++;
        }		
    }
    else if (1 == color)
    {
        if (pixelData > 0x00FF)
        {
            gbin8++;
        }

        if (pixelData > 0x01FF)
        {
            gbin9++;
        }

        if (pixelData > 0x03FF)
        {
            gbin10++;
        }

        if (pixelData > 0x07FF)
        {
            gbin11++;
        }
    }
    else if (2 == color)
    {
        if (pixelData > 0x00FF)
        {
            bbin8++;
        }

        if (pixelData > 0x01FF)
        {
            bbin9++;
        }

        if (pixelData > 0x03FF)
        {
            bbin10++;
        }

        if (pixelData > 0x07FF)
        {
            bbin11++;
        }
    }
}

void RawImage_findDataShiftCount(int i)
{
    //if (0 == binaryCompareValue)
    {
        //binaryCompareValue = BIN_COMPARE_VALUE;
    }
    if (0 == i)
    {
        //binaryCompareValue = 50000;
		binaryCompareValue = 70000;
    }
    else
    {
        binaryCompareValue = 100;
    }

    //printf("\r\nBinary Compare Value: %d", binaryCompareValue);

    rshift = 3;
    gshift = 3;
    bshift = 3;

    //RED color
    if (rbin8 < binaryCompareValue)
    {
        rshift = 0;
    }
    else if (rbin9 < binaryCompareValue)
    {
        rshift = 1;
    }
    else if (rbin10 < binaryCompareValue)
    {
        rshift = 2;
    }
    else if (rbin11 < binaryCompareValue)
    {
        rshift = 3;
    }

    if (gbin8 < binaryCompareValue)
    {
        gshift = 0;
    }
    else if (gbin9 < binaryCompareValue)
    {
        gshift = 1;
    }
    else if (gbin10 < binaryCompareValue)
    {
        gshift = 2;
    }
    else if (gbin11 < binaryCompareValue)
    {
        gshift = 3;
    }

    if (bbin8 < binaryCompareValue)
    {
        bshift = 0;
    }
    else if (bbin9 < binaryCompareValue)
    {
        bshift = 1;
    }
    else if (bbin10 < binaryCompareValue)
    {
        bshift = 2;
    }
    else if (bbin11 < binaryCompareValue)
    {
        bshift = 3;
    }

    if (rshift > gshift)
    {
        gshift = rshift;
        bshift = rshift;
    }
    else
    {
        rshift = gshift;
        bshift = gshift;
    }
	
}

//unsigned short rawBuffer[IMAGE_BLOCK_SIZE];

#if 0
void JpegColorImage_create(unsigned char imageSensor, int frame, uint8_t currentSession)
{
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	char byteString[200];
	
	/* Create File Name */	
	FileName_create(imageSensor, frame, ".jpeg", 1);
	RawImage_resetVars();				

	int imgRow = 0;					

	asm volatile("gettime %0" : "=r" (startTimer));	

	/* Find Bit Count Of Higher Byte Data */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	{						
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;
			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{
					unsigned short pixelData;
					
					//if (1 == iSelectedCamera)
					if (1 == imageSensor)
					{						
						pixelData = (unsigned short)(((((unsigned short)decoupler_r_cam1->frames[frame][imgRow*STRIDE+imgCol+1] << 8) & 0xFF00) | 
													   ((unsigned short)decoupler_r_cam1->frames[frame][imgRow*STRIDE+imgCol] & 0x00FF)) >> 4);
					}
					//else if (2 == iSelectedCamera)
					else if (2 == imageSensor)
					{
						pixelData = (unsigned short)(((((unsigned short)decoupler_r_cam2->frames[frame][imgRow*STRIDE+imgCol+1] << 8) & 0xFF00) | 
													   ((unsigned short)decoupler_r_cam2->frames[frame][imgRow*STRIDE+imgCol] & 0x00FF)) >> 4);	
					}		
					else
					{
						return;
					}

					unsigned char color = 0;

					if ((0 == (row % 2)) && (0 != (col % 2)))	//Red
					{
						color = 1;
					}								
					else if ((0 == (row % 2)) && (0 == (col % 2)))	//Gr
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 != (col % 2)))	//Gb
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 == (col % 2)))	//Blue
					{
						color = 3;
					}
					
					if ((color > 0) && (color < 4))
					{
						RawImage_findBitCountOfHigherByteData(pixelData, (color - 1));
					}	
				}
			}
		}	
	}					
	
	/* Find the bit shift based on upper 4 bit counts */				
	RawImage_findDataShiftCount(0);		
	
	/*printf("\r\nJC: Red Color:   bin8:%d  bin9:%d  bin10:%d  bin11:%d", rbin8, rbin9, rbin10, rbin11);
	printf("\r\nJC: Green Color: bin8:%d  bin9:%d  bin10:%d  bin11:%d", gbin8, gbin9, gbin10, gbin11);
	printf("\r\nJC: Blue Color:  bin8:%d  bin9:%d  bin10:%d  bin11:%d", bbin8, bbin9, bbin10, bbin11);         
	printf("\r\nJC: rshift=%d", rshift);
	printf("\r\nJC: gshift=%d", gshift);
	printf("\r\nJC: bshift=%d", bshift);*/
	
	imgRow = 0;
			
	/* Select Image Block */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	{
		memset(rawBuffer, 0, sizeof(rawBuffer));
				
		/* Copy the DDR Image Data to RAW Buffer */
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;
			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{
					//if (1 == iSelectedCamera)
					if (1 == imageSensor)	
					{
						rawBuffer[(row * IMG_COLS) + col] = (unsigned short)(((((unsigned short)decoupler_r_cam1->frames[frame][imgRow*STRIDE+imgCol+1] << 8) & 0xFF00) | 
																			   ((unsigned short)decoupler_r_cam1->frames[frame][imgRow*STRIDE+imgCol] & 0x00FF)) >> 4);
					}
					//else if (2 == iSelectedCamera)
					else if (2 == imageSensor)
					{						
						rawBuffer[(row * IMG_COLS) + col] = (unsigned short)(((((unsigned short)decoupler_r_cam2->frames[frame][imgRow*STRIDE+imgCol+1] << 8) & 0xFF00) | 
																			   ((unsigned short)decoupler_r_cam2->frames[frame][imgRow*STRIDE+imgCol] & 0x00FF)) >> 4);								
																			   
					}
					else
					{
						return;
					}

					if ((rshift > 0) && ((0 == (row % 2)) && (0 != (col % 2))))									
					{
						rawBuffer[(row * IMG_COLS) + col] >>= rshift;
					}								
					
					if ((gshift > 0) && ((0 == (row % 2)) && (0 == (col % 2))))
					{
						rawBuffer[(row * IMG_COLS) + col] >>= gshift;
					}								

					if ((gshift > 0) && ((0 != (row % 2)) && (0 != (col % 2))))
					{
						rawBuffer[(row * IMG_COLS) + col] >>= gshift;
					}								
					
					if ((bshift > 0) && ((0 != (row % 2)) && (0 == (col % 2))))
					{
						rawBuffer[(row * IMG_COLS) + col] >>= bshift;
					}								

					if (rawBuffer[(row * IMG_COLS) + col] > 0xFF)
					{
						rawBuffer[(row * IMG_COLS) + col] = 0xFF;
					}
				}
			}
		}
		
		JpegCompress_blockWise(imgBlock, rawBuffer, 0, 0, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK,currentSession);
	}

	asm volatile("gettime %0" : "=r" (currentTimer));	
	//char byteString[50];
	sprintf(byteString, "\r\nJC: Compression Time: %dms.\r\n", (((currentTimer - startTimer) / 100) / 1000));
	printstr(byteString);
	
	Debug_TextOut(0,byteString);	
}
#endif

#if 1

//TBD //added on 25_07_2022
typedef union RawBuffer {
	//uint8_t buffer[IMAGE_BLOCK_SIZE * 2 + (NUM_ROWS_PER_IMAGE_BLOCK * 48)];
	//uint16_t wordBuffer[IMAGE_BLOCK_SIZE + ((NUM_ROWS_PER_IMAGE_BLOCK * 48)/2)];
	uint8_t buffer[STRIDE * NUM_ROWS_PER_IMAGE_BLOCK];
	uint16_t wordBuffer[(STRIDE/2) * NUM_ROWS_PER_IMAGE_BLOCK];
} RawBuffer_t;

RawBuffer_t rawImage;	//TBD //added on 25_07_2022


void JpegColorImage_create(unsigned char imageSensor, int frame, uint8_t currentSession)
{
	////Debug_Output1(0,"\r\nJpegColorImage_create. = frame %d", frame);
	/* Create File Name */	
	FileName_create(imageSensor, frame, ".jpeg", 1);
	RawImage_resetVars();				

	int imgRow = 0;					
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	char byteString[200];
	uint32_t src;

	set_Second = 0;
	
	asm volatile("gettime %0" : "=r" (startTimer));	
#if 1
	/* Find Bit Count Of Higher Byte Data */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	{						
		/* Copy DDR to RAM Buffer */		
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;

			unsafe
			{
				/* Copy 1 Row DDR Data to RAM */
				//uint32_t src;		

				if (1 == imageSensor)	
				{
					src = ((uint32_t) decoupler_r_cam1) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				else if (2 == imageSensor)
				{
					src = ((uint32_t) decoupler_r_cam2) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
			
				unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[(row * (IMG_COLS * 2))], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32
			}
#if 0			
            if((set_Second == 0)/*&&(frame ==3)*/)
			{
				printstr("\r\n");
				printstr("Second data");
				printstr("\r\n");
				
				for(uint8_t v=0;v<50;v++)
				{
					/*printhex(rawImage.buffer[v]); 	 	
					printstr(" ");	*/	
					
				}
				
				for(uint8_t v=0;v<50;v++)
				{
					printhex(rawImage.wordBuffer[v]); 	 	
					printstr(" ");					
				}
				printstr("\r\n");
				set_Second = 1;
			}
#endif			
#if 1			
			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{				
					unsigned char color = 0;
					unsigned short pixelData;

					if ((0 == (row % 2)) && (0 != (col % 2)))	//Red
					{
						color = 1;
					}								
					else if ((0 == (row % 2)) && (0 == (col % 2)))	//Gr
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 != (col % 2)))	//Gb
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 == (col % 2)))	//Blue
					{
						color = 3;
					}
					
					if ((color > 0) && (color < 4))
					{
						//RawImage_findBitCountOfHigherByteData(pixelData, (color - 1)); 
						//RawImage_findBitCountOfHigherByteData((rawImage.wordBuffer[col] >> 4), (color - 1));
						
						//pixelData = rawImage.wordBuffer[(row * IMG_COLS) + col] >> 6;
						pixelData = rawImage.wordBuffer[(row * IMG_COLS) + col] >> 4;
						
						if (1 == color)
						{
							if (pixelData > 0x00FF)
							{
								rbin8++;
							}

							if (pixelData > 0x01FF)
							{
								rbin9++;
							}

							if (pixelData > 0x03FF)
							{
								rbin10++;
							}

							if (pixelData > 0x07FF)
							{
								rbin11++;
							}		
						}
						else if (2 == color)
						{
							if (pixelData > 0x00FF)
							{
								gbin8++;
							}

							if (pixelData > 0x01FF)
							{
								gbin9++;
							}

							if (pixelData > 0x03FF)
							{
								gbin10++;
							}

							if (pixelData > 0x07FF)
							{
								gbin11++;
							}
						}
						else if (3 == color)
						{
							if (pixelData > 0x00FF)
							{
								bbin8++;
							}

							if (pixelData > 0x01FF)
							{
								bbin9++;
							}

							if (pixelData > 0x03FF)
							{
								bbin10++;
							}

							if (pixelData > 0x07FF)
							{
								bbin11++;
							}
						}						
					}	
				}
			}
#endif
		}	
	}					
#endif
	//Debug_Output2(0,"Find Bit Count Of Higher Byte Data src = %ld , pt = %ld  ",src,rawImage.buffer[0]);		
#if 1
	/* Find the bit shift based on upper 4 bit counts */				
	RawImage_findDataShiftCount(0);		
#endif	
	/*printf("\r\nJC: Red Color:   bin8:%d  bin9:%d  bin10:%d  bin11:%d", rbin8, rbin9, rbin10, rbin11);
	printf("\r\nJC: Green Color: bin8:%d  bin9:%d  bin10:%d  bin11:%d", gbin8, gbin9, gbin10, gbin11);
	printf("\r\nJC: Blue Color:  bin8:%d  bin9:%d  bin10:%d  bin11:%d", bbin8, bbin9, bbin10, bbin11);         
	printf("\r\nJC: rshift=%d", rshift);
	printf("\r\nJC: gshift=%d", gshift);
	printf("\r\nJC: bshift=%d", bshift);*/
	
	Debug_Output6(0,"Red Color:   bin8:%d  bin9:%d  bin10:%d  bin11:%d",rbin8, rbin9, rbin10, rbin11,0,0);
	Debug_Output6(0,"Green Color: bin8:%d  bin9:%d  bin10:%d  bin11:%d",gbin8, gbin9, gbin10, gbin11,0,0);
	Debug_Output6(0,"Blue Color:  bin8:%d  bin9:%d  bin10:%d  bin11:%d",bbin8, bbin9, bbin10, bbin11,0,0);
	
	Debug_Output6(0,"before rshift=%d,gshift=%d,bshift=%d",rshift,gshift,bshift,0,0,0);
	if(rshift > 2)rshift = 3;
	else if(rshift > 1)rshift = 2;
	
	if(gshift > 2)gshift = 3;
	else if(gshift > 1)gshift = 2;
	
	if(bshift > 2)bshift = 3;
	else if(bshift > 1)bshift = 2;	
	Debug_Output6(0,"after rshift=%d,gshift=%d,bshift=%d",rshift,gshift,bshift,0,0,0);
	
	imgRow = 0;
	
	/* Select Image Block */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	{
#if 1
		/* Copy the DDR Image Data to RAW Buffer */
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;

			unsafe
			{
				/* Copy 1 Row DDR Data to RAM */
				//uint32_t src;		

				if (1 == imageSensor)	
				{
					src = ((uint32_t) decoupler_r_cam1) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				else if (2 == imageSensor)
				{
					src = ((uint32_t) decoupler_r_cam2) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				
				unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[(row * (IMG_COLS * 2))], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32				
			}
#if 1
			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{			
					if(rshift == 3)					
					rawImage.wordBuffer[(row * IMG_COLS) + col] >>= 6;
					else if(rshift == 2)					
					rawImage.wordBuffer[(row * IMG_COLS) + col] >>= 5;				
					else
					rawImage.wordBuffer[(row * IMG_COLS) + col] >>= 4;
					
					if ((rshift > 0) && ((0 == (row % 2)) && (0 != (col % 2))))									
					{
						rawImage.wordBuffer[(row * IMG_COLS) + col] >>= rshift;
					}								
					
					if ((gshift > 0) && ((0 == (row % 2)) && (0 == (col % 2))))
					{
						rawImage.wordBuffer[(row * IMG_COLS) + col] >>= gshift;
					}								

					if ((gshift > 0) && ((0 != (row % 2)) && (0 != (col % 2))))
					{
						rawImage.wordBuffer[(row * IMG_COLS) + col] >>= gshift;
					}								
					
					if ((bshift > 0) && ((0 != (row % 2)) && (0 == (col % 2))))
					{
						rawImage.wordBuffer[(row * IMG_COLS) + col] >>= bshift;
					}								

					if (rawImage.wordBuffer[(row * IMG_COLS) + col] > 0xFF)
					{
						rawImage.wordBuffer[(row * IMG_COLS) + col] = 0xFF;
					}
				}
			}
#endif			
		}	
#endif
		JpegCompress_blockWise(imgBlock, rawImage.wordBuffer, 0, 0, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK, currentSession);
	}

	//Debug_Output2(0,"Find Bit Count Of Higher Byte Data src = %ld , pt = %ld  ",src,rawImage.buffer[0]);		

	asm volatile("gettime %0" : "=r" (currentTimer));	
	sprintf(byteString, "\r\nJC: Compression Time: %dms.\n", (((currentTimer - startTimer) / 100) / 1000));
	printstr(byteString);

	Debug_TextOut(0,byteString);
	
}

#endif
#if 0
//TBD //added on 25_07_2022
typedef union RawBuffer {
	//uint8_t buffer[IMAGE_BLOCK_SIZE * 2 + (NUM_ROWS_PER_IMAGE_BLOCK * 48)];
	//uint16_t wordBuffer[IMAGE_BLOCK_SIZE + ((NUM_ROWS_PER_IMAGE_BLOCK * 48)/2)];
	uint8_t buffer[STRIDE * NUM_ROWS_PER_IMAGE_BLOCK];
	uint16_t wordBuffer[(STRIDE/2) * NUM_ROWS_PER_IMAGE_BLOCK];
} RawBuffer_t;

RawBuffer_t rawImage;	//TBD //added on 25_07_2022

void JpegColorImage_create(unsigned char imageSensor, int frame, uint8_t currentSession)
{
	/* Create File Name */	
	FileName_create(imageSensor, frame, ".jpeg", 1);
	RawImage_resetVars();				

	int imgRow = 0;					
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	char byteString[200];
	//RawBuffer_t rawImage;	//TBD //added on 25_07_2022

	//printstr("\r\nJC: Creating TEST_IMAGE_00.jpeg");	
	asm volatile("gettime %0" : "=r" (startTimer));	

	/* Find Bit Count Of Higher Byte Data */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	{						
		/* Copy DDR to RAM Buffer */		
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;

			unsafe		//TBD //added on 25_07_2022
			{
				/* Copy 1 Row DDR Data to RAM */
				uint32_t src;		

				if (1 == imageSensor)	
				{
					src = ((uint32_t) decoupler_r_cam1) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				else if (2 == imageSensor)
				{
					src = ((uint32_t) decoupler_r_cam2) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				
				//unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[row * MAX_IMG_ROWS], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32				
				//unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[row*STRIDE], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32				
				unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[(row * IMG_COLS)], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32
			}
			
			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{				
					unsigned char color = 0;
					unsigned short pixelData;

					if ((0 == (row % 2)) && (0 != (col % 2)))	//Red
					{
						color = 1;
					}								
					else if ((0 == (row % 2)) && (0 == (col % 2)))	//Gr
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 != (col % 2)))	//Gb
					{
						color = 2;
					}

					if ((0 != (row % 2)) && (0 == (col % 2)))	//Blue
					{
						color = 3;
					}
					
					if ((color > 0) && (color < 4))
					{
						//RawImage_findBitCountOfHigherByteData(pixelData, (color - 1)); 	//TBD //commented on 25_07_2022
						//RawImage_findBitCountOfHigherByteData((rawImage.wordBuffer[col] >> 4), (color - 1)); 	//TBD //added on 25_07_2022
						
						pixelData = (rawImage.wordBuffer[col] >> 4);
						
						if (0 == color)
						{
							if (pixelData > 0x00FF)
							{
								rbin8++;
							}

							if (pixelData > 0x01FF)
							{
								rbin9++;
							}

							if (pixelData > 0x03FF)
							{
								rbin10++;
							}

							if (pixelData > 0x07FF)
							{
								rbin11++;
							}		
						}
						else if (1 == color)
						{
							if (pixelData > 0x00FF)
							{
								gbin8++;
							}

							if (pixelData > 0x01FF)
							{
								gbin9++;
							}

							if (pixelData > 0x03FF)
							{
								gbin10++;
							}

							if (pixelData > 0x07FF)
							{
								gbin11++;
							}
						}
						else if (2 == color)
						{
							if (pixelData > 0x00FF)
							{
								bbin8++;
							}

							if (pixelData > 0x01FF)
							{
								bbin9++;
							}

							if (pixelData > 0x03FF)
							{
								bbin10++;
							}

							if (pixelData > 0x07FF)
							{
								bbin11++;
							}
						}						
					}	
				}
			}
		}	
	}					
	
	/* Find the bit shift based on upper 4 bit counts */				
	RawImage_findDataShiftCount(0);		
	
	/*printf("\r\nJC: Red Color:   bin8:%d  bin9:%d  bin10:%d  bin11:%d", rbin8, rbin9, rbin10, rbin11);
	printf("\r\nJC: Green Color: bin8:%d  bin9:%d  bin10:%d  bin11:%d", gbin8, gbin9, gbin10, gbin11);
	printf("\r\nJC: Blue Color:  bin8:%d  bin9:%d  bin10:%d  bin11:%d", bbin8, bbin9, bbin10, bbin11);         
	printf("\r\nJC: rshift=%d", rshift);
	printf("\r\nJC: gshift=%d", gshift);
	printf("\r\nJC: bshift=%d", bshift);*/
	
	imgRow = 0;
	
	//printstr("\r\nJC: Creating TEST_IMAGE_00.jpeg");	
	//asm volatile("gettime %0" : "=r" (startTimer));	

	//memset(rawBuffer, 0, sizeof(rawBuffer)); //TBD 	//added on 22_07_2022	//only for testing //--to be removed
	
	/* Select Image Block */
	for (int imgBlock = 0; imgBlock < NUM_IMAGE_BLOCKS; imgBlock++)
	//for (int imgBlock = 0; imgBlock < 1; imgBlock++)	//TBD	//added on 21_07_2022	//only for testing //--to be removed
	{
		//memset(rawBuffer, 0, sizeof(rawBuffer));
				
		/* Copy the DDR Image Data to RAW Buffer */
		for (int row = 0; row < NUM_ROWS_PER_IMAGE_BLOCK; row++, imgRow++)
		{
			int col = 0;

			unsafe	//TBD //added on 25_07_2022
			{
				/* Copy 1 Row DDR Data to RAM */
				uint32_t src;		

				if (1 == imageSensor)	
				{
					src = ((uint32_t) decoupler_r_cam1) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				else if (2 == imageSensor)
				{
					src = ((uint32_t) decoupler_r_cam2) + ((imgRow) * STRIDE + (frame)*FRAME_SIZE); 
				}
				
				//unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[row * MAX_IMG_ROWS], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32				
				//unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[row*STRIDE], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32
				unsafe{OptMemCpy_1line_no_prefetch((void*)src, &rawImage.buffer[(row * IMG_COLS)], STRIDE/4); }// best fcn to ddr -- /4 for uint8 to uint32
				
			}

			for (int imgCol = 0; imgCol < (IMG_COLS * 2); imgCol+=2, col++)
			{
				unsafe
				{			
					//TBD //added on 25_07_2022
					if ((rshift > 0) && ((0 == (row % 2)) && (0 != (col % 2))))									
					{
						rawImage.buffer[(row * IMG_COLS) + col] >>= rshift;
					}								
					
					if ((gshift > 0) && ((0 == (row % 2)) && (0 == (col % 2))))
					{
						rawImage.buffer[(row * IMG_COLS) + col] >>= gshift;
					}								

					if ((gshift > 0) && ((0 != (row % 2)) && (0 != (col % 2))))
					{
						rawImage.buffer[(row * IMG_COLS) + col] >>= gshift;
					}								
					
					if ((bshift > 0) && ((0 != (row % 2)) && (0 == (col % 2))))
					{
						rawImage.buffer[(row * IMG_COLS) + col] >>= bshift;
					}								

					if (rawImage.buffer[(row * IMG_COLS) + col] > 0xFF)
					{
						rawImage.buffer[(row * IMG_COLS) + col] = 0xFF;
					}
				}
			}
		}
			
		//TBD //commented on 21_07_2022	//only for testing //--to be UNcommented
		//JpegCompress_blockWise(imgBlock, rawBuffer, 0, 0, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK,currentSession);
		
		JpegCompress_blockWise(imgBlock, rawImage.wordBuffer, 0, 0, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK, currentSession);
		
		//TBD //added on 26_07_2022	//only for testing //--to be removed		
		//jo_write_jpg(imgBlock, "TEST_IMAGE_00.jpeg", (const void *)rawBuffer, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK, 0, 75);		
		//jo_write_jpg(imgBlock, "TEST_IMAGE_00.jpeg", (const void *)rawImage.wordBuffer, IMG_COLS, NUM_ROWS_PER_IMAGE_BLOCK, 1, 75);
	}

	asm volatile("gettime %0" : "=r" (currentTimer));	
	//char byteString[50];
	sprintf(byteString, "\r\nJC: Compression Time: %dms.", (((currentTimer - startTimer) / 100) / 1000));
	printstr(byteString);	
	//printf("\r\n\r\nTime %0.3f microseconds\n", ((currentTimer - startTimer)/100.0));
	//printstr("\r\nJC: Create TEST_IMAGE_00.jpeg Complete..");
}
#endif

unsigned short jpegTaskCount = 0;
unsigned short jpegTick = 0;
	ImageInfo_t * unsafe imgCap_Receive ;
	ImageInfo_t * unsafe imgCap_txData ;
	
void Jpeg_createTask(chanend c_ready, chanend JPEG_Status_Send, chanend FS_Status_Read)
{
	XmosConfig_read();
	
	uint8_t currentSession = 0;
	//uint8_t jpegState;
	int imageIndex = 0;	
	//int frame = 0;
	
	printf("\r\nJpeg_createTask.");		
	
	memset(&imageCapture, 0, sizeof(ImageInfo_t));
	
	unsafe {imgCap_Receive = &imageCapture;}
	unsafe {imgCap_txData = &imageCapture;}
	
    unsafe{
		while(1)
		{		
			uint8_t * unsafe camPtr = &selectedCamera;			
			uint8_t * unsafe totCapImagesPtr = &totCapImages; //added on 05_07_2022			
			uint8_t jpegState;
			
			select {
			case inuchar_byref(c_ready, jpegState):			
				int jstate = jpegState;				
				//selectedImageSensor = inuchar(c_ready);	//TBD //added on 21_06_2022
				//selectedImageSensor = iSelectedCamera;	//TBD //added on 21_06_2022
				selectedImageSensor = *camPtr;	//TBD //added on 21_06_2022
				//printf("\r\nselectedImageSensor: %d\n", selectedImageSensor);	
				/*printstrln(" ");
				printstr("JC: selectedImageSensor=");
				printint(selectedImageSensor);
				printstrln(" ");*/
				createFileName = 1;	 //TBD //--only for testing
					
				/* This Task/Loop triggers when RTC receives from MSP upon Sensor Trigger */
				//if (0x1A == inuchar(c_ready))
				if (0x1A == jstate)
				{
					imageIndex = 0;
					/* Set it when another trigger occurs */
					createFileName = 1;	 //TBD
					FileName_create(selectedImageSensor, imageIndex, ".jpeg", 1);
					//printf("\r\nCurrent File Name Base: %s\r\n", imageCapture.vars.currentFileNameBase);
					printstr("\r\nJC: Current File Name Base: ");
					printstr(imageCapture.vars.currentFileNameBase);
				}
				/* This Task/Loop triggers when Image capture completes */
				//else if (0x5A == inuchar(c_ready))
				else if (0x5A == jstate)
				{
					uint16_t index = 0;
					////frame++;
					
					////printf("\r 111 InterTileCommTile1_txInstance FROM TILE 1 before \n");
					outuchar(JPEG_Status_Send, JPEG_CREATION_START_INSTANCE);
					////printf("\r 111 InterTileCommTile1_txInstance FROM TILE 1 after \n");
					
					/***********WAIT UNTIL FILESYSTEM WORK BECOME SUSPEND**********/
					imageCapture.vars.ftpRunStatus = inuint(FS_Status_Read);
					rtos_fatfs_init_tast_call();
					
					//printf("\r\nJpeg_createTask Triggered..");
					//printstrln(" ");
					printstr("\r\nJC: Jpeg_createTask Triggered..");
					Debug_TextOut(0,"JC: Jpeg_createTask Triggered..");
					//printstrln(" ");
					//-----------------------------------------------------------------------------------
					//TODO:: below parameters to be taken from config.
					//TBD	//--to be Removed //--only for testing
					imageCapture.vars.brighterDataImages = 0xFFFFFFFF;
					//imageCapture.vars.totalCapturedNumImages = MAX_IMAGES_PER_CAMERA;					
					writeColorJpeg = 1;	
					//iSelectedCamera = 1;
					
					////iCustomerId = 1111;
					////iLeftMID = 32091;	
					////iRightMID = 32092;
	
					//-----------------------------------------------------------------------------------			
					
					imageCapture.vars.totalCapturedNumImages = *totCapImagesPtr - 1;	//added on 05_07_2022
					
					/* Create Jpeg, Raw file Loop */
					uint16_t createJpegNumImages = imageCapture.vars.totalCapturedNumImages + imageCapture.vars.previousResidualJpegNumImages + imageCapture.vars.previousCompletedAnprNumImages + imageCapture.vars.previousResidualAnprNumImages;

					imageCaptureCount = 0;
					
					
					/*//printf("\r\n");
					printf("\r\nJC: createJpegNumImages: %d", createJpegNumImages);
					printf("\r\nJC: totalCapturedNumImages: %d", imageCapture.vars.totalCapturedNumImages);
					printf("\r\nJC: previousResidualJpegNumImages: %d", imageCapture.vars.previousResidualJpegNumImages);
					printf("\r\nJC: previousCompletedAnprNumImages: %d", imageCapture.vars.previousCompletedAnprNumImages);
					printf("\r\nJC: previousResidualAnprNumImages: %d", imageCapture.vars.previousResidualAnprNumImages);					
					//printf("\r\n");*/
					

					//TODO:: overwrite image i.e. clear image count OR skip image writting to ddr.
					//if ((1 == iSelectedCamera) && (totalNumCompressedImagesDdrCam1 >= MAX_COMPRESSED_IMAGES_IN_DDR))
					if ((1 == selectedImageSensor) && (totalNumCompressedImagesDdrCam1 >= MAX_COMPRESSED_IMAGES_IN_DDR))
					{
						totalNumCompressedImagesDdrCam1 = 0;
						printf("\r\nCam1 Images in DDR Exceeds Max Limit. Overwrite Images..%d", totalNumCompressedImagesDdrCam1);
						Debug_Output1(0,"Cam1 Images in DDR Exceeds Max Limit. Overwrite Images..%d", totalNumCompressedImagesDdrCam1);
									//OR
						//printf("\r\nCam1 Images in DDR Exceeds Max Limit. Do not write Images in DDR..%d", totalNumCompressedImagesDdrCam1);
						//return;
					}
					
					//TODO:: overwrite image i.e. clear image count OR skip image writting to ddr.
					//if ((2 == iSelectedCamera) && (totalNumCompressedImagesDdrCam2 >= MAX_COMPRESSED_IMAGES_IN_DDR))
					if ((2 == selectedImageSensor) && (totalNumCompressedImagesDdrCam2 >= MAX_COMPRESSED_IMAGES_IN_DDR))
					{
						totalNumCompressedImagesDdrCam2 = 0;
						printf("\r\nCam2 Images in DDR Exceeds Max Limit. Overwrite Images..%d", totalNumCompressedImagesDdrCam2);
						Debug_Output1(0,"\r\nCam2 Images in DDR Exceeds Max Limit. Overwrite Images..%d", totalNumCompressedImagesDdrCam2);
									//OR
						//printf("\r\nCam2 Images in DDR Exceeds Max Limit. Do not write Images in DDR..%d", totalNumCompressedImagesDdrCam2);
						//return;
					}

					for (int frame = 0; frame < createJpegNumImages; frame++)		//TBD //commented on 22_07_2022 //--to be UNcommented
					//for (int frame = 0; frame < 4; frame++)			//TBD //added on 22_07_2022 //--to be removed
					{
						imageIndex = 0;
						
						/* Select Image Number for Current Captured Images */
						if ((frame/* - 1*/) < imageCapture.vars.totalCapturedNumImages)
						{
							index = (frame/* - 1*/);
							currentSession = 1;
							if ((1 == iEnableBrighterDataImageConfigPara) && (imageCapture.vars.executeBrighterDataImageTask))
							{
								imageCapture.vars.brighterDataImages = imageCapture.vars.currentBrighterDataImages;
							}
							
							//if ((iImageFilesTobeTransferred >> frame) & 0x1)
							if ((imageCapture.vars.brighterDataImages >> frame) & 0x1)
							{	
								imageIndex = index;
								
								if (imageIndex > MAX_IMAGES_PER_CAMERA)	
								{
									printf("\r\nImage Index Exceeds Max Limit. %d", (imageIndex + 1));
									Debug_Output1(0,"\r\nImage Index Exceeds Max Limit. %d", (imageIndex + 1));
									break;	//TBD
								}
							}
							Debug_Output2(0,"\r\nCurrent End Index=%d , index=%d", imageIndex,index);
						}
						/* Select  Image Number for Previous Pending Images */
						else if ((frame/* - 1*/) >= imageCapture.vars.totalCapturedNumImages)
						{
							index = ((frame/* - 1*/) - imageCapture.vars.totalCapturedNumImages) + imageCapture.vars.previousCompletedJpegNumImages;
							currentSession = 0;
							if ((1 == iEnableBrighterDataImageConfigPara) && (imageCapture.vars.executeBrighterDataImageTask))
							{
								imageCapture.vars.brighterDataImages = imageCapture.vars.previousBrighterDataImages;
							}
							
							//if ((iImageFilesTobeTransferred >> index) & 0x1)
							if ((imageCapture.vars.brighterDataImages >> index) & 0x1)
							{	
								imageIndex = index;
								
								if (imageIndex > MAX_IMAGES_PER_CAMERA)
								{
									printf("\r\nImage Index Exceeds Max Limit. %d", (imageIndex + 1));
									Debug_Output1(0,"\r\nImage Index Exceeds Max Limit. %d", (imageIndex + 1));
									break;	//TBD
								}
							}
							Debug_Output2(0,"\r\nPrevious End Index=%d , index=%d", imageIndex,index);
						}
						
						/* Before File create */
						if (imageCapture.vars.exitLoopUponAnotherCameraDetected)
						{
							Debug_Output1(0,"imageCapture.vars.exitLoopUponAnotherCameraDetected before resetting %d", imageCapture.vars.exitLoopUponAnotherCameraDetected);
							imageCapture.vars.exitLoopUponAnotherCameraDetected = 0;
						
						}
						if (imageCapture.vars.exitLoopUponAnotherCameraDetected)
						{
							/* Preserve Completed, Pending Jpeg to be created */
							imageCapture.vars.previousCompletedJpegNumImages = (frame/* - 1*/);
							imageCapture.vars.previousResidualJpegNumImages = imageCapture.vars.totalCapturedNumImages - frame;
							
							imageCapture.vars.previousJpegFullSizeImageCount = imageCapture.vars.jpegFullSizeImageCount;
							imageCapture.vars.previousJpegFullSizeImageNumbers = imageCapture.vars.jpegFullSizeImageNumbers;
							imageCapture.vars.previousJpegNumberPlateSizeImageCount = imageCapture.vars.jpegNumberPlateSizeImageCount;
							imageCapture.vars.previousJpegNumberPlateSizeImageNumbers = imageCapture.vars.jpegNumberPlateSizeImageNumbers;
							if ((1 == iEnableBrighterDataImageConfigPara) && (imageCapture.vars.executeBrighterDataImageTask))
							{
								imageCapture.vars.previousBrighterDataImages = imageCapture.vars.currentBrighterDataImages;
							}
							Debug_TextOut(0, "breaking 1 due to imageCapture.vars.exitLoopUponAnotherCameraDetected");
							break;
						}

						Debug_Output2(0,"\r\nJpeg_createTask.imageIndex=%d , index=%d", imageIndex,index);
						/* Create Color Jpeg Image */
						if (1 == writeColorJpeg)
						{				
							//if ((iImageFilesTobeTransferred >> index) & 0x1)
							if ((imageCapture.vars.brighterDataImages >> index) & 0x1)
							{
								JpegColorImage_create(selectedImageSensor, imageIndex, currentSession);
								#if 0
								/* Store Compressed Image to LPDDR RAM */						
								unsafe{
									//if (1 == iSelectedCamera)
									if (1 == selectedImageSensor)
									{
										totalNumCompressedImagesDdrCam1++;
										compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].number = totalNumCompressedImagesDdrCam1;
										compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].fileSize = outfileCount;
										memcpy(compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].fileName, newFileName, sizeof(newFileName));	/* Image File Name */
										memcpy(compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].data, jpegCompressedBuffer, sizeof(jpegCompressedBuffer));	/* Image Data */

										/*printf("\r\ncompressedImageCam1->image[%d].number: %d", totalNumCompressedImagesDdrCam1, compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].number);
										printf("\r\ncompressedImageCam1->image[%d].fileSize: %lu", totalNumCompressedImagesDdrCam1, compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].fileSize);
										printf("\r\ncompressedImageCam1->image[%d].fileName: %s", totalNumCompressedImagesDdrCam1, compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].fileName);							
										printf("\r\ncompressedImageCam1->image[%d].data[]: ", totalNumCompressedImagesDdrCam1);
										for (uint32_t ii = 0; ii < 10; ii++)
										{
											printf("%02X ", compressedImageCam1->image[totalNumCompressedImagesDdrCam1 - 1].data[ii]);
										}*/
									}
									//else if (2 == iSelectedCamera)
									else if (2 == selectedImageSensor)	
									{
										totalNumCompressedImagesDdrCam2++;
										compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].number = totalNumCompressedImagesDdrCam2;
										compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].fileSize = outfileCount;
										memcpy(compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].fileName, newFileName, sizeof(newFileName));	/* Image File Name */
										memcpy(compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].data, jpegCompressedBuffer, sizeof(jpegCompressedBuffer));	/* Image Data */

										/*printf("\r\ncompressedImageCam2->image[%d].number: %d", totalNumCompressedImagesDdrCam2, compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].number);
										printf("\r\ncompressedImageCam2->image[%d].fileSize: %lu", totalNumCompressedImagesDdrCam2, compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].fileSize);
										printf("\r\ncompressedImageCam2->image[%d].fileName: %s", totalNumCompressedImagesDdrCam2, compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].fileName);							
										printf("\r\ncompressedImageCam2->image[%d].data[]: ", totalNumCompressedImagesDdrCam2);
										for (uint32_t ii = 0; ii < 10; ii++)
										{
											printf("%02X ", compressedImageCam2->image[totalNumCompressedImagesDdrCam2 - 1].data[ii]);
										}*/
									}
								}	// best fcn to ddr -- /4 for uint8 to uint32
							#endif							
							}
						}
						

						/* Increment Image Count by 1 */
						if (++imageCaptureCount == imageCapture.vars.totalCapturedNumImages) 
						{
							imageCaptureCount = (imageCapture.vars.previousCompletedJpegNumImages == 0) ? 0 : imageCapture.vars.previousCompletedJpegNumImages;
						}

						/* After File create */
						if (imageCapture.vars.exitLoopUponAnotherCameraDetected)
						{
							/* Preserve Completed, Pending Jpeg to be created */
							imageCapture.vars.previousCompletedJpegNumImages = frame + 1;
							imageCapture.vars.previousResidualJpegNumImages = imageCapture.vars.totalCapturedNumImages - (frame + 1);
							
							imageCapture.vars.previousJpegFullSizeImageCount = imageCapture.vars.jpegFullSizeImageCount;
							imageCapture.vars.previousJpegFullSizeImageNumbers = imageCapture.vars.jpegFullSizeImageNumbers;
							imageCapture.vars.previousJpegNumberPlateSizeImageCount = imageCapture.vars.jpegNumberPlateSizeImageCount;
							imageCapture.vars.previousJpegNumberPlateSizeImageNumbers = imageCapture.vars.jpegNumberPlateSizeImageNumbers;
							if ((1 == iEnableBrighterDataImageConfigPara) && (imageCapture.vars.executeBrighterDataImageTask))
							{
								imageCapture.vars.previousBrighterDataImages = imageCapture.vars.currentBrighterDataImages;
							}
							Debug_TextOut(0, "breaking 2 due to imageCapture.vars.exitLoopUponAnotherCameraDetected");
							break;
						}
					}			
					
					/*if ((1 == selectedImageSensor) || (2 == selectedImageSensor))	//TBD //--only for testing 
					{
						outuchar(c_ready, 0);	//TBD //added on 21_06_2022			
					}*/
					
					//TODO:: add here ..Report Jpeg Creation Complete to TILE[0]...
					//printf("\r InterTileCommTile1_txInstance FROM TILE 1 before \n");
					/*if(frame >= createJpegNumImages)*/
					{
					outuchar(JPEG_Status_Send, JPEG_CREATION_COMPLETE_INSTANCE);
					/*frame = 0;*/
					}
					//Debug_Output2(0,"\r\nJpeg_createTask.frame=%d , createJpegNumImages=%d", frame,createJpegNumImages);
					//printf("\r InterTileCommTile1_txInstance FROM TILE 1 after \n");
					//InterTileCommTile1_txInstance(interTileInstance, &imageCapture, JPEG_CREATION_COMPLETE_INSTANCE);
					//return;	//TBD //commented on 20_06_2022					
				}
				break;
			}
		}
	}
}

/*
typedef chanend chanend_t;

extern "C" {
  void tile0_to_tile1(chanend_t);
  void tile1_to_tile0(chanend_t);
}
*/

////////////INTERTILE COMM///////////////

#define INTER_TILE_TAG "INTERTILE_COMM: "

typedef enum {
	HOST_MCU_NO_RESPONSE,
	HOST_MCU_VALID_RESPONSE,
	HOST_MCU_TILE1_BUSY
} HostMcuResponseStatus_t;

void InterTileCommTile0_waitResponse(uint8_t *Status)
{	
	/* Wait for Receive Response */ 	
	int waitTimeInMiliSeconds = 20;//1000;//txInstance.waitTimeInterval;
	
	for (int wait = 0; wait < waitTimeInMiliSeconds;  wait++) 
	{
		delay_milliseconds((int)1);
		
		if (1 == *Status)
		{			
			printf("\r\nstatusResponse: %d\n", *Status);
			return;
		}
	}		
}

void InterTileCommTile1_sendResponse(chanend txCmd, unsigned char packetType, unsigned char status, unsigned char *data)
{
	////printstr("\r\nInterTileCommTile1_sendResponse .....");
	/* Send Status/Data Response to TILE[0] */
	switch (status)
	{
		case HOST_MCU_NO_RESPONSE: 
			printstr("\r\nT[1]RX: No Response from HostMcu.");			
			break;
		case HOST_MCU_VALID_RESPONSE: 
			printstr("\r\nT[1]RX: Valid Response Received from HostMcu.");
			break;
		case HOST_MCU_TILE1_BUSY: 
			printstr("\r\nT[1]RX: Tile[1] Busy.");
			break;
		default:
			break;
	}
	
	/* Send Response Status to TILE[0] */
	outuchar(txCmd, status);

	/* Send Response Data to TILE[0] */
	if (HOST_MCU_VALID_RESPONSE == status)
	{
		switch (packetType)
		{
			case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
				printstr("\r\nT[1]TX: Image Deletion Table Response Sent.");				
				/* Delay To Fill vehicleOccupancyPacket.buffer */
				delay_milliseconds((int)2);
				/* Send vehicleOccupancyPacket Data to TILE[0] */
				//for (int count = 0; count < sizeof(VehicleOccupancyPacket_t); count++)
				for (int count = 0; count < IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH; count++)	
				{
					//outuchar(txCmd, vehicleOccupancyPacket.buffer[count]);
					outuchar(txCmd, data[count]);
				}
				break;
			case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
				printstr("\r\nT[1]TX: File Transfer Start Response Sent.");
				//NO Data to Send
				break;
			case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
				printstr("\r\nT[1]TX: File Transfer End Response Sent.");
				//NO Data to Send
				break;
			case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
				printstr("\r\nT[1]TX: Image File Header Response Sent.");
				//NO Data to Send
				break;
			default:
				break;
		}
	}
	
	////printstr("\r\nInterTileCommTile1_sendResponse ENDDDDDDD");
}

#if 1
#define HOST_MCU_PACKET_SEND_NUM_ATTEMPTS             4

unsigned char tile0VehicleOccupancyPacketBuffer[IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH];
uint8_t currentFileHeaderInfoTile0[205];

void InterTileCommTile0_sendPacketToHostMcuViaTile1(chanend txCmd, unsigned char packetType)
{
	int retry_count = 0;
for (retry_count = 0; retry_count < HOST_MCU_PACKET_SEND_NUM_ATTEMPTS;  retry_count++) 
	{	
	////printstr("\r\nInterTileCommTile0_sendPacketToHostMcuViaTile1..........");		
	switch (packetType)
    {
		case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
			printstr("\r\nT[0]TX: Image Deletion Table Request Sent.");		
			break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
		case HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE:
			printstr("\r\nT[0]TX: File Transfer Start Request Sent.");
			break;
		case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
			printstr("\r\nT[0]TX: File Transfer End Request Sent.");
			break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
			printstr("\r\nT[0]TX: Image File Header Packet Sent.");
            break;
        default:
            break;
    }

	/* Send Command to Host Mcu Via TILE[1] */
	outuchar(txCmd, packetType);
    
	/* Send Data/Info to TILE[1] */
    switch (packetType)
    {
		case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
			break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
		case HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE:
			break;
		case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
			break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
            //TODO:: add here .. Send IMG File Name to TILE[1]..
			//outuchar(txCmd, packetType);
			for (int count = 0; count < sizeof(currentFileHeaderInfoTile0); count++)
			{
				outuchar(txCmd, currentFileHeaderInfoTile0[count]);
			}
            break;
        default:
            break;
    }

	////printstr("\r\n/* Wait For Response from Host Mcu */");		
	/* Wait For Response from Host Mcu */
    unsigned char rxStatus = inuchar(txCmd);
	
	switch (rxStatus)
	{
		case HOST_MCU_NO_RESPONSE: 
			printstr("\r\nT[0]RX: No Response from HostMcu.");			
			break;
		case HOST_MCU_VALID_RESPONSE: 
			printstr("\r\nT[0]RX: Valid Response Received from HostMcu.");
			retry_count = 10;
			//TODO:: add here read and print data as per packetType.			
			break;
		case HOST_MCU_TILE1_BUSY: 
			printstr("\r\nT[0]RX: Tile[1] Busy.");
			break;
		default:
			printstr("\r\nT[0]RX: Tile[1] Deafult.");
			break;
	}
	
	/* Parse Valid Response */
	if (HOST_MCU_VALID_RESPONSE == rxStatus)
	{
		//unsigned char tile0VehicleOccupancyPacketBuffer[IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH];
		
		switch (packetType)
		{
			case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
				printstr("\r\nT[0]RX: Image Deletion Table Response Received");						
				/* Receive Image Deletion Table */
				//for (int count = 0; count < sizeof(VehicleOccupancyPacket_t); count++)
				for (int count = 0; count < IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH; count++)	
				{
					//tile0VehicleOccupancyPacket.buffer[count] = inuchar(txCmd);
					tile0VehicleOccupancyPacketBuffer[count] = inuchar(txCmd);
				}
				/* Parse and Process Vehicle Occupancy Status Info and Take Further Action such as deleting file, rearrange the file list. */
				int status = ImageDeletion_processVehicleOccupancyStatusInfo(tile0VehicleOccupancyPacketBuffer, sizeof(tile0VehicleOccupancyPacketBuffer));
				break;
			case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
			case HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE:
				printstr("\r\nT[0]RX: File Transfer Start Response Received.");
				////imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 0;
				//NO Receive Data
				break;
			case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
				printstr("\r\nT[0]RX: File Transfer End Response Received.");
				//NO Receive Data
				break;
			case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
				printstr("\r\nT[0]RX: Image File Header Response Received.");
				//NO Receive Data
				break;
			default:
				break;
		}
	}
	}
	////printstr("\r\nInterTileCommTile0_sendPacketToHostMcuViaTile1 ENDDDD");
	if(retry_count >= HOST_MCU_PACKET_SEND_NUM_ATTEMPTS)
	{
		/* Send Command to Host Mcu Via TILE[1] */
		////outuchar(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);
		////delay_milliseconds(100);
		/* Send Command to Host Mcu Via TILE[1] */
		////outuchar(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);
		////delay_milliseconds(100);		
		//InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);	
	}
	//return rxStatus;
}
#endif

void InterTileCommTile0_waitResponseFromHostMcuViaTile1(chanend txCmd, unsigned char packetType, unsigned char rxStatus)
{
	/* Wait For Response from Host Mcu */
    //unsigned char rxStatus = inuchar(txCmd);
	//printf("\r\n InterTileCommTile0_waitResponseFromHostMcuViaTile1 rxStatus(%d),packetType(%d) ", rxStatus,packetType);
	/*switch (rxStatus)
	{
		case HOST_MCU_NO_RESPONSE: 
			printf("\r\n%sNo Response from HostMcu.", INTER_TILE_TAG);
			break;
		case HOST_MCU_VALID_RESPONSE: 
			printf("\r\n%sValid Response Received from HostMcu...", INTER_TILE_TAG);
			//TODO:: add here read and print data as per packetType.			
			break;
		case HOST_MCU_TILE1_BUSY: 
			printf("\r\n%sTile[1] Busy. Retry.", INTER_TILE_TAG);
			break;
		default:
			break;
	}*/
	
	/* Parse Valid Response */
	if (HOST_MCU_VALID_RESPONSE == rxStatus)
	{
		//unsigned char tile0VehicleOccupancyPacketBuffer[IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH];
		
		switch (packetType)
		{
			case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
				printstr("\r\nT[0]RX: Image Deletion Table Response Received.");
				/* Receive Image Deletion Table */
				//for (int count = 0; count < sizeof(VehicleOccupancyPacket_t); count++)
				for (int count = 0; count < IMAGE_DELETION_TABLE_MESSAGE_MAX_PACKET_LENGTH; count++)	
				{
					//tile0VehicleOccupancyPacket.buffer[count] = inuchar(txCmd);
					tile0VehicleOccupancyPacketBuffer[count] = inuchar(txCmd);
				}
				/* Parse and Process Vehicle Occupancy Status Info and Take Further Action such as deleting file, rearrange the file list. */
				int status = ImageDeletion_processVehicleOccupancyStatusInfo(tile0VehicleOccupancyPacketBuffer, sizeof(tile0VehicleOccupancyPacketBuffer));
				break;
			case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
				printstr("\r\nT[0]RX: File Transfer Start Response Received..");
				//NO Receive Data
				//File_Transfer_Flag = 1;
				break;
			case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
				printstr("\r\nT[0]RX: File Transfer End Response Received.");
				//NO Receive Data
				//File_Transfer_Flag = 0;
				break;
			case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
				printstr("\r\nT[0]RX: Image File Header Response Received.");				
				//NO Receive Data
				break;
			default:
				break;
		}
	}

	//return rxStatus;
}



//void InterTileCommTile1_txInstance_seq_num(chanend c[], int seq_num)
//{
 
 //outuint(c[0], seq_num);
	
//}

//int InterTileCommTile0_rxInstance_seq_num(chanend c[])
//{

 //int seq_num;
 
 //seq_num = 	inuint(c[0]);
	
 //return seq_num;
	
//}
////////////SF////////////////////
/*uint8_t get_seq_num()
{
	chan seq_c;
	uint8_t seq_num;	
	seq_num = inuchar(seq_c);	
	return seq_num;	
}*/
/////////////////////////////////

//TODO:: InterTileCommTile0_rxInstance(instance[0], instance[1], instance[2], );
//void InterTileCommTile0_rxInstance(chanend sensorTriggerInstance, chanend imgCaptureDone, chanend jpegDone, chanend txCmd)
void InterTileCommTile0_rxInstance(chanend instance[])
{	
	unsigned char rxData = 0xFF;
	unsigned char packetType;	//TODO:: move this to global ...
	
	printf("\r\nInterTileCommTile0_rxData\n");
	
	memset(&imageCaptureTile0, 0, sizeof(ImageInfo_t));	
	
	unsafe{
		while (1)
		{
			select {
				case inuchar_byref(instance[2]/*imgCaptureDone*/, rxData):			
					if (SENSOR_TRIGGER_INSTANCE == rxData)		//Another Sensor Trigger
					{
						bootUpFlag = 0;
						imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;//inuint(instance[2]);	
						imageCaptureTile0.vars.transparentReadyStatus = 0;//inuint(instance[2]);
						printstr("\r\nT[0]RX: Sensor Trigger Instance..");	
						printint(rxData);
						//TODO:: add here ...stop FTP File xfer...Preserve Parameters such as currentFileNameBase (strcpy(imageCapture.vars.previousFileNameBase, FTPBaseFilename);), 
						//TBD ..need to check...											
						//File_Transfer_Flag = 0;					
					}
					else if (MODEM_FREE_START_FTP_INSTANCE == rxData)	//Another Sensor Trigger
					{
						bootUpFlag = 0;
						printstr("\r\nT[0]RX: Modem Free Start FTP Instance.."); 
						printint(rxData);
						//TODO:: add here ...
						//TBD ..need to check...						
						//File_Transfer_Flag = 1;
						imageCaptureTile0.vars.transparentReadyStatus = 1;//inuint(instance[2]);
						imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 0;//inuint(instance[2]);
						
						////outuint(instance[2], imageCaptureTile0.vars.transparentReadyStatus);
						////outuint(instance[2], imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected);
						
						////packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
						////InterTileCommTile0_sendPacketToHostMcuViaTile1(instance[6]/*txCmd*/, packetType);
					}
					else if (IMAGE_CAPTURE_COMPLETE_INSTANCE == rxData)	//Image Capture Complete
					{
						printstr("\r\nT[0]RX: Image Capture Complete Instance..");
						printint(rxData);

						//TODO:: add here Initiate HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE to start FTP File xfer..					
						imageCaptureTile0.vars.totalCapturedNumImages = inuint(instance[2]);
						////imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = inuint(instance[2]);						
						//packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
						//InterTileCommTile0_sendPacketToHostMcuViaTile1(instance[0]/*txCmd*/, packetType);
					}
					else if (JPEG_CREATION_START_INSTANCE == rxData)	//Jpeg Creation Complete
					{
						printf("\r\JPEG_CREATION_START_INSTANCE == rxData(%d)):\n",rxData);
						//TODO:: add here ..Stop FTP session rearrange file list to insert new images at start of file list. and continue sending files.
						//delay_milliseconds((int)5000);
						InterTileCommTile0_waitResponse(&imageCaptureTile0.vars.ftpRunStatus);
						outuint(instance[4], imageCaptureTile0.vars.ftpRunStatus);	
					}				
					else if (JPEG_CREATION_COMPLETE_INSTANCE == rxData)	//Jpeg Creation Complete
					{
						printstr("\r\nT[0]RX: Jpeg Creation Complete Instance..");
						printint(rxData);
						
						//TODO:: add here ..Stop FTP session rearrange file list to insert new images at start of file list. and continue sending files.

						imageCaptureTile0.vars.brighterDataImages = inuint(instance[2]);
						imageCaptureTile0.vars.currentBrighterDataImages = inuint(instance[2]);
						imageCaptureTile0.vars.previousBrighterDataImages = inuint(instance[2]);
						imageCaptureTile0.vars.previousTotalJpegImages = inuint(instance[2]);
						imageCaptureTile0.vars.executeBrighterDataImageTask = inuchar(instance[2]);
						
						imageCaptureTile0.vars.totalCapturedNumImages = inuint(instance[2]);
						imageCaptureTile0.vars.jpegFullSizeImageCount = inuint(instance[2]);
						imageCaptureTile0.vars.jpegFullSizeImageNumbers = inuint(instance[2]);
						imageCaptureTile0.vars.jpegNumberPlateSizeImageCount = inuint(instance[2]);
						imageCaptureTile0.vars.jpegNumberPlateSizeImageNumbers = inuint(instance[2]);
						imageCaptureTile0.vars.previousJpegFullSizeImageCount = inuint(instance[2]);
						imageCaptureTile0.vars.previousJpegFullSizeImageNumbers = inuint(instance[2]);
						imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount = inuint(instance[2]);
						imageCaptureTile0.vars.previousJpegNumberPlateSizeImageNumbers = inuint(instance[2]);
					
						//printf("\rJPEG_CREATION_COMPLETE_INSTANCE FINISHED, rxData(%d)):\n", rxData);
						//in_char_array(instance[3]/*rxInstance*/, imageCaptureTile0.vars.currentFileNameBase, sizeof(imageCaptureTile0.vars.currentFileNameBase));
						//in_char_array(instance[3]/*rxInstance*/, imageCaptureTile0.vars.previousFileNameBase, sizeof(imageCaptureTile0.vars.previousFileNameBase));				
						//imageCaptureTile0.vars.currentFileNameBaseSize = inuint(instance[2]);
						//imageCaptureTile0.vars.previousFileNameBaseSize = inuint(instance[2]);
						//TODO:: to be UNcommented
						for (int count = 0; count < sizeof(imageCaptureTile0.vars.currentFileNameBase); count++)
						{
							imageCaptureTile0.vars.currentFileNameBase[count] = inuchar(instance[2]);
						}
						for (int count = 0; count < sizeof(imageCaptureTile0.vars.previousFileNameBase); count++)
						{
							imageCaptureTile0.vars.previousFileNameBase[count] = inuchar(instance[2]);
						}
						//bootUpFlag = 1;
						/*printstr("\r\nT[0]currentFileNameBase=");
						printstr(imageCaptureTile0.vars.currentFileNameBase);
						printstr("\r\nT[0]previousFileNameBase=");
						printstr(imageCaptureTile0.vars.previousFileNameBase);
						printstr("\r\n");*/
						//delay_milliseconds((int)5000);
						////packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;

						//For starting share flash Immediately after capture
						imageCaptureTile0.vars.transparentReadyStatus = 1;//inuint(instance[2]);
						imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 0;
						
						//packetType = HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE;
						//InterTileCommTile0_sendPacketToHostMcuViaTile1(instance[6]/*txCmd*/, packetType);
					}
					else if (CAMERA_POWER_OFF_INSTANCE == rxData)	//Image Capture Complete
					{
						printstr("\r\nT[0]RX: Camera Power OFF Instance..");
						printint(rxData);

						//TODO:: add here Initiate HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE to start FTP File xfer..										
						//packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
						//InterTileCommTile0_sendPacketToHostMcuViaTile1(instance[0]/*txCmd*/, packetType);
					}
					else if (CAMERA_POWER_ON_INSTANCE == rxData)	//Image Capture Complete
					{
						printstr("\r\nT[0]RX: Camera Power ON Instance..");
						printint(rxData);

						//TODO:: add here Initiate HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE to start FTP File xfer..									
						//packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
						//InterTileCommTile0_sendPacketToHostMcuViaTile1(instance[0]/*txCmd*/, packetType);
					}					
					break;
				#if 0					
				//case inuchar_byref(instance[3]/*]txCmd*/, rxData):	
				//	printf("\r\inuchar_byref(instance[3]/*]txCmd*/, rxData(%d)):\n", rxData);
					//TODO:: add here ..get appropriate packetType...
				//	InterTileCommTile0_waitResponseFromHostMcuViaTile1(instance[3]/*]txCmd*/, packetType, rxData);
					//printf("\r\inuchar_byref(instance[0]/*]txCmd ENDDDD*/, rxData(%d):\n", rxData);
				//	break;
				#endif	
			}
		}
	}
}

void InterTileCommTile1_txInstance(chanend txInstance, ImageInfo_t * unsafe imageCap, unsigned char instance)
{
	unsigned char txData[7] = 
	{	
		SENSOR_TRIGGER_INSTANCE,
		IMAGE_CAPTURE_COMPLETE_INSTANCE,
		JPEG_CREATION_COMPLETE_INSTANCE,	
		JPEG_CREATION_START_INSTANCE,
		MODEM_FREE_START_FTP_INSTANCE,
		CAMERA_POWER_OFF_INSTANCE,
		CAMERA_POWER_ON_INSTANCE,
	};
	
	uint8_t transparentReadyStatusTemp = 0; 
	uint8_t exitLoopUponAnotherCameraDetected_Temp = 0;
	
	//printf("\r InterTileCommTile1_txInstance FROM TILE 1 %d\n",instance);
	outuchar(txInstance, txData[instance]);
	//printf("\r After  InterTileCommTile1_txInstance FROM TILE 1 After %d\n",instance);
	
	unsafe{
		if (SENSOR_TRIGGER_INSTANCE == instance)		//Another Sensor Trigger
		{
			//outuint(txInstance, 1/*imageCap->vars.exitLoopUponAnotherCameraDetected*/);
			//outuint(txInstance, 0/*imageCap->vars.transparentReadyStatus*/);
			printstr("\r\nT[1]TX: Sensor Trigger Instance.");
			Debug_TextOut(0,"T[1]TX: Sensor Trigger Instance.");
		}
		else if (MODEM_FREE_START_FTP_INSTANCE == instance)		//Modem Free Start FTP 
		{
			////outuint(txInstance, 1/*imageCap->vars.transparentReadyStatus*/);
			////outuint(txInstance, 0/*imageCap->vars.exitLoopUponAnotherCameraDetected*/);
			
			////transparentReadyStatusTemp = inuint(txInstance);
			////exitLoopUponAnotherCameraDetected_Temp = inuint(txInstance);
									
			printstr("\r\nT[1]TX: Modem Free Start FTP Instance.");
			Debug_TextOut(0,"T[1]TX: Modem Free Start FTP Instance.");
			////Debug_Output2(0,"transparentReadyStatusTemp=%d , exitLoopUponAnotherCameraDetected_Temp=%d",transparentReadyStatusTemp,exitLoopUponAnotherCameraDetected_Temp);
		}
		else if (IMAGE_CAPTURE_COMPLETE_INSTANCE == instance)	//Image Capture Complete
		{
			printstr("\r\nT[1]TX: Image Capture Complete Instance.");
			Debug_TextOut(0,"T[1]TX: Image Capture Complete Instance.");

			/* Send Additional Info to TILE[0] */
			outuint(txInstance, imageCap->vars.totalCapturedNumImages);			
		}
		else if (JPEG_CREATION_START_INSTANCE == instance)	//Jpeg Create Start Instance
		{
			printstr("\r\nT[1]TX: Jpeg Create Start Instance");
			Debug_TextOut(0,"T[1]TX: Jpeg Create Start Instance.");
			/* Send Additional Info to TILE[0] */
		}
		else if (JPEG_CREATION_COMPLETE_INSTANCE == instance)	//Jpeg Creation Complete
		{
			printstr("\r\nT[1]TX: Jpeg Creation Complete Instance.");
			Debug_TextOut(0,"T[1]TX: Jpeg Creation Complete Instance.");
			/* Send Additional Info to TILE[0] */

			outuint(txInstance, imageCap->vars.brighterDataImages);
			outuint(txInstance, imageCap->vars.currentBrighterDataImages);
			outuint(txInstance, imageCap->vars.previousBrighterDataImages);
			outuint(txInstance, imageCap->vars.previousTotalJpegImages);
			outuchar(txInstance, imageCap->vars.executeBrighterDataImageTask);
			
			outuint(txInstance, imageCap->vars.totalCapturedNumImages);
			outuint(txInstance, imageCap->vars.jpegFullSizeImageCount);
			outuint(txInstance, imageCap->vars.jpegFullSizeImageNumbers);
			outuint(txInstance, imageCap->vars.jpegNumberPlateSizeImageCount);
			outuint(txInstance, imageCap->vars.jpegNumberPlateSizeImageNumbers);
			outuint(txInstance, imageCap->vars.previousJpegFullSizeImageCount);
			outuint(txInstance, imageCap->vars.previousJpegFullSizeImageNumbers);
			outuint(txInstance, imageCap->vars.previousJpegNumberPlateSizeImageCount);
			outuint(txInstance, imageCap->vars.previousJpegNumberPlateSizeImageNumbers);
		
			//out_char_array(txInstance, imageCap->vars.currentFileNameBase, sizeof(imageCap->vars.currentFileNameBase));	
			//out_char_array(txInstance, imageCap->vars.previousFileNameBase, sizeof(imageCap->vars.previousFileNameBase));

			//outuint(txInstance, (sizeof(imageCap->vars.currentFileNameBase)));
			//outuint(txInstance, (sizeof(imageCap->vars.previousFileNameBase)));
			
			//TODO:: to be UNcommented
			for (int count = 0; count < sizeof(imageCap->vars.currentFileNameBase); count++)
			{
				outuchar(txInstance, imageCap->vars.currentFileNameBase[count]);
			}
			
			for (int count = 0; count < sizeof(imageCap->vars.previousFileNameBase); count++)
			{
				outuchar(txInstance, imageCap->vars.previousFileNameBase[count]);
			}
		
			/*printstr("\r\nT[1]currentFileNameBase=");
			printstr(imageCaptureTile0.vars.currentFileNameBase);
			printstr("\r\nT[1]previousFileNameBase=");
			printstr(imageCaptureTile0.vars.previousFileNameBase);
			printstr("\r\n");*/
			
			//Debug_TextOut(0,"TRANPARENT REQUEST SENT");
			Debug_TextOut(0,"IMAGE FILE MOVING TO SHARE FLASH PROCESS STARTED");
		}
		else if (CAMERA_POWER_OFF_INSTANCE == instance)	//Image Capture Complete
		{
			printstr("\r\nT[1]TX: Camera Power OFF Instance.");
			Debug_TextOut(0,"T[1]TX: Camera Power OFF Instance.");			
		}
		else if (CAMERA_POWER_ON_INSTANCE == instance)	//Image Capture Complete
		{
			printstr("\r\nT[1]TX: Camera Power OFF Instance.");
			Debug_TextOut(0,"T[1]TX: Camera Power OFF Instance.");			
		}
	}
}

//////////////////////INTERTILE END//////////////////
//void capture_thread(chanend tile0TxCmd, chanend instance[])

void capture_thread(chanend instance[])
{
#if ON_TILE(1)

#if 0
/*Current version V1.3 we need this*/
	int a = 0;
	a = readGpio();
	//UART_4BIT_TXD :> a;
	Debug_TextOut(0,"\r\nSELECTED CAMERA INDICATOR V1.3: ");
    printintln(a);  

	Debug_Output1(0,"SINGLE CAPTURE SELECTED CAMERA INDICATOR: = %ld",a);
	
	if((a & 0x08) == 0x08)
	{
		Debug_Output1(0,"LEFT CAMERA INDICATOR SELECTED: = %ld",(a & 0x08));		
		MIPI_SW_OE_PIN <: 0;	//Clear 
		MIPI_SW_SEL_PIN <: 1;	//Set
		a = 1;
	}	
	else if((a & 0x04) == 0x04)
	{
		Debug_Output1(0,"RIGHT CAMERA INDICATOR SELECTED: = %ld",(a & 0x04));
		//No Need to Set any Default Pin Out for Right Side
		a = 1;
	}
	else
	{
		Debug_Output1(0,"NO CAMERA SELECTED: = %ld",a);
		a = 1;
	}
#endif

#if 1
/*V1.4 Upcoming version we need this */
	int a = 0;
	a = readGpio();
	//UART_4BIT_TXD :> a;
	//Debug_TextOut(0,"\r\nXMOS H/W Version V1.4  F/W SF V1.0.0.2");	
	Debug_TextOut(0,"\r\nXMOS H/W Version V1.4  F/W SF V1.0.1.3_memfullfix_4");
	Debug_TextOut(0,"\r\nSELECTED CAMERA INDICATOR V1.4: V1");
    printintln(a);  

	Debug_Output1(0,"SINGLE CAPTURE SELECTED CAMERA INDICATOR: = %ld",a);
	
	if((a & 0x02) == 0x02)
	{
		Debug_Output1(0,"RIGHT CAMERA INDICATOR SELECTED: = %ld",(a & 0x02));
		//No Need to Set any Default Pin Out for Right Side
		a = 1;
	}	
	else if((a & 0x08) == 0x08)
	{
		Debug_Output1(0,"LEFT CAMERA INDICATOR SELECTED: = %ld",(a & 0x08));		
		MIPI_SW_OE_PIN <: 0;	//Clear 
		MIPI_SW_SEL_PIN <: 1;	//Set
		a = 1;
	}
	else
	{
		Debug_Output1(0,"NO CAMERA SELECTED: = %ld",a);
		a = 1;
	}	
#endif

	if(a > 0)
		{
    p_reset_camera @ 0 <: 0;
	p_reset_camera @ 1000 <: ~0;
	p_reset_camera @ 2000 <: 0;
	
	configure_in_port_strobed_slave(p_mipi_rxd, p_mipi_rxv, clk_mipi);
	set_clock_src(clk_mipi, p_mipi_clk);

	/* Sample on falling edge - shim outputting on rising */
    #ifdef DELAY_MIPI_CLK // TODO should be #if?
	    set_clock_rise_delay(clk_mipi, DELAY_MIPI_CLK);
    #else
	    set_clock_rise_delay(clk_mipi, 0);
    #endif
	set_pad_delay(p_mipi_rxa, 1);
	start_clock(clk_mipi);

	write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_MIPI_DPHY_CFG3_NUM, 0x7042);	// // two lanes  //Polarity Same

    //x[0] = 12;	// gets the compiler to turn on DDR for us on this tile
  
    /*#ifdef CLK_600
        printstr("\nRunning at 600MHz.\r\n");
    #else
        printstr("\nRunning at 800MHz.\r\n");
    #endif*/
	printstr("\nRunning at 600MHz.\r\n");

    #undef MIPI_CLK_DIV
    #define MIPI_CLK_DIV (0) // for secondary PLL
    
    #define MY_BYPASS (0) // bit 29
    #define MY_INPUT_FROM_SYS_PLL (0) // bit 28  (use 24MHz crystal)
    #define MY_ENABLE (1) // bit 27
    #define MY_POST_DIVISOR (0x00) //bits 25:23
    #define MY_FEEDBACK_MUL (0xDB) // bits 20:8  24 * ((F+1)/2)
    #define MY_INPUT_DIVISOR (0x05) // bits 5:0   / (R+1)
    
    const unsigned PLL_SETTINGS = 0 
                                 | MY_BYPASS << 29
                                 | MY_INPUT_FROM_SYS_PLL << 28
                                 | MY_ENABLE << 27
                                 | MY_POST_DIVISOR << 23
                                 | MY_FEEDBACK_MUL << 8
                                 | MY_INPUT_DIVISOR;
                                 
    
	write_node_config_reg ( tile[MIPI_TILE] , XS1_SSWITCH_SS_APP_PLL_CTL_NUM , 0) ;// enable APP PLL
	printstr("\r\nEnabled APP PLL");
	write_node_config_reg( tile[MIPI_TILE], XS1_SSWITCH_SS_APP_PLL_CTL_NUM, PLL_SETTINGS);
	printf("\r\nConfigured APP PLL 0x%08X",  PLL_SETTINGS);
    write_node_config_reg( tile[MIPI_TILE] , XS1_SSWITCH_MIPI_CLK_DIVIDER_NUM, 0x80000000 ); // use secondary PLL and only divide by 2
	printstr("\nConfigured MIPI CLK DIVIDER");
	printstr("\r\nMIPI_TILE: ");
    printintln(MIPI_TILE);     
    printstr("\r\nTEST_DEMUX_EN: ");
    printhex(TEST_DEMUX_EN); 
    printstr("\r\nTEST_DEMUX_DATATYPE: ");
    printhex(TEST_DEMUX_DATATYPE); 
    printstr("\r\nTEST_DEMUX_MODE: ");
    printhex(TEST_DEMUX_MODE); 
    printstr("\r\nMIPI_CLK_DIV: ");
    printhex(MIPI_CLK_DIV); 
    printstr("\r\nMIPI_CFG_CLK_DIV: ");
    printhex(MIPI_CFG_CLK_DIV);
    printstr("\r\nLANES: ");
    printhex(LANES); 	 	
    printstr("\r\n");
	unsigned rdata;
	read_node_config_reg(tile[MIPI_TILE],XS1_SSWITCH_MIPI_CLK_DIVIDER_NUM, rdata);
    printstr("\r\nXS1_SSWITCH_MIPI_CLK_DIVIDER_NUM: ");
    printhex(rdata); 	 	
    printstr("\r\n");
		}
if(a == 0)
		{
			/* Sets the PLL, 0D = 2, F=109 (0x6D), R=0. So Fcore=440MHz @ Fcry=24MHz */
			//write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_PLL_CTL_NUM, 0x01006D00);
			//write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_PLL_CTL_NUM, 0x81006D00);	//bit31=1 chip will not reset 

			/* Sets the PLL, 0D = 2, F=219 (0xDB), R=0. So Fcore=880MHz @ Fcry=24MHz */
			//write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_PLL_CTL_NUM, 0x0100DB00);	
			//write_node_config_reg(tile[MIPI_TILE], XS1_SSWITCH_PLL_CTL_NUM, 0x8100DB00);	//bit31=1 chip will not reset			
		}
    chan c;
    chan c_kill;
	chan c_img;
	chan c_imgCount;
	
	chan c_ctrl;
	chan tile1TxCmd;
	chan JPEG_Status_Send;
	chan txd_rxd_channel;
	chan seq_c;
#endif

#if ON_TILE(1)
if(a == 0)
{
    par
	{
            /*MipiReceive(tile[MIPI_TILE], LANES, c, p_mipi_rxd, p_mipi_rxa, c_kill,
                    TEST_DEMUX_EN, TEST_DEMUX_DATATYPE, TEST_DEMUX_MODE,
                    MIPI_CLK_DIV, MIPI_CFG_CLK_DIV);           
            MipiDecoupler(c, c_kill, c_img);
			MipiImager(c_img, c_ctrl, tile1TxCmd, c_imgCount);*/
			HostMcuUart_ReceiveData(c_ctrl, instance[1], txd_rxd_channel,seq_c);
			HostMcuUart_txData(instance[0], tile1TxCmd, instance[2], JPEG_Status_Send, instance[3], txd_rxd_channel,instance[6]);
			//Jpeg_createTask(c_imgCount,JPEG_Status_Send,instance[4]);		
	}
}
else
{
    par
	{
            /*MipiReceive(tile[MIPI_TILE], LANES, c, p_mipi_rxd, p_mipi_rxa, c_kill,
                    TEST_DEMUX_EN, TEST_DEMUX_DATATYPE, TEST_DEMUX_MODE,
                    MIPI_CLK_DIV, MIPI_CFG_CLK_DIV);*/  
            MipiReceive(tile[MIPI_TILE], LANES, c, p_mipi_rxd, p_mipi_rxa, c_kill,
                    TEST_DEMUX_EN, TEST_DEMUX_DATATYPE, TEST_DEMUX_MODE,
                    MIPI_CLK_DIV, MIPI_CFG_CLK_DIV); 					
            MipiDecoupler(c, c_kill, c_img);
			MipiImager(c_img, c_ctrl, tile1TxCmd, c_imgCount);
			HostMcuUart_ReceiveData(c_ctrl, instance[1], txd_rxd_channel,seq_c);
			HostMcuUart_txData(instance[0], tile1TxCmd, instance[2], JPEG_Status_Send, instance[3], txd_rxd_channel,instance[6]);
			Jpeg_createTask(c_imgCount,JPEG_Status_Send,instance[4]);
	}
}
#endif
	/*while(1)
	{
		
	}*/		
	//return 0;	//TBD
}

void camera_main(chanend txCmd, chanend instance[]) 
{
	
	//chan txCmd;
    //par
	{
        //on tile[MIPI_TILE]:
		{
			#if ON_TILE(MIPI_TILE)
			printf("\r\ncapture_thread(txCmd);%ld", txCmd);
			par
			{
				////capture_thread(txCmd, instance);
			}
			#endif
			
			#if ON_TILE(0)
			printf("\r\nHostMcuUart_testIntertile(txCmd);%ld", txCmd);
			par
			{
				//start_FreeRTOS_Task();
			}
			#endif
        }
    }	
    //return 0;
}

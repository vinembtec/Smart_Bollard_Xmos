// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "ffconf.h"
#include "ff.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "xmos_config.h"	//"filesystem/xmos_config.h"
#include "fs_support.h"

#include "ff.h"	
	
//#pragma stackfunction 5000	
   
#define DEMO_FILEPATH		"/flash/config.ini"
#define TIME_DECOUPLER (1)

#define MAX_CONFIG_VARIABLE_LEN 200
#define CONFIG_LINE_BUFFER_SIZE 100//2000 //100
#define MAX_LLIST_NAME_LEN 256
#define MAX_OUT_NAME_LEN 256
/*
# Total bytes per line
TBPL 70

# bar minimum
NUMIN 10.
# bar maximum
NUMAX 30000.

# input filename
LLIST_NAME DEMO_FILEPATH	//foo.txt
*/

	//uint8_t uint8_t_data;
	//uint16_t uint16_t_data;
	//uint32_t uint32_t_data;
struct config_struct { 
    int bytes_per_line;
    char llist_name[MAX_LLIST_NAME_LEN];
    double numin, numax;
} config;

extern writeColorJpeg;

uint8_t xmosConfigInitFinished = 0;

extern void rtos_fatfs_init_tast_call(void);

//uint8_t cfg_Param_string_len;
char
	Region[6],
	License_Filename[48],
	SCPServerIPAddress[16],		
	SCPServerPwd[16],				
	SCPServerusername[16],
	SDCardDriveName[16],		
	UsbDrivePath[32],
	FTPServerIPAddress[32],	
	FTPServerusername[16],			
	FTPServerPwd[32];
	ModemPartNumber[16],	
	ModemTtyPort[16];	
	
//int Buff_Count;		
uint8_t
	Buff_Count,		
	Num_images,
	Camera_1,
	Camera_2,
	DeleteImagesNoStableState,
	DeleteJpegFiles,
	DeleteRawFiles,
	Vertical_flip,
	Write_Graw1,
	Write_Graw2,
	Write_Rawfile,
	Write_Rlm,
	grayscale_endianness,
	//writeColorJpeg,
	writeGrayscaleJpeg,			
	JPEG_Quality,	
	AnprEnable,
	EnableBrighterDataImage,		
	BrighterDataImageStartTime,
	BrighterDataImageEndTime,
	WiFi_Transfer,
	UART_baudRate,	
	UART_stopBit,
	UART_ttymxc,
	FTPEnable,				
	FTPFileSendOption,
	FTPServerPortnumber,
		
	FileDownloadEnable,				
	FileDownloadCrcCheck,
	SendFtpStatus,
	SomLogVerboseLevel,		
	PrintDebugMessage,
	SomLogEnable,
	LOGLEVEL_WAKE_SLEEP_ACTIVETIME,
	LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR,
	LOGLEVEL_COMMS,
	LOGLEVEL_FILESAVE_TRANSP_FTP,
	LogfilesizeLimit_in_KB;
	
uint16_t
	Delay_between_images,
	DelayBetweenImagesVehicleExit,
	CenterRowStart,			
	CenterColStart,
	CenterWidth,
	CenterHeight,
	
	//[Grayscale_Buffer]
	Col_Start,				
	Gray_Cols,
	Gray_Rows,
	Row_Start,

	//[Input_Camera_Image]
	Cols,					
	Rows,
	LeftMID,			
	RightMID,
	CustomerId,
	BrighterDataCountThreshold,	
	Crc16Polynomial,
	Crc16InitialValue,
	ModemPidNumber,				
	LastPacketDelay,
	InterPacketDelay,
	FTPResponseTimeout,
	Preserve_Last_JPEGImage_File,
	FTPTimeout;	
	
uint32_t
		ImageFilesTobeTransferred,	
		BatchedImagesToSend;
		
static int read_int_from_config_line(char* config_line, uint8_t* cfg_param_string_len) {
    //port_out(XS1_PORT_4C, 0x09);
	char  separator, prm_name[MAX_CONFIG_VARIABLE_LEN];
    int val;
	char format_specifier[10]={" "},temp[10];
	sprintf(&temp, "%ds", cfg_param_string_len);
	strcat(format_specifier,"%");
	strcat(format_specifier,temp);
	strcat(format_specifier," %*c %d\n");
	
	//rtos_printf("format_specifier: %s ",format_specifier);	
    //sscanf(config_line, "%10s %*c %d\n", prm_name,  &val);
	sscanf(config_line, format_specifier, prm_name,  &val);
	//rtos_printf("param name: %s  val: %d\n ",prm_name, val);
	//port_out(XS1_PORT_4C, 0x00);	
    return val;
}

/*
void read_double_from_config_line(char* config_line, double* val) {    
    char prm_name[MAX_CONFIG_VARIABLE_LEN];
	char format_specifier[10]={" "},temp[10];
	
	sprintf(&temp, "%ds", cfg_Param_string_len);
	strcat(format_specifier,"%");
	strcat(format_specifier,temp);
	strcat(format_specifier," %*c %lf\n");
	
    //sscanf(config_line, "%s %lf\n", prm_name, val);
	sscanf(config_line, format_specifier, prm_name, val);
	//rtos_printf("param name: %s  val: %lf\n ",prm_name, val);
}*/

static void read_str_from_config_line(char* config_line, char* val,uint8_t* cfg_param_string_len) {    
    char prm_name[MAX_CONFIG_VARIABLE_LEN];
	char format_specifier[10]={" "},temp[10];
	
	sprintf(&temp, "%ds", cfg_param_string_len);
	strcat(format_specifier,"%");
	strcat(format_specifier,temp);
	strcat(format_specifier," %*c %s\n");
    
	//rtos_printf("format_specifier: %s ",format_specifier);
	
	//sscanf(config_line, "%s %s\n", prm_name, val);
	sscanf(config_line, format_specifier, prm_name, val);
	//rtos_printf("param name: %s  val: %s\n ",prm_name, val);
}

#pragma stackfunction 1000
void XmosConfig_read(void/** args*/)
{	
    FIL test_file;
    FRESULT result;
    //uint32_t demo_file_size = -1;
    //uint32_t bytes_read = 0;
    //uint8_t *file_contents_buf = NULL;
	int i = 0;   
	unsigned now,later;
	char buf[CONFIG_LINE_BUFFER_SIZE];
	////////////////////////////////////
	
	rtos_printf("XmosConfig_read %s\n", DEMO_FILEPATH);
#if 1
		//port_out(XS1_PORT_4C, 0x09);
		result = f_open(&test_file, DEMO_FILEPATH, FA_READ);
		//port_out(XS1_PORT_4C, 0x00);
		if (result == FR_OK)
		{
			rtos_printf("Found file %s\n", DEMO_FILEPATH);

		} else {
			rtos_printf("Failed to open file %s\n", DEMO_FILEPATH);
		}
		int no_of_lines = 0;
		uint8_t temp_uint8t;
		uint16_t temp_uint16t;
		uint16_t temp_uint32t;
		uint32_t bytes_read = 0;
		
		//f_gets(buf, sizeof buf, &test_file);
		
		/*result = f_read(&test_file,
                        (uint8_t*)buf,
                        CONFIG_LINE_BUFFER_SIZE,
                        (unsigned int*)&bytes_read);*/
						
		//rtos_printf("Contents of %s are:\n%s\n", DEMO_FILEPATH, buf);
		
		while (f_gets(buf, sizeof buf, &test_file)) 
		{
			////port_out(XS1_PORT_4C, 0x00);
			//rtos_printf("String is : %s",buf);
			no_of_lines++;
			if (buf[0] == '#' || strlen(buf) < 4) 
			{
				continue;
			}
			if (strstr(buf, "Buff_Count"))				
			{
				//cfg_Param_string_len = strlen("Buff_Count");
				temp_uint8t= read_int_from_config_line(buf, strlen("Buff_Count"));
				
				if ((temp_uint8t < 0) || (temp_uint8t > 20))
				{
					Buff_Count = 8;	// 
				}
				else
					Buff_Count = temp_uint8t;
				rtos_printf("	Buff_Count:	%d \n ",Buff_Count);
			}
			
			if (strstr(buf, "Num_images")) {
				//cfg_Param_string_len = strlen("Num_images");
				temp_uint8t= read_int_from_config_line(buf, strlen("Num_images"));
				//config.bytes_per_line= read_int_from_config_line(buf);
				if ((temp_uint8t < 0) || (temp_uint8t > 20))
				{
					Num_images = 8;	// 
				}
				else
					Num_images = temp_uint8t;
				rtos_printf("	Num_images:	%d \n ",Num_images);
			}
			
			if (strstr(buf, "Camera_1")) {
				//cfg_Param_string_len = strlen("Camera_1");
				temp_uint8t= read_int_from_config_line(buf, strlen("Camera_1"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Camera_1 = 1;	// 
				}
				else
					Camera_1 = temp_uint8t;
				rtos_printf("	Camera_1:	%d \n ",Camera_1);
			}
			if (strstr(buf, "Camera_2")) {
				//cfg_Param_string_len = strlen("Camera_2");
				temp_uint8t= read_int_from_config_line(buf, strlen("Camera_2"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Camera_2 = 0;	// 
				}
				else
					Camera_2 = temp_uint8t;
				rtos_printf("	Camera_2:	%d \n ",Camera_2);
			}			
//#if 0			
			if (strstr(buf, "Delay_between_images")) {
				//cfg_Param_string_len = strlen("Delay_between_images");
				temp_uint16t= read_int_from_config_line(buf, strlen("Delay_between_images"));
				if ((temp_uint16t < 0) || (temp_uint16t > 50000))
				{
					Delay_between_images = 0;	// 
				}
				else
					Delay_between_images = temp_uint16t;
				rtos_printf("	Delay_between_images:	%d \n ",Delay_between_images);
			}
			
			////////
			if (strstr(buf, "DelayBetweenImagesVehicleExit")) {
				//cfg_Param_string_len = strlen("DelayBetweenImagesVehicleExit");
				temp_uint16t= read_int_from_config_line(buf, strlen("DelayBetweenImagesVehicleExit"));
				if ((temp_uint16t < 0) || (temp_uint16t > 50000))
				{
					DelayBetweenImagesVehicleExit = 0;	// 
				}
				else
					DelayBetweenImagesVehicleExit = temp_uint16t;
				rtos_printf("	DelayBetweenImagesVehicleExit:	%d \n ",DelayBetweenImagesVehicleExit);
			}
			
			if (strstr(buf, "DeleteImagesNoStableState")) {
				//cfg_Param_string_len = strlen("DeleteImagesNoStableState");
				temp_uint8t= read_int_from_config_line(buf, strlen("DeleteImagesNoStableState"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					DeleteImagesNoStableState = 0;	// 
				}
				else
					DeleteImagesNoStableState = temp_uint8t;
				rtos_printf("	DeleteImagesNoStableState:	%d \n ",DeleteImagesNoStableState);
			}
			
			if (strstr(buf, "DeleteJpegFiles")) {
				//cfg_Param_string_len = strlen("DeleteJpegFiles");
				temp_uint8t= read_int_from_config_line(buf, strlen("DeleteJpegFiles"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					DeleteJpegFiles = 0;	// 
				}
				else
					DeleteJpegFiles = temp_uint8t;
				rtos_printf("	DeleteJpegFiles:	%d \n ",DeleteJpegFiles);
			}
			if (strstr(buf, "DeleteRawFiles")) {
				//cfg_Param_string_len = strlen("DeleteRawFiles");
				temp_uint8t= read_int_from_config_line(buf, strlen("DeleteRawFiles"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					DeleteRawFiles = 0;	// 
				}
				else
					DeleteRawFiles = temp_uint8t;
				rtos_printf("	DeleteRawFiles:	%d \n ",DeleteRawFiles);
			}			
			
			if (strstr(buf, "ImageFilesTobeTransferred")) {
				//cfg_Param_string_len = strlen("ImageFilesTobeTransferred");
				temp_uint32t= read_int_from_config_line(buf, strlen("ImageFilesTobeTransferred"));
				/*if ((temp_uint32t < 0) || (temp_uint8t > 50000))
				{
					ImageFilesTobeTransferred = 0;	// 
				}
				else*/
				ImageFilesTobeTransferred = temp_uint32t;
				rtos_printf("	ImageFilesTobeTransferred:	%ld \n ",ImageFilesTobeTransferred);
			}
			if (strstr(buf, "BatchedImagesToSend")) {
				//cfg_Param_string_len = strlen("BatchedImagesToSend");
				temp_uint32t= read_int_from_config_line(buf, strlen("BatchedImagesToSend"));
				/*if ((temp_uint32t < 0) || (temp_uint8t > 50000))
				{
					BatchedImagesToSend = 0;	// 
				}
				else*/
				BatchedImagesToSend = temp_uint32t;
				rtos_printf("	BatchedImagesToSend:	%ld \n ",BatchedImagesToSend);
			}
//#if 0
			if (strstr(buf, "LeftMID")) {
				//cfg_Param_string_len = strlen("LeftMID");
				temp_uint16t= read_int_from_config_line(buf, strlen("LeftMID"));
				if ((temp_uint16t < 32001) || (temp_uint16t > 35000))
				{
					LeftMID = 32001;	// 
				}
				else
					LeftMID = temp_uint16t;
				rtos_printf("	LeftMID:	%d \n ",LeftMID);
			}
			if (strstr(buf, "RightMID")) {
				//cfg_Param_string_len = strlen("RightMID");
				temp_uint16t= read_int_from_config_line(buf, strlen("RightMID"));
				if ((temp_uint16t < 32001) || (temp_uint16t > 35000))
				{
					RightMID= 32001;	// 
				}
				else
					RightMID = temp_uint16t;
				rtos_printf("	RightMID:	%d \n ",RightMID);
			}
			if (strstr(buf, "CustomerId")) {
				//cfg_Param_string_len = strlen("CustomerId");
				temp_uint16t= read_int_from_config_line(buf, strlen("CustomerId"));
				if ((temp_uint16t < 4000) || (temp_uint16t > 9000))
				{
					CustomerId= 8068;	// 
				}
				else
					CustomerId= temp_uint16t;
				rtos_printf("	CustomerId:	%d \n ",CustomerId);
			}
			if (strstr(buf, "AnprEnable")) {
				//cfg_Param_string_len = strlen("AnprEnable");
				temp_uint8t= read_int_from_config_line(buf, strlen("AnprEnable"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					AnprEnable= 0;	// 
				}
				else
					AnprEnable= temp_uint8t;
				rtos_printf("	AnprEnable:	%d \n ",AnprEnable);
			}

			if (strstr(buf, "Region")) {
				//cfg_Param_string_len = strlen("Region");
				read_str_from_config_line(buf, config.llist_name, strlen("Region"));
				strcpy(Region,config.llist_name);
				rtos_printf("	ALPR/ANPR Region:	%s \n ",Region);
			}
//#if 0			
			if (strstr(buf, "License_Filename")) {
				//cfg_Param_string_len = strlen("License_Filename");
				read_str_from_config_line(buf, config.llist_name, strlen("License_Filename"));
				strcpy(License_Filename,config.llist_name);
				rtos_printf("	License_Filename:	%s \n ",License_Filename);
			}	
			if (strstr(buf, "SCPServerIPAddress")) {
				//cfg_Param_string_len = strlen("SCPServerIPAddress");
				read_str_from_config_line(buf, config.llist_name, strlen("SCPServerIPAddress"));
				strcpy(SCPServerIPAddress,config.llist_name);
				rtos_printf("	SCPServerIPAddress:	%s \n ",SCPServerIPAddress);
			}		
			if (strstr(buf, "SCPServerPwd")) {
				//cfg_Param_string_len = strlen("SCPServerPwd");
				read_str_from_config_line(buf, config.llist_name, strlen("SCPServerPwd"));
				strcpy(SCPServerPwd,config.llist_name);
				rtos_printf("	SCPServerPwd:	%s \n ",SCPServerPwd);
			}	
			if (strstr(buf, "SCPServerusername")) {
				//cfg_Param_string_len = strlen("SCPServerusername");
				read_str_from_config_line(buf, config.llist_name, strlen("SCPServerusername"));
				strcpy(SCPServerusername ,config.llist_name);
				rtos_printf("	SCPServerusername:	%s \n ",SCPServerusername );
			}		
			if (strstr(buf, "SDCardDriveName")) {
				//cfg_Param_string_len = strlen("SDCardDriveName");
				read_str_from_config_line(buf, config.llist_name, strlen("SDCardDriveName"));
				strcpy(SDCardDriveName ,config.llist_name);
				rtos_printf("	SDCardDriveName:	%s \n ",SDCardDriveName );
			}
//#if 0
			if (strstr(buf, "UART_baudRate")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("UART_baudRate"));
				UART_baudRate= temp_uint8t;
				rtos_printf("	UART_baudRate:	%d \n ",UART_baudRate);
			}	
			if (strstr(buf, "UART_stopBit")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("UART_stopBit"));
				UART_stopBit= temp_uint8t;
				rtos_printf("	UART_stopBit:	%d \n ",UART_stopBit);
			}	
			if (strstr(buf, "UART_ttymxc")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("UART_ttymxc"));
				UART_ttymxc= temp_uint8t;
				rtos_printf("	UART_ttymxc:	%d \n ",UART_ttymxc);
			}	
			if (strstr(buf, "Vertical_flip")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Vertical_flip"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Vertical_flip= 0;	// 
				}
				else
					Vertical_flip= temp_uint8t;
				rtos_printf("	Vertical_flip:	%d \n ",Vertical_flip);
			}
			if (strstr(buf, "WiFi_Transfer")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("WiFi_Transfer"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					WiFi_Transfer= 1;	// 
				}
				else
					WiFi_Transfer= temp_uint8t;
				rtos_printf("	WiFi_Transfer:	%d \n ",WiFi_Transfer);
			}
			if (strstr(buf, "Write_Graw1")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Write_Graw1"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Write_Graw1= 0;	// 
				}
				else
					Write_Graw1= temp_uint8t;
				rtos_printf("	Write_Graw1:	%d \n ",Write_Graw1);
			}
			if (strstr(buf, "Write_Graw2")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Write_Graw2"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Write_Graw2= 0;	// 
				}
				else
					Write_Graw2= temp_uint8t;
				rtos_printf("	Write_Graw2:	%d \n ",Write_Graw2);
			}
			if (strstr(buf, "Write_Rawfile")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Write_Rawfile"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Write_Rawfile= 0;	// 
				}
				else
					Write_Rawfile= temp_uint8t;
				rtos_printf("	Write_Rawfile:	%d \n ",Write_Rawfile);
			}
			if (strstr(buf, "Write_Rlm")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Write_Rlm"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					Write_Rlm= 0;	// 
				}
				else
					Write_Rlm= temp_uint8t;
				rtos_printf("	Write_Rlm:	%d \n ",Write_Rlm);
			}
			if (strstr(buf, "grayscale_endianness")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("grayscale_endianness"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					grayscale_endianness= 0;	// 
				}
				else
					grayscale_endianness= temp_uint8t;
				rtos_printf("	grayscale_endianness:	%d \n ",grayscale_endianness);
			}
			if (strstr(buf, "writeColorJpeg")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("writeColorJpeg"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					writeColorJpeg= 1;	// 
				}
				else
					writeColorJpeg= temp_uint8t;
				rtos_printf("	writeColorJpeg:	%d \n ",writeColorJpeg);
			}
			if (strstr(buf, "writeGrayscaleJpeg")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("writeGrayscaleJpeg"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					writeGrayscaleJpeg= 0;	// 
				}
				else
					writeGrayscaleJpeg= temp_uint8t;
				rtos_printf("	writeGrayscaleJpeg:	%d \n ",writeGrayscaleJpeg);
			}
			if (strstr(buf, "FTPEnable")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("FTPEnable"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					FTPEnable= 1;	// 
				}
				else
					FTPEnable= temp_uint8t;
				rtos_printf("	FTPEnable:	%d \n ",FTPEnable);
			}			
			if (strstr(buf, "FTPFileSendOption")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("FTPFileSendOption"));
				if ((temp_uint8t < 0) || (temp_uint8t > 3))
				{
					FTPFileSendOption= 3;	// 
				}
				else
					FTPFileSendOption= temp_uint8t;
				rtos_printf("	FTPFileSendOption:	%d \n ",FTPFileSendOption);
			}			
			if (strstr(buf, "FTPServerIPAddress")) {
				read_str_from_config_line(buf, config.llist_name, strlen("FTPServerIPAddress"));
				strcpy(FTPServerIPAddress,config.llist_name);
				rtos_printf("	FTPServerIPAddress:	%s \n ",FTPServerIPAddress);
			}
			if (strstr(buf, "FTPServerusername")) {
				read_str_from_config_line(buf, config.llist_name, strlen("FTPServerusername"));
				strcpy(FTPServerusername,config.llist_name);
				rtos_printf("	FTPServerusername:	%s \n ",FTPServerusername);
			}
			if (strstr(buf, "FTPServerPwd")) {
				read_str_from_config_line(buf, config.llist_name, strlen("FTPServerPwd"));
				strcpy(FTPServerPwd,config.llist_name);
				rtos_printf("	FTPServerPwd:	%s \n ",FTPServerPwd);
			}
			if (strstr(buf, "FTPServerPortnumber")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("FTPServerPortnumber"));
				FTPServerPortnumber= temp_uint8t;
				rtos_printf("	FTPServerPortnumber:	%d \n ",FTPServerPortnumber);
			}
//#if 0			
			if (strstr(buf, "FTPTimeout")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("FTPTimeout"));
				if ((temp_uint16t < 100) || (temp_uint16t > 5000))
				{
					FTPTimeout= 100;	// 
				}
				else
					FTPTimeout= temp_uint16t;
				rtos_printf("	FTPTimeout:	%d \n ",FTPTimeout);
			}
			if (strstr(buf, "FileDownloadEnable")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("FileDownloadEnable"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					FileDownloadEnable= 1;	// 
				}
				else
					FileDownloadEnable= temp_uint8t;
				rtos_printf("	FileDownloadEnable:	%d \n ",FileDownloadEnable);
			}
			if (strstr(buf, "FileDownloadCrcCheck")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("FileDownloadCrcCheck"));
				if ((temp_uint8t < 0) || (temp_uint8t > 3))
				{
					FileDownloadCrcCheck= 1;	// 
				}
				else
					FileDownloadCrcCheck= temp_uint8t;
				rtos_printf("	FileDownloadCrcCheck:	%d \n ",FileDownloadCrcCheck);
			}
			if (strstr(buf, "SomLogVerboseLevel")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("SomLogVerboseLevel"));
				if ((temp_uint8t < 0) || (temp_uint8t > 3))
				{
					SomLogVerboseLevel= 3;	// 
				}
				else
					SomLogVerboseLevel= temp_uint8t;
				rtos_printf("	SomLogVerboseLevel:	%d \n ",SomLogVerboseLevel);
			}
			if (strstr(buf, "Crc16Polynomial")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("Crc16Polynomial"));
				Crc16Polynomial= temp_uint16t;
				rtos_printf("	Crc16Polynomial:	%d \n ",Crc16Polynomial);
			}
			if (strstr(buf, "Crc16InitialValue")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("Crc16InitialValue"));
				if ((temp_uint16t < 0) || (temp_uint16t > 0xFFFF))
				{
					Crc16InitialValue= 0;	// 
				}
				else
					Crc16InitialValue= temp_uint16t;
				rtos_printf("	Crc16InitialValue:	%d \n ",Crc16InitialValue);
			}
			
			
			if (strstr(buf, "JPEG_Quality")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("JPEG_Quality"));
				if ((temp_uint8t < 50) || (temp_uint8t > 100))
				{
					JPEG_Quality= 75;	// 
				}
				else
					JPEG_Quality= temp_uint8t;
				rtos_printf("	JPEG_Quality:	%d \n ",JPEG_Quality);
			}			
			if (strstr(buf, "UsbDrivePath")) {
				read_str_from_config_line(buf, config.llist_name, strlen("UsbDrivePath"));
				strcpy(UsbDrivePath,config.llist_name);
				rtos_printf("	UsbDrivePath:	%s \n ",UsbDrivePath);
			}			
			if (strstr(buf, "FTPResponseTimeout")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("FTPResponseTimeout"));
				if ((temp_uint16t < 20) || (temp_uint16t > 100))
				{
					FTPResponseTimeout= 40 * 10;	// 
				}
				else
					FTPResponseTimeout= temp_uint16t * 10;
				rtos_printf("	FTPResponseTimeout:	%d \n ",FTPResponseTimeout);
			}	
			if (strstr(buf, "Preserve_Last_JPEGImage_File")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("Preserve_Last_JPEGImage_File"));
				if ((temp_uint16t < 100) || (temp_uint16t > 1000))
				{
					Preserve_Last_JPEGImage_File= 100;	// 
				}
				else
					Preserve_Last_JPEGImage_File= temp_uint16t;
				rtos_printf("	Preserve_Last_JPEGImage_File:	%d \n ",Preserve_Last_JPEGImage_File);
			}
			if (strstr(buf, "ModemPartNumber")) {
				read_str_from_config_line(buf, config.llist_name, strlen("ModemPartNumber"));
				strcpy(ModemPartNumber,config.llist_name);
				rtos_printf("	ModemPartNumber:	%s \n ",ModemPartNumber);
						/* Validate Modem Part Number */
				if ((0 == strncmp(ModemPartNumber, "ME910C1-WW", strlen("ME910C1-WW"))) ||
					(0 == strncmp(ModemPartNumber, "LE910-NA1", strlen("LE910-NA1"))) ||
					(0 == strncmp(ModemPartNumber, "LE910C4-CN", strlen("LE910C4-CN"))))
				{
					rtos_printf("		Modem Part Number is Valid: %s \n", ModemPartNumber );
				}
				else
				{
					rtos_printf("		Modem Part Number is Invalid: %s \n", ModemPartNumber );
					//return -2;
				}
			}		
				
			if (strstr(buf, "ModemPidNumber")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("ModemPidNumber"));
				ModemPidNumber= temp_uint8t;
				rtos_printf("	ModemPidNumber:	%d \n ",ModemPidNumber);
				if (((0 == strncmp(ModemPartNumber, "ME910C1-WW", strlen("ME910C1-WW"))) && (1101 == ModemPidNumber)) ||
					((0 == strncmp(ModemPartNumber, "LE910-NA1", strlen("LE910-NA1"))) && (36 == ModemPidNumber)) ||
					((0 == strncmp(ModemPartNumber, "LE910C4-CN", strlen("LE910C4-CN"))) && (1201 == ModemPidNumber)))
				{
					rtos_printf("		Modem PID Number is Valid: %d \n", ModemPidNumber );

				}
				else
				{
					//rtos_printf("		Modem PID Number is Invalid: %d \n", ModemPidNumber );
					
					if (0 == strncmp(ModemPartNumber, "ME910C1-WW", strlen("ME910C1-WW")))
					{
						ModemPidNumber = 1101;
					}
					else if (0 == strncmp(ModemPartNumber, "LE910-NA1", strlen("LE910-NA1")))
					{
						ModemPidNumber = 36;
					}
					else if (0 == strncmp(ModemPartNumber, "LE910C4-CN", strlen("LE910C4-CN")))
					{
						ModemPidNumber = 1201;
					}
					else
					{
						rtos_printf("		Modem Part Number : %s or  Pid number: %d is Invalid.\n"
						,ModemPartNumber, ModemPidNumber );
						//qDebug("Modem PID Number is Invalid: %d", ModemPidNumber);
						//return -2;
					}
				}			
			}
//////////////////////////////////////////////////////////////////
			
//////////////////////////////////////////////////////////////////			
			if (strstr(buf, "ModemTtyPort")) {
				read_str_from_config_line(buf, config.llist_name, strlen("ModemTtyPort"));
				strcpy(ModemTtyPort,config.llist_name);
				rtos_printf("	ModemTtyPort:	%s \n ",ModemTtyPort);
				if ((0 == strncmp(ModemPartNumber, "ME910C1-WW", strlen("ME910C1-WW")))  && (1101 == ModemPidNumber))
				{
					strncpy(ModemTtyPort, "/dev/ttyUSB2", strlen("/dev/ttyUSB2"));
				}
				else if ((0 == strncmp(ModemPartNumber, "LE910-NA1", strlen("LE910-NA1"))) && (36 == ModemPidNumber))
				{
					strncpy(ModemTtyPort, "/dev/ttyACM0", strlen("/dev/ttyACM0"));
				}
				else if ((0 == strncmp(ModemPartNumber, "LE910C4-CN", strlen("LE910C4-CN"))) && (1201 == ModemPidNumber))
				{
					strncpy(ModemTtyPort, "/dev/ttyUSB2", strlen("/dev/ttyUSB2"));
				}
				else
				{
					rtos_printf("		Modem part number: %s or ModemPidNumber: %d is Invalid. \n", ModemPartNumber, ModemPidNumber );

				}				
			}			
		
			if (strstr(buf, "LastPacketDelay")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LastPacketDelay"));
				if ((temp_uint16t < 0) || (temp_uint16t > 10000))
				{
					LastPacketDelay= 2000;	// 
				}
				else
					LastPacketDelay= temp_uint16t;
				rtos_printf("	LastPacketDelay:	%d \n ",LastPacketDelay);
			}
			if (strstr(buf, "InterPacketDelay")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("InterPacketDelay"));
				if ((temp_uint16t < 0) || (temp_uint16t > 10000))
				{
					InterPacketDelay= 200;	// 
				}
				else
					InterPacketDelay= temp_uint16t;
				rtos_printf("	InterPacketDelay:	%d \n ",InterPacketDelay);
			}			
			if (strstr(buf, "PrintDebugMessage")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("PrintDebugMessage"));
				if ((temp_uint8t < 0) || (temp_uint8t > 5))
				{
					PrintDebugMessage= 1;	// 
				}
				else
					PrintDebugMessage= temp_uint8t;
				rtos_printf("	PrintDebugMessage:	%d \n ",PrintDebugMessage);
			}	
			if (strstr(buf, "SomLogEnable")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("SomLogEnable"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					SomLogEnable= 1;	// 
				}
				else
					SomLogEnable= temp_uint8t;
				rtos_printf("	SomLogEnable:	%d \n ",SomLogEnable);
			}	
			if (strstr(buf, "SendFtpStatus")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("SendFtpStatus"));
				if ((temp_uint8t < 0) || (temp_uint8t > 1))
				{
					SendFtpStatus= 0;	// 
				}
				else
					SendFtpStatus= temp_uint8t;
				rtos_printf("	SendFtpStatus:	%d \n ",SendFtpStatus);
			}

			if (strstr(buf, "LOGLEVEL_WAKE_SLEEP_ACTIVETIME")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LOGLEVEL_WAKE_SLEEP_ACTIVETIME"));
				if ((temp_uint16t < 0) || (temp_uint16t > 4))
				{
					LOGLEVEL_WAKE_SLEEP_ACTIVETIME= 1;	// 
				}
				else
					LOGLEVEL_WAKE_SLEEP_ACTIVETIME= temp_uint16t;
				rtos_printf("	LOGLEVEL_WAKE_SLEEP_ACTIVETIME:	%d \n ",LOGLEVEL_WAKE_SLEEP_ACTIVETIME);
			}
			if (strstr(buf, "LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR"));
				if ((temp_uint16t < 0) || (temp_uint16t > 4))
				{
					LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR= 1;	// 
				}
				else
					LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR= temp_uint16t;
				rtos_printf("	LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR:	%d \n ",LOGLEVEL_CAMERATRIGGER_IMAGECAPTURE_ANPR);
			}
			if (strstr(buf, "LOGLEVEL_COMMS")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LOGLEVEL_COMMS"));
				if ((temp_uint16t < 0) || (temp_uint16t > 4))
				{
					LOGLEVEL_COMMS= 1;	// 
				}
				else
					LOGLEVEL_COMMS= temp_uint16t;
				rtos_printf("	LOGLEVEL_COMMS:	%d \n ",LOGLEVEL_COMMS);
			}
			if (strstr(buf, "LOGLEVEL_FILESAVE_TRANSP_FTP")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LOGLEVEL_FILESAVE_TRANSP_FTP"));
				if ((temp_uint16t < 0) || (temp_uint16t > 4))
				{
					LOGLEVEL_FILESAVE_TRANSP_FTP= 1;	// 
				}
				else
					LOGLEVEL_FILESAVE_TRANSP_FTP= temp_uint16t;
				rtos_printf("	LOGLEVEL_FILESAVE_TRANSP_FTP:	%d \n ",LOGLEVEL_FILESAVE_TRANSP_FTP);
			}
			
			if (strstr(buf, "LogfilesizeLimit_in_KB")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("LogfilesizeLimit_in_KB"));
				if ((temp_uint16t < 50) || (temp_uint16t > 500))
				{
					LogfilesizeLimit_in_KB= 100;	// 
				}
				else
					LogfilesizeLimit_in_KB= temp_uint16t;
				rtos_printf("	LogfilesizeLimit_in_KB:	%d \n ",LogfilesizeLimit_in_KB);
			}	
			if (strstr(buf, "CenterRowStart")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("CenterRowStart"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1088))
				{
					CenterRowStart= 0;	// 
				}
				else
					CenterRowStart= temp_uint16t;
				rtos_printf("	CenterRowStart:	%d \n ",CenterRowStart);
			}	
			if (strstr(buf, "CenterColStart")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("CenterColStart"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1928))
				{
					CenterColStart= 0;	// 
				}
				else
					CenterColStart= temp_uint16t;
				rtos_printf("	CenterColStart:	%d \n ",CenterColStart);
			}			
			if (strstr(buf, "CenterWidth")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("CenterWidth"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1928))
				{
					CenterWidth= 1928;	// 
				}
				else
					CenterWidth= temp_uint16t;
				rtos_printf("	CenterWidth:	%d \n ",CenterWidth);
			}
			if (strstr(buf, "CenterHeight")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("CenterHeight"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1088))
				{
					CenterHeight= 1088;	// 
				}
				else
					CenterHeight= temp_uint16t;
				rtos_printf("	CenterHeight:	%d \n ",CenterHeight);
			}
		
			if (strstr(buf, "BrighterDataCountThreshold")) {
				temp_uint32t= read_int_from_config_line(buf, strlen("BrighterDataCountThreshold"));
				if ((temp_uint32t < 0) || (temp_uint32t > 1000000))
				{
					BrighterDataCountThreshold= 1000;	// 
				}
				else
					BrighterDataCountThreshold= temp_uint32t;
				rtos_printf("	BrighterDataCountThreshold:	%ld \n ",BrighterDataCountThreshold);
			}
			if (strstr(buf, "EnableBrighterDataImage")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("EnableBrighterDataImage"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1088))
				{
					EnableBrighterDataImage= 1088;	// 
				}
				else
					EnableBrighterDataImage= temp_uint16t;
				rtos_printf("	EnableBrighterDataImage:	%d \n ",EnableBrighterDataImage);
			}
			if (strstr(buf, "BrighterDataImageStartTime")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("BrighterDataImageStartTime"));
				if ((temp_uint8t < 0) || (temp_uint8t > 23))
				{
					BrighterDataImageStartTime= 17;	// 
				}
				else
					BrighterDataImageStartTime= temp_uint8t;
				rtos_printf("	BrighterDataImageStartTime:	%d \n ",BrighterDataImageStartTime);
			}
			if (strstr(buf, "BrighterDataImageEndTime")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("BrighterDataImageEndTime"));
				if ((temp_uint8t < 0) || (temp_uint8t > 23))
				{
					BrighterDataImageEndTime= 8;	// 
				}
				else
					BrighterDataImageEndTime= temp_uint8t;
				rtos_printf("	BrighterDataImageEndTime:	%d \n ",BrighterDataImageEndTime);
			}


			if (strstr(buf, "Col_Start")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Col_Start"));
				/*if ((temp_uint8t < 0) || (temp_uint8t > 23))
				{
					Col_Start= 8;	// 
				}
				else*/
					Col_Start= temp_uint8t;
				rtos_printf("	Col_Start:	%d \n ",Col_Start);
			}
			if (strstr(buf, "Gray_Cols")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("Gray_Cols"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1928))
				{
					Gray_Cols= 1928;	// 
				}
				else
					Gray_Cols= temp_uint16t;
				rtos_printf("	Gray_Cols:	%d \n ",Gray_Cols);
			}	
			if (strstr(buf, "Gray_Rows")) {
				temp_uint16t= read_int_from_config_line(buf, strlen("Gray_Rows"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1088))
				{
					Gray_Rows= 1088;	// 
				}
				else
					Gray_Rows= temp_uint16t;
				rtos_printf("	Gray_Rows:	%d \n ",Gray_Rows);
			}			
			if (strstr(buf, "Row_Start")) {
				temp_uint8t= read_int_from_config_line(buf, strlen("Row_Start"));
				/*if ((temp_uint8t < 0) || (temp_uint8t > 23))
				{
					Row_Start = 8;	// 
				}
				else*/
					Row_Start = temp_uint8t;
				rtos_printf("	Row_Start:	%d \n ",Row_Start );
			}	
			//if (strstr(buf, "Cols")) 
			if ((strncmp(buf, "Cols",strlen("Cols"))) == 0)	
			{
				temp_uint16t= read_int_from_config_line(buf, strlen("Cols"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1928))
				{
					Cols= 1928;	// 
				}
				else
					Cols= temp_uint16t;
				rtos_printf("	Cols:	%d \n ",Cols);
			}
			//if (strstr(buf, "Rows")) 
			if ((strncmp(buf, "Rows",strlen("Rows"))) == 0)	
			{
				temp_uint16t= read_int_from_config_line(buf, strlen("Rows"));
				if ((temp_uint16t < 0) || (temp_uint16t > 1088))
				{
					Rows= 1088;	// 
				}
				else
					Rows= temp_uint16t;
				rtos_printf("	Rows:	%d \n ",Rows);
			}
			////port_out(XS1_PORT_4C, 0x09);
			///////			

//////////////////////////////////////////////////////////////////////////////////////////			
//////////////////////////////////////////////////////////////////////////////////////////			
		}
		xmosConfigInitFinished  = 1;
		////port_out(XS1_PORT_4C, 0x00);
		rtos_printf("Number of lines in file: %d\n", no_of_lines);
		rtos_printf("File operation completed ");
		
#endif
	/*printf("TBPL = %d\n", config.bytes_per_line);
    printf("NUMIN = %f\nNUMAX = %f\n", config.numin, config.numax);
    printf("LLIST_NAME = %s\n", config.llist_name);
	*/
	////////////////////////////////////
	
	/*while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(5000));
		rtos_printf("Alive:%d\n ", i++);
	}*/
	f_close(&test_file);
#if 0	
//int main(void) 
{
/* ls -al | grep '^d' */
  FILE *pp;
  pp = popen("ls -al", "r");
  if (pp != NULL) {
    while (1) {
      char *line;
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);
      if (line == NULL) break;
      if (line[0] == 'd') printf("%s", line); /* line includes '\n' */
    }
    pclose(pp);
  }
  //return 0;
}	
#endif
	return;
}

void XmosConfig_create( void/*UBaseType_t priority*/ )
{
	printf("PS: XmosConfig_create\n");
/*	
    xTaskCreate((TaskFunction_t) XmosConfig_read,
                "XmosConfig_read",
                RTOS_THREAD_STACK_SIZE(XmosConfig_read),
                NULL,
                priority,
                NULL);*/
	XmosConfig_read();
    /*xTaskCreate((TaskFunction_t) XmosConfig_read,
                "XmosConfig_read",
                portTASK_STACK_DEPTH(XmosConfig_read),
                NULL,
                priority,
                NULL);*/	
				
	/*xTaskCreate((TaskFunction_t) capture_thread,
                "capture_thread",
                (portTASK_STACK_DEPTH(capture_thread)),
                c,
                appconfUART_RXD2_TASK_PRIORITY,
                NULL);	*/
}

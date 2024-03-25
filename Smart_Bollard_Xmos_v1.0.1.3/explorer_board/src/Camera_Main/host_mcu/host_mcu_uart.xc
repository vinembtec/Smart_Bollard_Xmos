// Copyright (c) 2020, XMOS Ltd, All rights reserved

/*** Include Files ***/

#include <syscall.h>
#include <xs1.h>
#include <platform.h>
#include <stdint.h>
#include <timer.h>
#include <xmos_flash.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "host_mcu_uart.h"
#include "host_mcu_interface.h"
#include "host_mcu_types.h"

#include "xmos_config/xmos_config.h"
/*** Macro Definitions ***/

#define XMOS_MSP_UART_BAUD_RATE 		115200


extern "C" {
	
	
	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	
}

on tile[1]:out port XMOS_TX_PIN = XS1_PORT_1A;	//X1D00 (X1D03->old)
on tile[1]:in port XMOS_RX_PIN = XS1_PORT_1C;	//X1D10 (X0D15->old)
char temptString[300];

/*** Type Definitions ***/

typedef enum 
{
    XMOS_MSP_UART_PORT
} UartPort_t;

typedef enum {
	HOST_MCU_NO_RESPONSE,
	HOST_MCU_VALID_RESPONSE,
	HOST_MCU_TILE1_BUSY
} HostMcuResponseStatus_t;


/*** Function Prototypes ***/

//extern void start_FreeRTOS_Task(void);
extern void InterTileCommTile1_sendResponse(chanend txCmd, unsigned char packetType, unsigned char status, unsigned char *data);
//extern void InterTileCommTile1_txInstance(chanend txInstance, unsigned char instance);
extern void InterTileCommTile1_txInstance(chanend txInstance, ImageInfo_t * unsafe imageCap, unsigned char instance);


/*** Variable Declarations ***/

unsigned char hostMcuRxReceived = 0;
unsigned char currentFileHeaderInfoTile1[205];
//QueueEntry_t QueueEntry;

extern unsigned char cameraImageCapture; //added on 16_06_2022
extern unsigned char imageCaptureTriggerType; //added on 05_07_2022
extern unsigned char hostMcuResponseReceived;

//extern VehicleOccupancyPacket_t vehicleOccupancyPacket;
extern unsigned char vehicleOccupancyPacketBuffer[MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH];
extern ImageInfo_t imageCapture;
extern ImageInfo_t * unsafe imgCap_Receive ;
extern ImageInfo_t * unsafe imgCap_txData ;

unsigned char FTP_Ready_Flag_From_MCU = 0;
unsigned char FTP_End_Flag_To_MCU = 0;
unsigned char FTP_Ready_Flag_Set_in_XMOS = 0;

extern int iCustomerId;
extern int iLeftMID;
extern int iRightMID;

int mspUartCustomerId = 0;
int mspUartbollardId = 0;
int mspUartAreaId = 0;

extern "C" {
	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	extern void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 );
	extern void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 );
	extern void Debug_Display( int8_t minimumDebugLevel, const char * pszText );
	//////////////////////////////////	
	extern uint8_t xmosConfigInitFinished;
}
/*** Function Definitions ***/

void HostMcuUart_initialize(void)
{
	
}

static void HostMcuUart_sendData(unsigned char uartPort, unsigned char txByte)
{
    unsigned char dataBit;
	int clocks = XS1_TIMER_HZ / XMOS_MSP_UART_BAUD_RATE;
	int ticks;
	
	switch (uartPort)
	{
		#if 0
		case XMOS_MSP_UART_PORT:
			//clocks = XS1_TIMER_HZ / XMOS_MSP_UART_BAUD_RATE;
			dataBit = txByte;
			XMOS_TX_PIN <: 0 @ ticks; //send start bit and timestamp (grab uartPort timer value)
			ticks += clocks;
			#pragma loop unroll(8)
			for (int i = 0; i < 8; i++) 
			{
				XMOS_TX_PIN @ ticks <: >> dataBit; //timed output with post right shift
				ticks += clocks;
			}
			XMOS_TX_PIN @ ticks <: 1; //send stop bit
			ticks += clocks;
			XMOS_TX_PIN @ ticks <: 1; //wait until end of stop bit	
			break;
			#endif
		case XMOS_MSP_UART_PORT:			
#if 1
	  //output_gpio_if p_txd = TXD;
	  //p_txd.output(1);
	  timer tmr;
      int t;
	  XMOS_TX_PIN <: 1;
	  tmr :> t;
      t += clocks * 5;
	  tmr when timerafter(t) :> void;
	  XMOS_TX_PIN <: 1;
	  
	  // Output start bit
      //p_txd.output(0);
	  XMOS_TX_PIN <: 0;
      tmr :> t;
      t += clocks;
      unsigned byte = txByte;
      // Output data bits
	  //#pragma loop unroll(8)
      for (int j = 0; j < 8/*bits_per_byte*/; j++) {
        tmr when timerafter(t) :> void;
        //p_txd.output(byte & 1);
		XMOS_TX_PIN <: (byte & 1);
        byte >>= 1;
        t += clocks;
      }
      // Output parity
      /*if (parity != UART_PARITY_NONE) {
        tmr when timerafter(t) :> void;
        p_txd.output(parity32(data, parity));
        t += bit_time;
      }*/
      // Output stop bits
      tmr when timerafter(t) :> void;
      //p_txd.output(1);
	  XMOS_TX_PIN <: 1;
      t += clocks * 1;
      tmr when timerafter(t) :> void;
	  XMOS_TX_PIN <: 1;
#endif			
			break;			
			
		default:
			break;
	}
}

unsigned char firstByteRcvd = 0;

void HostMcuUart_ReceiveData(chanend c_decoupler, chanend interTileInstance, chanend txd_rxd_channel,chanend seq_c)
//void HostMcuUart_ReceiveData(chanend c_decoupler, chanend rxRcvd)
{
	//channel_t imageCaptureTrigger;
	int clocks = XS1_TIMER_HZ / XMOS_MSP_UART_BAUD_RATE;
    int dt2 = (clocks * 3) >> 1; //one and a half bit times
    int dt = clocks;
    int ticks;
    unsigned int rxByte = 0;
	
	//chan_alloc(&imageCaptureTrigger);	
	HostMcuInterface_initialize();	
	
	//sprintf(temptString, "\r\nHostMcuUart_ReceiveData.");
			                //Debug_TextOut(0, temptString);	
			                
 
	
	unsafe {
		uint16_t * unsafe iCustomerId_temp = &iCustomerId;	
		uint16_t * unsafe iLeftMID_temp = &iLeftMID;
		uint16_t * unsafe iRightMID_temp = &iRightMID;		
		while (1) 
		{
			/////////THIS STEP ADDED TO WAIT FOR XMOS TO GET READY - NOT REQUIRED/ NECESSARY////////////
			if(xmosConfigInitFinished == 0)
				{
				for(uint8_t configCOunt = 0; configCOunt < 100;configCOunt++)
					{
						if(xmosConfigInitFinished == 1) break;
						delay_milliseconds(1);
					}
					xmosConfigInitFinished = 1;						
				}			
			////////////////////
			
			//TODO:: monitor Frame Receive Timeout ...if timer timedout reset receive parameters
			HostMcuInterface_decrementFrameReceiveTimeoutTimer();

			(XMOS_RX_PIN) when pinseq(0) :> int _ @ ticks; //wait until falling edge of start bit
			ticks += dt2;
			
			#pragma loop unroll(8)
			for (int i = 0; i < 8; i++) 
			{
				(XMOS_RX_PIN) @ ticks :> >> rxByte; //sample value when port timer = t //includes post right shift            					
				ticks += dt;
			}

			rxByte >>= 24;			//shift into MSB
			//cOut <: (unsigned char) rxByte; //send to client
			(XMOS_RX_PIN) @ ticks :> int _;
			
			
			
			
			/* Load Received Data in Buffer */	
			HostMcuInterface_handleUartReceiveData((unsigned char)rxByte);	
				                
		
			//sprintf(temptString, " Testing %02X ", (unsigned char)rxByte);
			
			 //Debug_TextOut(0, temptString);
	  	
			rxByte = 0;	
			
			if (1 == hostMcuResponseReceived)
			{
				delay_milliseconds((int)2);
				hostMcuResponseReceived = 0;
				//outuchar(rxRcvd, 1);
				//sprintf(temptString, "\r\nhostMcuResponseReceived.");
			                //Debug_TextOut(0, temptString);	
				HostMcuInterface_parseReceivedPackets(); 
				//delay_milliseconds((int)5);
				//outuchar(seq_c, 8);
			}

			/* Image Capture Trigger */
			if ((1 == cameraImageCapture) || (2 == cameraImageCapture))
			{			
				//sprintf(temptString, "\r\nImage Capture Trigger.");
			                //Debug_TextOut(0, temptString);	
				//if (HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE == cameraImageCapture)
				if ((IMAGE_TRIGGER_CAPTURE_MULTIPLE_IMAGES == imageCaptureTriggerType) || (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imageCaptureTriggerType))	
				{
					if (1 == cameraImageCapture)
					{
						*iCustomerId_temp = mspUartCustomerId;
						*iLeftMID_temp = mspUartbollardId;
					}
					else if (2 == cameraImageCapture)				
					{
						*iCustomerId_temp = mspUartCustomerId;
						*iRightMID_temp = (mspUartbollardId+1);						
					}
					
					if(xmosConfigInitFinished == 0)
					{
						for(uint8_t configCOunt = 0; configCOunt < 100;configCOunt++)
						{
							if(xmosConfigInitFinished == 1) break;
							delay_milliseconds(1);
						}			
						xmosConfigInitFinished = 1;							
					}
					/*printstrln("\nTrigger MipiImager/MipiDecoupler to Capture Images , ");
					printstrln("imageCaptureTriggerType=");
					printint(imageCaptureTriggerType);
					printstrln(",");
					printstrln("cameraImageCapture=");
					printint(cameraImageCapture);
					printstrln("\n");*/
					/* Trigger MipiImager/MipiDecoupler to Capture Images */
					outuchar(c_decoupler, 1);
					//outuchar(seq_c, 1);
					outuchar(c_decoupler, cameraImageCapture);
					outuchar(c_decoupler, imageCaptureTriggerType);
					
					/* Report New Sensor Trigger to TILE[0] */
					//InterTileCommTile1_txInstance(interTileInstance, imgCap_Receive, SENSOR_TRIGGER_INSTANCE);
					outuchar(txd_rxd_channel, SENSOR_TRIGGER_INSTANCE);
				}
				else if (IMAGE_TRIGGER_MODEM_FREE_START_FTP == imageCaptureTriggerType)
				{
					/*printstrln("\nReport Modem Free Start FTP to TILE[0] , ");
					printstrln("imageCaptureTriggerType=");
					printint(imageCaptureTriggerType);
					printstrln(",");
					printstrln("cameraImageCapture=");
					printint(cameraImageCapture);
					printstrln("\n");*/
					/* Report Modem Free Start FTP to TILE[0] */
					//InterTileCommTile1_txInstance(interTileInstance, imgCap_Receive, MODEM_FREE_START_FTP_INSTANCE);
					outuchar(txd_rxd_channel, MODEM_FREE_START_FTP_INSTANCE);
				}
				
				cameraImageCapture = 0;
				imageCaptureTriggerType = 0;
				
				//return; //TBD //added on 17_06_2022
			}		
		}
	}
}

void HostMcuUart_sendBuffer(unsigned char* buffer, unsigned short length)
{
	for (unsigned short count = 0; count < length; count++)
    {
        HostMcuUart_sendData(XMOS_MSP_UART_PORT, buffer[count]);
    }	
}

//void HostMcuUart_txData(chanend txCmd, chanend rxRcvd)
void HostMcuUart_txData(chanend tile0TxCmd, chanend tile1TxCmd, chanend interTileInstance, chanend JPEG_Status_Send, chanend JPEGinterTileInstance, chanend txd_rxd_channel, chanend interTileInstance_new)
{	
	unsigned char commandSend = 0;
	unsigned char txCommand;	
	unsigned char txTaskExecute = 0;
	char byteString[50];
	
	////printf("\r\nHostMcuUart_txData\n");	
	//start_FreeRTOS_Task();
	
	unsafe {
		while (1)
		{		
			select {
				case inuchar_byref(tile0TxCmd, txCommand):
					////printstr("\r\nT[1]RX: Inter Tile(tile0TxCmd).");
					txTaskExecute = 1;
					break;
				case inuchar_byref(tile1TxCmd, txCommand):	
					////printstr("\r\nT[1]RX: Intra Tile(tile1TxCmd).");
					txTaskExecute = 1;
					break;
				case inuchar_byref(JPEGinterTileInstance, txCommand):	
					////printstr("\r\nT[1]RX: Jpeg Complete Inter Tile. File Transfer Start.");
					txTaskExecute = 1;
					break;	
				case inuchar_byref(JPEG_Status_Send, txCommand):	
					////printstr("\r\nT[1]RX: Jpeg Start Intra Tile.");
					txTaskExecute = 2;
					break;
				case inuchar_byref(txd_rxd_channel, txCommand):	
					////printstr("\r\nT[1]RX: txd_rxd_channel Start Intra Tile.");
					txTaskExecute = 2;
					break;
				case inuchar_byref(interTileInstance_new, txCommand):
					////printstr("\r\nT[1]RX: Inter Tile(interTileInstance_new).");
					txTaskExecute = 1;
					break;					
				/*case inuchar_byref(rxRcvd, hostMcuRxReceived):				
					if (1 == hostMcuRxReceived)
					{
						printf("\r\nrxReceived: %d\n", hostMcuRxReceived);
					}
					break;*/
			}
			
			if (1 == txTaskExecute)
			{
				txTaskExecute = 0;
				
				//printf("\r\nTILE[1]: RX: %d\n", txCommand);				
				/*printstr("__TILE[1]: RX: ");
				printint(txCommand);
				printstrln(" ");*/
				if ((txCommand > HOST_MCU_VEHICLE_RTC_INFO_MESSAGE) && (txCommand <= HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE))
				{
					if (HOST_MCU_IMAGE_FILE_HEADER_MESSAGE == txCommand)
					{
						for (int count = 0; count < sizeof(currentFileHeaderInfoTile1); count++)
						{
							currentFileHeaderInfoTile1[count] = inuchar(interTileInstance_new);
						}
						unsigned long imageSize = (currentFileHeaderInfoTile1[201] << 24) | (currentFileHeaderInfoTile1[202] << 16) |
													(currentFileHeaderInfoTile1[203] << 8) | currentFileHeaderInfoTile1[204];
					//	char tempPrintString[300];							
					//	sprintf(tempPrintString, "\r\nT[1]RX: File Header Info: %s %d", &currentFileHeaderInfoTile1[1], imageSize);
					//	printstr(tempPrintString);
					//	Debug_TextOut(0,tempPrintString);
					}
					HostMcuInterface_txTaskHandler(txCommand);					
					commandSend = 1;
					
					/* Report Image Capture Complete to TILE[0] */
					if (HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE == txCommand)
					{
						FTP_Ready_Flag_From_MCU = 0;
						InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, IMAGE_CAPTURE_COMPLETE_INSTANCE);				
					}
					else if (HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE == txCommand)
					{
						FTP_Ready_Flag_From_MCU = 0;
						////printstr("__TILE[1]1: RX: HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE ");
						//InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);				
					}
					else if (HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE == txCommand)
					{
						////HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
						//FTP_Ready_Flag_From_MCU = 0;
						//InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, CAMERA_POWER_OFF_INSTANCE);				
					}
					else if (HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE == txCommand)
					{
						////HostMcuInterface_sendDataPacket(HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE);
						//FTP_Ready_Flag_From_MCU = 0;
						//InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, CAMERA_POWER_ON_INSTANCE);				
					}
				}
			}
			
			if (2 == txTaskExecute)
			{
				txTaskExecute = 0;
				
				//printf("\r\nTILE[1]: RX: %d\n", txCommand);				
				/*printstr("__TILE[1]: RX: ");
				printint(txCommand);
				printstrln(" ");*/
				if(SENSOR_TRIGGER_INSTANCE == txCommand)
				{
					InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, SENSOR_TRIGGER_INSTANCE);						
				}
				 if(MODEM_FREE_START_FTP_INSTANCE == txCommand)
				{
					//FTP_Ready_Flag_From_MCU = 1;
					//FTP_End_Flag_To_MCU = 0;
					InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, MODEM_FREE_START_FTP_INSTANCE);						
				}
				 if(JPEG_CREATION_START_INSTANCE == txCommand)
				{
					InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, JPEG_CREATION_START_INSTANCE);						
				}	
				 if(JPEG_CREATION_COMPLETE_INSTANCE == txCommand)
				{
					InterTileCommTile1_txInstance(interTileInstance, imgCap_txData, JPEG_CREATION_COMPLETE_INSTANCE);							
				}
				//commandSend = 1;	
				txCommand = 0;				
			}
			
			if (1 == commandSend)
			{
				commandSend = 0;
				
				if ((txCommand > HOST_MCU_VEHICLE_RTC_INFO_MESSAGE) && (txCommand <= HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE))
				{
					////printstr("\r\nstep1");
					
					if((HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE == txCommand)||
					  (HOST_MCU_IMAGE_FILE_HEADER_MESSAGE == txCommand) /*||
					  (HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE == txCommand) ||
					  (HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE == txCommand)*/) 
					{
						FTP_End_Flag_To_MCU = 1 ;
					}
					
					if((FTP_Ready_Flag_From_MCU == 1)||(FTP_End_Flag_To_MCU == 1)) 
					{
						////printstr("\r\nstep2");
						FTP_Ready_Flag_Set_in_XMOS = 1;
					}
					unsigned char status = HostMcuInterface_waitForResponse();				
					////printstr("\r\nstep3");
					if (1 == status)	/* Retry */
					{
						////printstr("\r\nT[1](1 == status).");
						//outuchar(txCmd, txCommand);
						////if(FTP_Ready_Flag_From_MCU == 0)
						if(HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE == txCommand)
						{
							InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_NO_RESPONSE,vehicleOccupancyPacketBuffer);
							//outuchar(tile1TxCmd, txCommand);
						}
						else if(HOST_MCU_IMAGE_FILE_HEADER_MESSAGE == txCommand)
						{
							InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_NO_RESPONSE,vehicleOccupancyPacketBuffer);
							//outuchar(tile1TxCmd, txCommand);
						}
						else
						{
							////printstr("\r\nT[1](1 == status). else");
							InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_NO_RESPONSE,vehicleOccupancyPacketBuffer);
							////outuchar(JPEGinterTileInstance, HOST_MCU_NO_RESPONSE);
						}
					}
					else if (status > 1)
					{
						commandSend = 0;
						
						/* Response to Tile 0 */ 
						if (HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE == txCommand)
						{
							//printstr("\r\nResponse to Tile 0,0");
							if (2 == status)
							{
								//TODO:: add here intertile response for HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE
								////printf("\r\n(1 == commandSend)(2 == status) = %d",txCommand);
								//InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_VALID_RESPONSE, QueueEntry.data);
								//InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_VALID_RESPONSE, vehicleOccupancyPacket.buffer);
								InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_VALID_RESPONSE, vehicleOccupancyPacketBuffer);
							}
							else if (3 == status)
							{
								////printf("\r\n(1 == commandSend)(3 == status) = %d",txCommand);
								//InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_NO_RESPONSE, QueueEntry.data);
								//InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_NO_RESPONSE, vehicleOccupancyPacket.buffer);
								InterTileCommTile1_sendResponse(JPEGinterTileInstance, txCommand, HOST_MCU_NO_RESPONSE, vehicleOccupancyPacketBuffer);
							}
						}
						else if((HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE == txCommand)||
								(HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE == txCommand)||
								(HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE == txCommand))
						{
							////printstr("\r\nResponse to Tile 0,1");
							FTP_End_Flag_To_MCU = 0;
							if (2 == status)
							{ 
								////printstr("\r\nResponse to Tile 0 (2 == status)1");
								//printf("\r\n(1 == commandSend)(2 == status(%d))1 = %d",status,txCommand);
								InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_VALID_RESPONSE,vehicleOccupancyPacketBuffer);
							}
							else if (3 == status)
							{
								////printstr("\r\nResponse to Tile 0 (3 == status)1");
								//printf("\r\n(1 == commandSend)(3 == status(%d))1 = %d",status,txCommand);
								InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_NO_RESPONSE,vehicleOccupancyPacketBuffer);
							}							
						}
						else if(HOST_MCU_IMAGE_FILE_HEADER_MESSAGE == txCommand)
						{
							////printstr("\r\nResponse to Tile 0,2");
							FTP_End_Flag_To_MCU = 0;
							if (2 == status)
							{ 
								////printstr("\r\nResponse to Tile 0 (2 == status)2");
								//printf("\r\n(1 == commandSend)(2 == status(%d))1 = %d",status,txCommand);
								InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_VALID_RESPONSE,vehicleOccupancyPacketBuffer);
							}
							else if (3 == status)
							{
								////printstr("\r\nResponse to Tile 0 (3 == status)2");
								//printf("\r\n(1 == commandSend)(3 == status(%d))1 = %d",status,txCommand);
								InterTileCommTile1_sendResponse(interTileInstance_new, txCommand, HOST_MCU_NO_RESPONSE,vehicleOccupancyPacketBuffer);
							}							
						}
						else
						{
							////printstr("\r\nNo Response to Tile 0,3");
							//printf("\r\n(1 == commandSend)(2 == status(%d)2) = %d",status,txCommand);
						}
						////printstr("\r\nEND OF HostMcuUart_txData 1111111111");
						txCommand = 0;
					}
				}
				else
				{
					////printstr("\r\nEND OF HostMcuUart_txData 2222222222");
					commandSend = 0;
					txCommand = 0;
				}
				////printstr("\r\nEND OF HostMcuUart_txData 333333333333");
			}
			////printstr("\r\nEND OF HostMcuUart_txData 444444444444");
		}
	}
}

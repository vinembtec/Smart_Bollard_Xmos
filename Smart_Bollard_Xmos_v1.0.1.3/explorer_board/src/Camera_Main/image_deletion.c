
/*** Include Files ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rtos_osal.h"
#include "image_deletion.h"
#include "record_queue.h"
#include "host_mcu/protocol_data_host_mcu.h"
#include "host_mcu/host_mcu_utils.h"


/*** Macro Definitions ***/

#define MAX_SIZE            200
#define MAX_BATCHED_FILES   20//100


/*** Type Definitions ***/

typedef union {
	struct {
		VehicleOccupancyData_t data;
	};
	unsigned char buffer[MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH];
} VehicleOccupancyPkt_t;


/*** Function Prototypes ***/

int ImageDeletion_processVehicleOccupancyStatusInfo(uint8_t *buffer, uint16_t length);
//int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(void);
extern int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(chanend_t txCmd);
extern void InterTileCommTile0_sendPacketToHostMcuViaTile1(chanend_t txCmd, unsigned char packetType);
extern void GetImageFilesInDirToArray_For_FTP_Transfer(uint8_t instance);


/*** Variable Declarations ***/

//extern VehicleOccupancyPacket_t vehicleOccupancyPacketTile0;
//VehicleOccupancyPacket_t vehicleOccupancyPacketTile0;
VehicleOccupancyPkt_t vehicleOccupancyPacketTile0;

//extern int iPrintDebugMessage;
extern int iAnprEnable;
extern int iCustomerId;
extern int iLeftMID;
extern int iRightMID;
extern uint16_t Number_of_FL_BatchedImages;
extern char FTP_FL_Filename[MAX_BATCHED_FILES][MAX_SIZE];


/*** Function Definitions ***/

//int ImageDeletion_processVehicleOccupancyStatusInfo(uint8_t *buffer, uint16_t length)
int ImageDeletion_processVehicleOccupancyStatusInfo(unsigned char *buffer, unsigned short length)
{
	/* Copy Received Data to Vehicle Occupancy Packet Buffer */
	memcpy(vehicleOccupancyPacketTile0.buffer, buffer, length);

	printf("\r\n	Processing Vehicle Occupancy Status Info Response..");
		
	/* Validate Total Number Of Vehicle Occupancy Status Timestamp Records */
	printf("\r\n	TotalNumberOfRecords: %d", vehicleOccupancyPacketTile0.data.totalNumberOfRecords); 
	
	/* Sanity Check */
	if (vehicleOccupancyPacketTile0.data.totalNumberOfRecords > MAX_VEHICLE_OCCUPANCY_RECORDS)
	{
		vehicleOccupancyPacketTile0.data.totalNumberOfRecords = MAX_VEHICLE_OCCUPANCY_RECORDS;
	}

	/* Add No Stable State Records to Queue */
	for (uint16_t count = 0; count < vehicleOccupancyPacketTile0.data.totalNumberOfRecords; count++)
	{
		/* Swap Bytes */ 
		vehicleOccupancyPacketTile0.data.record[count].meterId = ((vehicleOccupancyPacketTile0.data.record[count].meterId & 0x00ff) << 8) | ((vehicleOccupancyPacketTile0.data.record[count].meterId & 0xff00) >> 8);
		vehicleOccupancyPacketTile0.data.record[count].timeStamp.year = ((vehicleOccupancyPacketTile0.data.record[count].timeStamp.year & 0x00ff) << 8) | ((vehicleOccupancyPacketTile0.data.record[count].timeStamp.year & 0xff00) >> 8);

		//if (iPrintDebugMessage > 2)
		{
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].status: %d", vehicleOccupancyPacketTile0.data.record[count].status);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].meterId: %d", vehicleOccupancyPacketTile0.data.record[count].meterId);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.year: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.year);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.month: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.month);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.date: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.date);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.hour: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.hour);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.minute: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.minute);
			printf("\r\n	vehicleOccupancyPacketTile0.data.record[count].timeStamp.second: %d", vehicleOccupancyPacketTile0.data.record[count].timeStamp.second);
		}
		
		if ((VEHICLE_OCCUPANCY_NO_STABLE_STATE == vehicleOccupancyPacketTile0.data.record[count].status) &&
			((iLeftMID == vehicleOccupancyPacketTile0.data.record[count].meterId) || (iRightMID == vehicleOccupancyPacketTile0.data.record[count].meterId)))
		{	
			//if (iPrintDebugMessage > 1)
			{
				printf("\r\n	vehicleOccupancyPacketTile0.data.record(%d):", (count + 1));
				HostMcuUtils_printByteArrayHexString((uint8_t *)vehicleOccupancyPacketTile0.data.record[count].buffer, (uint16_t)VEHICLE_OCCUPANCY_RECORD_SIZE);
			}
			
			/* Validate Timestamp */
			if(!((vehicleOccupancyPacketTile0.data.record[count].timeStamp.second < 60) && (vehicleOccupancyPacketTile0.data.record[count].timeStamp.minute < 60) &&
				   (vehicleOccupancyPacketTile0.data.record[count].timeStamp.hour < 24) && (vehicleOccupancyPacketTile0.data.record[count].timeStamp.date < 32) &&
					(vehicleOccupancyPacketTile0.data.record[count].timeStamp.month < 13) && (vehicleOccupancyPacketTile0.data.record[count].timeStamp.year < 9999)))
			{
				printf("\r\n	Invalid Timestamp(%d)", (count + 1));
			}
			else
			{	
				QueueEntry_t newRecord;
				
				memcpy(&newRecord, vehicleOccupancyPacketTile0.data.record[count].buffer, sizeof(RecordInfo_t));
				
				//if (iPrintDebugMessage > 2)
				{
					printf("\r\n	newRecord.data(%d):", (count + 1));
					HostMcuUtils_printByteArrayHexString((uint8_t *)newRecord.data, (uint16_t)VEHICLE_OCCUPANCY_RECORD_SIZE);
				}

				uint8_t status = RecordQueue_addEntry(&newRecord);
				
				if (STATUS_BUSY == status)
				{
					printf("\r\n	Record Queue Full %d", (count +1));
					break;
				}
				else if (STATUS_ERROR == status)
				{
					printf("\r\n	Add Queue Error %d %d", status, (count +1));
				}
				else if (STATUS_RECORD_EXITS_IN_OLD_ENTRY == status)
				{
					printf("\r\n	Record Exists In Old Entry %d", (count +1));
				}
				else if (STATUS_OK == status)
				{					
					//if (iPrintDebugMessage >= 1)
					{					
						printf("\r\n	NO Stable State Record Add Queue Success %d", (count +1));
					}					
				}
			}
		}
	}
	
	printf("\r\n	Processing Vehicle Occupancy Status Info Response Completed.\r\n\r\n");	
	return 0;
}

//TODO:: Call Function, when Image Upload Successful
int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(chanend_t txCmd)
{
	QueueEntry_t *readRecord;
	
	uint8_t recordQueueNullCount = 0;
	uint16_t executeLoop = 0;
	uint16_t numEntries = 0;
	
	printf("\r\n\r\n	Delete Images Session (Based on False Trigger)");

	do 
	{		
		executeLoop++;
		
		/* Check Existing Record In Queue */
		numEntries = RecordQueue_getNumEntries();
		
		if (numEntries > RECORD_QUEUE_SIZE)
		{
			numEntries = RECORD_QUEUE_SIZE;
		}

		printf("\r\n	Record Queue numEntries: %d", numEntries);
			
		/* NO Existing Records in Queue. Get Vehicle Occupancy Status Info */
		if ((0 == numEntries) || (3 == executeLoop))
		{
			recordQueueNullCount++;			
			
			//if (iPrintDebugMessage > 1)
			{
				printf("\r\n	executeLoop: %d", executeLoop);
			}			
			
			if (0 == numEntries)
			{
				printf("\r\n	NO Valid Records in Queue.\r\n\r\n");
			}
			
			if (recordQueueNullCount >= 2)
			{
				//if (iPrintDebugMessage > 1)
				{
					printf("\r\n	Exit Loop: recordQueueNullCount(%d)", recordQueueNullCount);
				}			
				break;
			}
			
			/* Get Vehicle Occupancy Status Info */
			/* InterTile task to report to TILE[1]..to Send VehicleOccupancyStatusInfoRequest to MSP */
			InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE);
		}
		/* Found the Valid Records in Queue. Delete Images Based on the Vehicle Occupancy Status Info */
		else
		{		
			for (uint16_t entry = 0; entry < numEntries; entry++)
			{
				/* Get Record from Queue */
				readRecord = RecordQueue_getEntry();

				//if (iPrintDebugMessage > 1)
				{
					printf("\r\n	entry: %d", (entry + 1));
				}	
				
				if (NULL == readRecord)
				{
					break;
				}
			
				RecordInfo_t currentRecord;				
				memcpy(currentRecord.buffer, readRecord, sizeof(RecordInfo_t));
				
				if (VEHICLE_OCCUPANCY_NO_STABLE_STATE == currentRecord.status)
				{			
					char noStableStateImage[200];
					unsigned char fileNameMatched = 0;
					
					/* Find Image Name Match */
					for (uint16_t value = 0; value < 4; value++)	
					{
						fileNameMatched = 0;				
						
						/* Create Image Name */
						sprintf(noStableStateImage,"%04d_%05d_%04d%02d%02d_%02d%02d%02d", iCustomerId, currentRecord.meterId, (uint16_t)currentRecord.timeStamp.year,
										 (uint8_t)currentRecord.timeStamp.month, (uint8_t)currentRecord.timeStamp.date,
										 currentRecord.timeStamp.hour, currentRecord.timeStamp.minute, currentRecord.timeStamp.second);
						
						//if (iPrintDebugMessage >= 1)
						{
							printf("\r\n	noStableStateImage: %s", noStableStateImage);
						}
						
						/* Compare the Name with File Name List */
						for (uint16_t image = 0; image < Number_of_FL_BatchedImages; image++)
						{
							//TODO:: compare here with Image File Name in Name List...
							if (0 == strncmp(noStableStateImage, &FTP_FL_Filename[image][strlen("IMG_FL_")], strlen(noStableStateImage)))
							{
								printf("\r\n	File Name Match Found: %s", noStableStateImage);
								fileNameMatched = 1;
								break;
							}
						}
						
						if (fileNameMatched)
						{
							break;
						}
					
						/* Add Tolerance Value Based Number of Images to be Captured */
						if ((currentRecord.timeStamp.second >= 0) && (currentRecord.timeStamp.second < 59))
						{
							currentRecord.timeStamp.second += 1;
						}
						else if (59 == currentRecord.timeStamp.second) 
						{
							currentRecord.timeStamp.second = 0;
							
							if ((currentRecord.timeStamp.minute >= 0) && (currentRecord.timeStamp.minute < 59))
							{
								currentRecord.timeStamp.minute += 1;
							}
							else if (59 == currentRecord.timeStamp.minute)
							{
								currentRecord.timeStamp.minute = 0;
								
								if ((currentRecord.timeStamp.hour >= 0) && (currentRecord.timeStamp.hour < 23))
								{
									currentRecord.timeStamp.hour += 1;
								}
								else if (23 == currentRecord.timeStamp.hour)
								{
									currentRecord.timeStamp.hour = 0;
									
									uint8_t maxDate = 31;
																		
									//JAN Mar May July August October December
									if ((1 == currentRecord.timeStamp.month) || (3 == currentRecord.timeStamp.month) || (5 == currentRecord.timeStamp.month) ||
										(7 == currentRecord.timeStamp.month) || (8 == currentRecord.timeStamp.month) || (10 == currentRecord.timeStamp.month) ||
										(12 == currentRecord.timeStamp.month))
									{
										maxDate = 31;
									}
									//FEB
									else if (2 == currentRecord.timeStamp.month)
									{
										if (0 == (currentRecord.timeStamp.year % 4))
										{
											maxDate = 29;
										}
										else
										{
											maxDate = 28;
										}
									}
									//April, June, September, November
									else if ((4 == currentRecord.timeStamp.month) || (6 == currentRecord.timeStamp.month) || (9 == currentRecord.timeStamp.month) ||
										(11 == currentRecord.timeStamp.month))
									{
										maxDate = 30;
									}
									
									if ((currentRecord.timeStamp.date >= 1) && (currentRecord.timeStamp.date < maxDate))
									{
										currentRecord.timeStamp.date += 1;
									}
									else if (maxDate == currentRecord.timeStamp.date)
									{
										currentRecord.timeStamp.date = 1;
										
										if ((currentRecord.timeStamp.month >= 1) && (currentRecord.timeStamp.month < 12))
										{
											currentRecord.timeStamp.month += 1;
										}
										else if (12 == currentRecord.timeStamp.month)
										{
											currentRecord.timeStamp.month = 1;
											
											if ((currentRecord.timeStamp.year >= 0) && (currentRecord.timeStamp.year < 9999))
											{
												currentRecord.timeStamp.year += 1;
											}
											else if (9999 == currentRecord.timeStamp.year)
											{
												currentRecord.timeStamp.year = 0;																					
											}
										}
									}
								}
							}
						}										
					}
					
					/* Delete Images with Matched Name */
					if (fileNameMatched)
					{
						//char command[300];					
						
						for (uint16_t image = 0; image < 2; image++)
						{
							//TODO:: delete Files with noStableStateImage from SPI Flash File System..
							if (0 == image)
							{			
								//TODO:: change the File Path here...
								//sprintf(command, "rm /home/root/build-AutoParking-arm_esomimx7-Debug/IMG_FL_%s_*", noStableStateImage);
							}
							else if (1 == image) 
							{
								if (1 == iAnprEnable)
								{
									//TODO:: change the File Path here...
									//sprintf(command, "rm /home/root/build-AutoParking-arm_esomimx7-Debug/IMG_NP_%s_*", noStableStateImage);
								}
								else 
								{
									break;
								}								
							}
							
							//if (iPrintDebugMessage >= 1)
							{
								//printf("\r\n	command: %s", command);
							}
							
							//int iRet = system(command);
							int iRet = 0;
							//TODO:: delete Files with noStableStateImage..		
							//TODO:: add here File Delete ...
						
							if (0 == iRet)
							{
								printf("\r\n	Deleted All IMG Files Success. Start Name: %s", noStableStateImage);
								
								/* Delete Record in Queue */
								RecordQueue_deleteEntry(readRecord);
								
								/* Re-Arrange File List */
								GetImageFilesInDirToArray_For_FTP_Transfer(1);
							}
							else
							{
								printf("\r\n	Failed to Delete All IMG Files. Start Name: %s", noStableStateImage);
							}
						}
					}
					else
					{
						printf("\r\n	File Name %s Match NOT Found. Delete Record Queue Entry.", noStableStateImage);						
						/* Delete Record in Queue */
						RecordQueue_deleteEntry(readRecord);
					}
				}
			}
		}	
		
		if (executeLoop >= 4)
		{
			//if (iPrintDebugMessage > 1)
			{
				printf("\r\n	Exit Loop: executeLoop(%d)", executeLoop);
			}			
			break;
		}		
	} while ((recordQueueNullCount < 2) || (executeLoop < 4));

	//if (iPrintDebugMessage > 1)
	{
		printf("\r\n	executeLoop: %d", executeLoop);
		printf("\r\n	recordQueueNullCount: %d", recordQueueNullCount);
	}			
			
	printf("\r\n	Delete Images Session Completed.\r\n\r\n");		
	return 0;
}

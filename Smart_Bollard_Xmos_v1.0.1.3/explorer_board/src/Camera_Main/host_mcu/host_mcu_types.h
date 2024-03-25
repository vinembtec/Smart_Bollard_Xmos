
#ifndef HOST_MCU_TYPES_H_
#define HOST_MCU_TYPES_H_

#include <stdint.h>
//#include "protocol_data_host_mcu.h"

#define RECORD_QUEUE_SIZE    100

#define NUM_VEHICLE_OCCUPANCY_RECORDS  10

//#define HOST_MCU_RESPONSE_WAIT_TIME_INTERVAL_IN_MS	500 ///< 500ms

#define DEFAULT_AREA_ID      1
#define DEFAULT_CUSTOMER_ID  8042
#define DEFAULT_BOLLARD_ID   32091

#define MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH  101

/*#define TIMESTAMP_INFO_PACKET_LENGTH   7

#define MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH  101
#define MAX_VEHICLE_OCCUPANCY_RECORDS  		 10
#define VEHICLE_OCCUPANCY_RECORD_SIZE        10*/


typedef enum {
	HOST_MCU_IDLE_TYPE,
    HOST_MCU_VEHICLE_RTC_INFO_MESSAGE,
	HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE,
	HOST_MCU_ANPR_INFO_MESSAGE,
    HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE,
    HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE,
	HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE,
	HOST_MCU_IMAGE_FILE_HEADER_MESSAGE,	
	HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE,
	HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE,
	HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE,
} PacketType_t;

typedef enum {
  STATUS_OK,
  STATUS_BUSY,
  STATUS_ERROR,
  STATUS_RECORD_EXITS_IN_OLD_ENTRY
} Status_t;

typedef enum {
	VEHICLE_OCCUPANCY_IDLE_STATE,
	VEHICLE_OCCUPANCY_ENTRY_STATE,
	VEHICLE_OCCUPANCY_EXIT_STATE,
} VehicleOccupancyTransitionState_t;

typedef enum {
	SENSOR_TRIGGER_INSTANCE,
	IMAGE_CAPTURE_COMPLETE_INSTANCE,
	JPEG_CREATION_COMPLETE_INSTANCE,	
	JPEG_CREATION_START_INSTANCE,
	MODEM_FREE_START_FTP_INSTANCE,
	CAMERA_POWER_OFF_INSTANCE,
	CAMERA_POWER_ON_INSTANCE,	
} InterTileInstance_t;

typedef enum {
    IMAGE_TRIGGER_CAPTURE_MULTIPLE_IMAGES,
    IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE,
	IMAGE_TRIGGER_MODEM_FREE_START_FTP,
} ImageTriggerType_t;     //TBD //added on 05_07_2022

typedef struct {
  unsigned char data[NUM_VEHICLE_OCCUPANCY_RECORDS];
} QueueEntry_t;
/*
typedef union {
    struct {
        unsigned short year;
        unsigned char month;
        unsigned char date;
        unsigned char hour;
        unsigned char minute;
		unsigned char second;
    };
    unsigned char info[TIMESTAMP_INFO_PACKET_LENGTH];
} RtcInfo_t;

typedef union {
	struct {
		unsigned char status;		 ///< 0 - NO Stable State, 1 - Final Stable State, 2 - Realization In Progress.
		unsigned short meterId;    ///< 1 to 99999
		RtcInfo_t timeStamp; ///< Instance Timestamp
	};
	unsigned char buffer[VEHICLE_OCCUPANCY_RECORD_SIZE];
} RecordInfo_t;

typedef struct {
	unsigned char totalNumberOfRecords;						///< 0 to 10
	RecordInfo_t record[MAX_VEHICLE_OCCUPANCY_RECORDS];	///< Record Information.
} VehicleOccupancyData_t;

typedef union {
	struct {
		VehicleOccupancyData_t data;
	};
	unsigned char buffer[MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH];
} VehicleOccupancyPacket_t;
*/
typedef struct {
	uint8_t exitLoopUponAnotherCameraDetected;
	uint8_t executeBrighterDataImageTask;

	uint8_t jpegFullSizeImageCount;
	uint8_t jpegNumberPlateSizeImageCount;
	uint8_t previousJpegFullSizeImageCount;
	uint8_t previousJpegNumberPlateSizeImageCount;

	uint32_t jpegFullSizeImageNumbers;
	uint32_t jpegNumberPlateSizeImageNumbers;
	uint32_t previousJpegFullSizeImageNumbers;
	uint32_t previousJpegNumberPlateSizeImageNumbers;

	uint16_t previousCompletedAnprNumImages;
	uint16_t previousResidualAnprNumImages;
	uint16_t previousCompletedJpegNumImages;
	uint16_t previousResidualJpegNumImages;
	uint16_t previousTotalJpegImages;
	uint16_t totalCapturedNumImages;

	uint32_t currentBrighterDataImages;
	uint32_t previousBrighterDataImages;
	uint32_t brighterDataImages;

	uint8_t	 ftpRunStatus;
	uint8_t	 transparentReadyStatus;
	
	char currentFileNameBase[200];
	char previousFileNameBase[200];
	
	//uint8_t currentFileNameBaseSize;
	//uint8_t previousFileNameBaseSize;
} ImageVars_t;

typedef union {
	ImageVars_t vars;
	uint8_t buffer[sizeof(ImageVars_t)];
} ImageInfo_t;


#endif /* HOST_MCU_TYPES_H_ */

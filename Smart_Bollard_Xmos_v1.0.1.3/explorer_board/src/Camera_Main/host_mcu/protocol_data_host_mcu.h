
#ifndef PROTOCOL_DATA_HOST_MCU_H_
#define PROTOCOL_DATA_HOST_MCU_H_

#define TIMESTAMP_INFO_PACKET_LENGTH   7

#define ANPR_STRING_PACKET_LENGTH  13
#define ANPR_STRING_MAX_LENGTH     12

#define MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH  101
#define MAX_VEHICLE_OCCUPANCY_RECORDS  		 10
#define VEHICLE_OCCUPANCY_RECORD_SIZE        10

#define FILE_NAME_MAX_LENGTH        200
#define FILE_HEADER_PACKET_LENGTH   205


typedef enum {
	VEHICLE_OCCUPANCY_NO_STABLE_STATE,
	VEHICLE_OCCUPANCY_FINAL_STABLE_STATE,
	VEHICLE_OCCUPANCY_REALIZATION_IN_PROGRESS,
} VehicleOccupancyStatus_t;

#pragma pack(1)

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


/* HOST_MCU_VEHICLE_RTC_INFO_MESSAGE */

typedef union {
    struct {
        unsigned char bayPosition: 1;			  ///< 1-Left/0-Right.
        unsigned char bayStatus: 1; 			  ///< 1-Entry/0-Exit.
        unsigned char configFileAvailable: 1;   ///< 1-Yes/0-No.
        unsigned char firmwareFileAvailable: 1; ///< 1-Yes/0-No.
		unsigned char powerOnInstance: 1;		  ///< 1-Power ON/0-with Sensor Trigger.	
		unsigned char sensorTrigger: 1;         ///< 1-Sensor Trigger/0-Periodic Trigger.
        unsigned char reserved: 2;			  ///< Reserved for Future Use.
    };
    unsigned char bitmap;
} BollardStatusBits_t;

typedef struct {
    RtcInfo_t currentRtc;
    BollardStatusBits_t statusInfo;
	uint8_t imageTriggerType;     //TBD //added on 05_07_2022
	uint8_t capture_sequence_number;
} BollardInfo_t;


/* HOST_MCU_ANPR_INFO_MESSAGE */

typedef union {
    struct {
        unsigned char totalStrings;		  		  ///< Value: 1 to 5.
        char string[ANPR_STRING_MAX_LENGTH];  ///< Maximum 12 characters.
    };
    unsigned char resultBuffer[ANPR_STRING_PACKET_LENGTH];
} AnprResult_t;


/* HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE */

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


/* HOST_MCU_IMAGE_FILE_HEADER_MESSAGE */

typedef union {
    struct {
        unsigned char number;               ///< Value: 1 to 255.
        char name[FILE_NAME_MAX_LENGTH];    ///< Length: 1 to 200 Characters.
        unsigned int size;                  ///< Value: Minimum 1.
    };
    unsigned char info[FILE_HEADER_PACKET_LENGTH];
} ImageFileHeader_t;


#endif /* PROTOCOL_DATA_HOST_MCU_H_ */

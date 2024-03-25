
#ifndef PROTOCOL_FRAME_HOST_MCU_H_
#define PROTOCOL_FRAME_HOST_MCU_H_

#include "host_mcu_types.h"


#define HOST_MCU_PACKET_PAYLOAD_LENGTH 	1024

#define HOST_MCU_PROTOCOL_FRAME_HEADER 	0x02
#define HOST_MCU_PROTOCOL_FRAME_FOOTER 	0x03


/*typedef enum {
	HOST_MCU_IDLE_TYPE,
    HOST_MCU_VEHICLE_RTC_INFO_MESSAGE,
	HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE,
	HOST_MCU_ANPR_INFO_MESSAGE,
    HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE,
    HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE,
	HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE,
	HOST_MCU_IMAGE_FILE_NAME_MESSAGE,
} PacketType_t;*/

#pragma pack(1)

typedef struct {
    unsigned short areaId;
    unsigned short customerId;
    unsigned short bollardId;
} DeviceInfo_t;

typedef struct {
    unsigned char header;
    unsigned char sequenceNumber;
    DeviceInfo_t deviceInfo;
	//PacketType_t packetType;
	unsigned char packetType;
} PacketInfo_t;

typedef struct {
    unsigned short length;
    unsigned char data[HOST_MCU_PACKET_PAYLOAD_LENGTH];
} Payload_t;

typedef struct {
    PacketInfo_t info;
    Payload_t payload;
    unsigned char checksum;
    unsigned char footer;
} HostMcuTxPacketStructure_t;

typedef struct {
    union {
        HostMcuTxPacketStructure_t packet;
        unsigned char buffer[sizeof(HostMcuTxPacketStructure_t)];
    };
    unsigned short packetLength;
} HostMcuTxPacket_t;

typedef HostMcuTxPacketStructure_t HostMcuRxPacketStructure_t;

typedef struct {
    union {
        HostMcuRxPacketStructure_t packet;
        unsigned char buffer[sizeof(HostMcuRxPacketStructure_t)];
    };
    unsigned short packetLength;
} HostMcuRxPacket_t;


#endif /* PROTOCOL_FRAME_HOST_MCU_H_ */

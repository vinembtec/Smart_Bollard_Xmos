
/*** Include Files ***/

#include <stdio.h>
#include <string.h>
#include <print.h>
#include <timer.h>
#include "host_mcu_interface.h"
#include "protocol_frame_host_mcu.h"
#include "protocol_data_host_mcu.h"
#include "host_mcu_uart.h"
#include "host_mcu_types.h"
#include "host_mcu_utils.h"
#include <xclib.h>
 #include <xcore/channel.h>


/*** Macro Definitions ***/

//#define MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH  		  101

#define HOST_MCU_RESPONSE_WAIT_TIME_INTERVAL_IN_MS    500

#define HOST_MCU_START_FTP_RECEIVE_WAIT_TIME_IN_MS    500//35000

#define HOST_MCU_FRAME_RECEIVE_TIMEOUT_IN_MS	      500

#define HOST_MCU_PACKET_SEND_NUM_ATTEMPTS             4

#define HOST_MCU_TAG "HOST MCU: "


/*** Type Definitions ***/

typedef struct {
    HostMcuTxPacket_t hostMcuTx;
    unsigned short waitTimeInterval;
    unsigned char totalAttempts;
    unsigned char numAttempts;
    unsigned char sequenceNumber;
    unsigned char lock: 1;
    unsigned char rxTimedout: 1;
} HostMcuTxInstance_t;

typedef struct {
    unsigned char buffer[sizeof(HostMcuRxPacketStructure_t)];
} UartReceiveData_t;

typedef struct {
    unsigned short byteCount;
    unsigned short length;
	unsigned receiveStartTimer;
	unsigned receiveCurrentTimer;
    unsigned char headerByte: 1;
    unsigned char footerByte: 1;
} UartRxFrameParas_t;

typedef struct {
    UartReceiveData_t data;
    UartRxFrameParas_t frame;
} UartReceive_t;

/*typedef union {
	struct {
		VehicleOccupancyData_t data;
	};
	unsigned char buffer[MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH];
} VehicleOccupancyPacket_t;*/

extern int mspUartCustomerId;
extern int mspUartbollardId;
extern int mspUartAreaId;
/*** Function Prototypes ***/

//static void HostMcuInterface_parseReceivedPackets(void);


/*** Variable Declarations ***/

int iPrintDebugMessage = 0;
static unsigned char rxSequenceNumber = 0;
unsigned char hostMcuResponseReceived = 0;
	
static DeviceInfo_t rxDeviceInfo;
static DeviceInfo_t configDeviceInfo;
static DeviceInfo_t defaultDeviceInfo;
static BollardInfo_t bollardInfo;
static HostMcuTxInstance_t txInstance;
static UartReceive_t hostMcuReceive;
VehicleOccupancyPacket_t vehicleOccupancyPacket;
unsigned char vehicleOccupancyPacketBuffer[MAX_VEHICLE_OCCUPANCY_PACKET_LENGTH];

unsigned char sendRtcAckTwiceOnBootup = 1;		//TBD //--to be commented //only for testing 
//unsigned char txState = 0;		//TBD //--to be commented //only for testing 
unsigned char cameraImageCapture; //added on 16_06_2022
unsigned char imageCaptureTriggerType; //added on 05_07_2022
extern unsigned char vehicleOccupancyStatusIndication;	//TBD //--to be commented //added on 09_06_2022
extern unsigned char hostMcuRxReceived;
extern unsigned char currentFileHeaderInfoTile1[205];

extern int year;
extern int month;
extern int date;
extern int hour;
extern int minute;
extern int second;
extern uint8_t capture_sequence_number;
//extern uint8_t capture_sequence_number1;
//extern uint8_t capture_sequence_number2;
//extern uint8_t capture_sequence_number3;
//extern uint8_t capture_sequence_number4;
extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
//extern void InterTileCommTile1_txInstance_seq_num(chanend_t c[], int seq_num);
char tempPrintStr[300];

extern unsigned char FTP_Ready_Flag_From_MCU;
extern unsigned char FTP_End_Flag_To_MCU;
extern unsigned char FTP_Ready_Flag_Set_in_XMOS;
/*** Function Definitions ***/
//{uint8_t, uint8_t,uint8_t,uint8_t,uint8_t} update_capture_seq_num(uint8_t num,uint8_t num1,uint8_t num2,uint8_t num3,uint8_t num4);

//void HostMcuInterface_sendDataPacket(unsigned char packetType);

//uint8_t capture_sequence_number;
//uint8_t capture_sequence_number1;
//uint8_t capture_sequence_number2;
//uint8_t capture_sequence_number3;
//uint8_t capture_sequence_number4 = 4;
uint8_t update_capture_seq_num()
{
	 return capture_sequence_number;
}


//uint8_t update_capture_seq_num1()
//{
	 //return capture_sequence_number1;
//}

//uint8_t update_capture_seq_num2()
//{
	 //return capture_sequence_number2;
//}

//uint8_t update_capture_seq_num3()
//{
	 //return capture_sequence_number3;
//}

//uint8_t update_capture_seq_num4()
//{
	 //return capture_sequence_number4;
//}

void HostMcuInterface_initialize(void)
{
    /* Initialize Host Mcu Uart */
    //HostMcuUart_initialize();

    /* Initialize Receive Buffers */
    memset(&hostMcuReceive, 0, sizeof(UartReceive_t));

    /* Initialize Tx Instances */
    memset(&txInstance, 0, sizeof(HostMcuTxInstance_t));

	/* Initialize Device Info */
    memset(&rxDeviceInfo, 0, sizeof(DeviceInfo_t));  
	memset(&configDeviceInfo, 0, sizeof(DeviceInfo_t));  
	memset(&defaultDeviceInfo, 0, sizeof(DeviceInfo_t));  
	
	/* Initialize Rtc Buffer */
    memset(&bollardInfo, 0, sizeof(BollardInfo_t));

    /* Initialize Vehicle Occupancy Packet_t Buffers */
    memset(&vehicleOccupancyPacket, 0, sizeof(VehicleOccupancyPacket_t));
	
	//TODO:: add here get the IDs from config and if Invalid then set to default...
	
	configDeviceInfo.areaId = DEFAULT_AREA_ID;
	configDeviceInfo.customerId = DEFAULT_CUSTOMER_ID;
	configDeviceInfo.bollardId = DEFAULT_BOLLARD_ID;

	//HostMcuUart_sendBuffer("Test Message From XMOS Board..!", strlen("Test Message From XMOS Board..!"));	//TBD //--to be Removed..
	//HostMcuInterface_sendDataPacket(HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE);	//TBD  //--only for Testing
}

static void HostMcuInterface_printTxLogs(HostMcuTxPacket_t *hostMcuTx)
{
    unsigned char packetType = hostMcuTx->packet.info.packetType;

    if ((0xF0 | HOST_MCU_VEHICLE_RTC_INFO_MESSAGE) == packetType)
    {
        packetType -= 0xF0;
    }

	//printstrln(" ");
	
    switch (packetType)
    {
        case HOST_MCU_VEHICLE_RTC_INFO_MESSAGE:
            //printf("\r\n%sVehicle/RTC Info Response Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Vehicle/RTC Info Response Sent.");
            break;
        case HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE:
            //printf("\r\n%sImage Capture Complete Request Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image Capture Complete Request Sent.");
            break;
        case HOST_MCU_ANPR_INFO_MESSAGE:
            //printf("\r\n%sAnpr Info Packet Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Anpr Info Packet Sent.");
            break;
		case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
			//printf("\r\n%sImage Deletion Table Request Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image Deletion Table Request Sent.");
			break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
			//printf("\r\n%sFile Transfer Start Request Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: File Transfer Start Request Sent.");
			break;
		case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
			//printf("\r\n%sFile Transfer End Request Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: File Transfer End Request Sent.");
			break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
            //printf("\r\n%sImage File Header Packet Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image File Header Packet Sent.");
            break;
		case HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE:
            //printf("\r\n%sCamera Power Off Packet Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Camera Power Off Packet Sent.");		
            break;
		case HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE:
            //printf("\r\n%sCamera Power On Packet Sent.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Camera Power On Packet Sent.");		
            break;			
        default:
			printstr("\r\nHOST MCU: Invalid packetType.");
			printint(packetType);
            break;
    }
	
	//printstrln(" ");	
    //HostMcuUtils_printByteArrayHexString(hostMcuTx->buffer, hostMcuTx->packetLength);
}

static void HostMcuInterface_printRxLogs(HostMcuRxPacket_t *hostMcuRx)
{
    unsigned char packetType = hostMcuRx->packet.info.packetType;

	if (((0xF0 | HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE) == packetType) ||				 
	((0xF0 | HOST_MCU_ANPR_INFO_MESSAGE) == packetType) || 				  
	((0xF0 | HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE) == packetType) || 
	((0xF0 | HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_IMAGE_FILE_HEADER_MESSAGE) == packetType))
	{
        packetType -= 0xF0;
	}	

	//printstrln(" ");
	
    switch (packetType)
    {
        case HOST_MCU_VEHICLE_RTC_INFO_MESSAGE:
            //printf("\r\n%sVehicle/RTC Info Packet Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Vehicle/RTC Info Packet Received.");
            break;
        case HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE:
            //printf("\r\n%sImage Capture Complete Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image Capture Complete Response Received.");
            break;
        case HOST_MCU_ANPR_INFO_MESSAGE:
            //printf("\r\n%sAnpr Info Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Anpr Info Response Received.");
            break;
		case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
			//printf("\r\n%sImage Deletion Table Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image Deletion Table Response Received.");
			break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
			//printf("\r\n%sFile Transfer Start Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: File Transfer Start Response Received...");
			break;
		case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
			//printf("\r\n%sFile Transfer End Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: File Transfer End Response Received.");
			break;
		case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
			//printf("\r\n%sImage File Header Response Received.", HOST_MCU_TAG);
			printstr("\r\nHOST MCU: Image File Header Response Received.");
			break;
        default:
            break;
    }
	
	//printstrln(" ");
    //HostMcuUtils_printByteArrayHexString(hostMcuRx->buffer, hostMcuRx->packetLength);
}

static unsigned char HostMcuInterface_calculateCrc(unsigned char *buffer, unsigned short length)
{
    unsigned char calculatedCrc = 0;
    unsigned short count;

    for (count = 0; count < length; count++)
    {
        calculatedCrc ^= buffer[count];
    }

    return calculatedCrc;
}

static void HostMcuInterface_buildVehicleRtcInfoResponse(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;	//TODO:: change here...
	request->packetLength += sizeof(request->packet.info.sequenceNumber);
	//request->packet.info.sequenceNumber = rxSequenceNumber;
	//request->packetLength += sizeof(rxSequenceNumber);    
	//rxSequenceNumber = 0;

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = (0xF0 | HOST_MCU_VEHICLE_RTC_INFO_MESSAGE);
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildImageCaptureCompleteMessage(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildAnprInfoMessage(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_ANPR_INFO_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    //request->packet.payload.length = 0;	//TODO:: change here length 
	request->packet.payload.length = sizeof(AnprResult_t);	//TODO:: change here length //TBD //--need to change	
    request->packetLength += sizeof(request->packet.payload.length);
	
	//TODO:: change here Data //TBD //--need to change	
	request->buffer[request->packetLength++] = 1;	
	strncpy((char *)&request->buffer[request->packetLength], "ANPR_TEST_00", strlen("ANPR_TEST_00"));
	request->packetLength += strlen("ANPR_TEST_00");
	
	//TODO:: modify here ANPR string data	
    //memcpy(request->packet.payload.data, anpr.info, sizeof(anpr.info)); 
    //request->packetLength += sizeof(anpr.info);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildImageDeletionTableRequest(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildFileTransferStartRequest(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildFileTransferEndRequest(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildImageFileHeaderRequest(HostMcuTxPacket_t *request)
{
	ImageFileHeader_t imageFileHeader;
	
	memset(&imageFileHeader, 0, sizeof(ImageFileHeader_t));
	
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_IMAGE_FILE_HEADER_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

	request->packet.payload.length = sizeof(ImageFileHeader_t);	
    request->packetLength += sizeof(request->packet.payload.length);
	
	//TBD  //TODO:: modify here data	
	/*imageFileHeader.number = 1;
	strncpy(imageFileHeader.name, "IMG_FL_8042_32091_20220614_190822_00", strlen("IMG_FL_8042_32091_20220614_190822_00"));
	imageFileHeader.size = 100000;		*/

	imageFileHeader.number = currentFileHeaderInfoTile1[0];
	strncpy(imageFileHeader.name, &currentFileHeaderInfoTile1[1], 200);
	imageFileHeader.size = (currentFileHeaderInfoTile1[201] << 24) | (currentFileHeaderInfoTile1[202] << 16) |
								(currentFileHeaderInfoTile1[203] << 8) | currentFileHeaderInfoTile1[204];

	memcpy(request->packet.payload.data, &imageFileHeader, sizeof(ImageFileHeader_t));
	request->packetLength += sizeof(ImageFileHeader_t);
	
    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildCameraPowerONRequest(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_buildCameraPowerOFFRequest(HostMcuTxPacket_t *request)
{
    request->packet.info.header = HOST_MCU_PROTOCOL_FRAME_HEADER;
    request->packetLength += sizeof(request->packet.info.header);

    request->packet.info.sequenceNumber = txInstance.sequenceNumber;
    request->packetLength += sizeof(request->packet.info.sequenceNumber);

    request->packet.info.deviceInfo.areaId = configDeviceInfo.areaId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.areaId);

    request->packet.info.deviceInfo.customerId = configDeviceInfo.customerId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.customerId);

    request->packet.info.deviceInfo.bollardId = configDeviceInfo.bollardId;
    request->packetLength += sizeof(request->packet.info.deviceInfo.bollardId);

    request->packet.info.packetType = HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE;
    request->packetLength += sizeof(request->packet.info.packetType);

    request->packet.payload.length = 0;
    request->packetLength += sizeof(request->packet.payload.length);

    unsigned char calculateCrcStartIndex = (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    unsigned short crcLength = request->packetLength - (sizeof(request->packet.info.header) + sizeof(request->packet.info.sequenceNumber));
    
	request->buffer[request->packetLength++] = HostMcuInterface_calculateCrc(&request->buffer[calculateCrcStartIndex], crcLength);
    request->buffer[request->packetLength++] = HOST_MCU_PROTOCOL_FRAME_FOOTER;
}

static void HostMcuInterface_resetTxInstances(void)
{
	unsigned char sequenceNumber = txInstance.sequenceNumber;
    memset(&txInstance, 0, sizeof(HostMcuTxInstance_t));
	txInstance.sequenceNumber = sequenceNumber;
	txInstance.totalAttempts = HOST_MCU_PACKET_SEND_NUM_ATTEMPTS;
}

static void HostMcuInterface_sendMessage(unsigned char packetType)
{    
	if (1 == txInstance.lock)
	{
		//printf("\r\n%sWaiting for Host Mcu Response.", HOST_MCU_TAG);
		printstr("\r\nHOST MCU: Waiting for Host Mcu Response.");
		return;
	}
	
	txInstance.lock = 1;
	
    /* Reset the Tx Packet */
    memset(&txInstance.hostMcuTx, 0, sizeof(HostMcuTxPacket_t));
	
    /* Reset Receive Buffers */
    memset(&hostMcuReceive, 0, sizeof(UartReceive_t));
	hostMcuResponseReceived = 0;
	hostMcuRxReceived = 0;

	//TODO:: change the totalAttempts as defined in config...
    txInstance.totalAttempts = HOST_MCU_PACKET_SEND_NUM_ATTEMPTS;	
	
	if(packetType == HOST_MCU_START_OF_FILE_TRANSFER_WITHOUT_DELAY_MESSAGE)
	{
		/* Reset the Tx Instances Parameters */
		txInstance.waitTimeInterval = 50;
		
		packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
		
		/* Load the Packet Type */
		txInstance.hostMcuTx.packet.info.packetType = HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE;
		
		//TODO:: change the totalAttempts as defined in config...
		txInstance.totalAttempts = HOST_MCU_PACKET_SEND_NUM_ATTEMPTS + 5;	
	}
	else
	{
		/* Reset the Tx Instances Parameters */
		txInstance.waitTimeInterval = HOST_MCU_RESPONSE_WAIT_TIME_INTERVAL_IN_MS;
		
		/* Load the Packet Type */
		txInstance.hostMcuTx.packet.info.packetType = packetType;
	}
		
	if (!((1 == sendRtcAckTwiceOnBootup) && (1 == txInstance.sequenceNumber)))	//TBD
	{
		/* Increment Transmit Attempt Count */
		txInstance.numAttempts++;

		/* Increment Packet Sequence Number */
		if (1 == txInstance.numAttempts)
		{
			txInstance.sequenceNumber++;
		}
	}
	
    switch (packetType)
    {
        case HOST_MCU_VEHICLE_RTC_INFO_MESSAGE:
			txInstance.waitTimeInterval = 0;
			//txInstance.totalAttempts = 0;
			txInstance.numAttempts = 0;
			txInstance.lock = 0;
			HostMcuInterface_buildVehicleRtcInfoResponse(&txInstance.hostMcuTx);
            break;
        case HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE:
            HostMcuInterface_buildImageCaptureCompleteMessage(&txInstance.hostMcuTx);
            break;
		case HOST_MCU_ANPR_INFO_MESSAGE:
			HostMcuInterface_buildAnprInfoMessage(&txInstance.hostMcuTx);
			break;
        case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
            HostMcuInterface_buildImageDeletionTableRequest(&txInstance.hostMcuTx);
            break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
			//TODO:: change timeInterval as defined in config...
			////txInstance.waitTimeInterval = HOST_MCU_START_FTP_RECEIVE_WAIT_TIME_IN_MS;		
			HostMcuInterface_buildFileTransferStartRequest(&txInstance.hostMcuTx);
			break;
        case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
            HostMcuInterface_buildFileTransferEndRequest(&txInstance.hostMcuTx);
            break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
            HostMcuInterface_buildImageFileHeaderRequest(&txInstance.hostMcuTx);
            break;
        case HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE:
            HostMcuInterface_buildCameraPowerOFFRequest(&txInstance.hostMcuTx);
            break;
        case HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE:
            HostMcuInterface_buildCameraPowerONRequest(&txInstance.hostMcuTx);
            break;			
        default:
            break;
    }
	
	printf("\r\n SENT");
	char new;
	for(new = 0; new < txInstance.hostMcuTx.packetLength; new++)
	{
		printf(" %02X", txInstance.hostMcuTx.buffer[new]);
	}
	printf("\r\n");
	
    /* Send the Packet */
    HostMcuUart_sendBuffer(txInstance.hostMcuTx.buffer, txInstance.hostMcuTx.packetLength);
	
    /* Print Debug Logs */
    HostMcuInterface_printTxLogs(&txInstance.hostMcuTx);
}

unsigned char HostMcuInterface_waitForResponse(void)
{	
	/* Wait for Receive Response */ 	
	int waitTimeInMiliSeconds = txInstance.waitTimeInterval;
	
	for (int wait = 0; wait < waitTimeInMiliSeconds;  wait++) 
	{		
		if ((1 == hostMcuResponseReceived) || (1 == hostMcuRxReceived) || ((FTP_Ready_Flag_From_MCU || FTP_End_Flag_To_MCU)&&(FTP_Ready_Flag_Set_in_XMOS)))
		{			
			//printf("\r\nhostMcuRxReceived: %d\n", hostMcuRxReceived);
			//printstrln(" ");
			printstr("\r\nHOST MCU: hostMcuResponseReceived= ");
			printint(hostMcuResponseReceived);
			FTP_Ready_Flag_Set_in_XMOS = 0;
			txInstance.lock = 0;			
			//InterTileCommTile1_sendResponse(txCmd, HOST_MCU_VALID_RESPONSE);
			//break;
			return 2;
		}
		
		/* Decrement Response Time Interval Timer */
		if (txInstance.waitTimeInterval > 0)
		{
			txInstance.waitTimeInterval--;			
			/*if (0 == txInstance.waitTimeInterval)
			{
				break;
			}*/
		}
		
		delay_milliseconds((int)1);
	}		
	
	/* Host Mcu Communication Response Timeout */
	if (((0 == hostMcuResponseReceived) || (0 == hostMcuRxReceived)) && (0 == txInstance.waitTimeInterval))
	{
		/* Re-send the packet */
		if (txInstance.numAttempts <= txInstance.totalAttempts)
		{
			printf("\r\n%stxInstance.numAttempts %d", HOST_MCU_TAG, txInstance.numAttempts);
			//printstrln(" ");
			////printstr("\r\nHOST MCU: txInstance.numAttempts ");
			////printint(txInstance.numAttempts);
			////printstr("\r\nHOST MCU: txInstance.totalAttempts ");
			////printint(txInstance.totalAttempts);
			
			txInstance.lock = 0;			

			/* All Tx attempts are Finished */
			if (txInstance.numAttempts == txInstance.totalAttempts)
			{
				//printf("\r\n%sAll Tx Attempts Finished.", HOST_MCU_TAG);
				//printstrln(" ");
				printstr("\r\nHOST MCU: All Tx Attempts Finished.");
				/* Reset Tx Instances */
				HostMcuInterface_resetTxInstances();
				//InterTileCommTile1_sendResponse(txCmd, HOST_MCU_NO_RESPONSE);
				return 3;
			}
			
			return 1;
		}
	}	

	return 0;
}
#if 1
static void HostMcuInterface_handleResponse(void)
{	
	/*if (0 == txInstance.waitTimeInterval)
	{
		return;
	}*/
	
     //sprintf(tempPrintStr, "\r\nHostMcuInterface_handleResponse.");
			                //Debug_TextOut(0, tempPrintStr);
	
	/* Wait for Receive Response */ 	
	int waitTimeInMiliSeconds = 200;//txInstance.waitTimeInterval;		
	
	for (int wait = 0; wait < waitTimeInMiliSeconds;  wait++) 
	{
		delay_milliseconds((int)1);
		
		if ((1 == hostMcuResponseReceived) || (1 == hostMcuRxReceived))
		{
			break;
		}
		
		/* Decrement Response Time Interval Timer */
		if (txInstance.waitTimeInterval > 0)
		{
			txInstance.waitTimeInterval--;			
			if (0 == txInstance.waitTimeInterval)
			{
				break;
			}
		}
	}		

	/* Host Mcu Communication Response Timeout */
	if (((0 == hostMcuResponseReceived) || (0 == hostMcuRxReceived)) && (0 == txInstance.waitTimeInterval))
	{
		/* Re-send the packet */
		if (txInstance.numAttempts <= txInstance.totalAttempts)
		{
			//printf("\r\n%stxInstance.numAttempts %d", HOST_MCU_TAG, txInstance.numAttempts);
			//printstrln(" ");
			printstr("\r\nHOST MCU: txInstance.numAttempts ");
			printint(txInstance.numAttempts);
			
			txInstance.lock = 0;			

			/* All Tx attempts are Finished */
			if (txInstance.numAttempts == txInstance.totalAttempts)
			{
				//txState++;		//TBD  //--only for Testing
				//printf("\r\n%sAll Tx Attempts Finished", HOST_MCU_TAG);
				//printstrln(" ");
				printstr("\r\nHOST MCU: All Tx Attempts Finished.");
				/* Reset Tx Instances */
				HostMcuInterface_resetTxInstances();
			}
		}
	}	
	/* Parse/Process Received Data */
	else if (((1 == hostMcuResponseReceived) || (1 == hostMcuRxReceived)) && (txInstance.waitTimeInterval > 0))
	{
		HostMcuInterface_parseReceivedPackets();		
	}
}

void HostMcuInterface_sendDataPacket(unsigned char packetType)
{
	//for (int attempts = 0;  attempts < txInstance.totalAttempts;  attempts++)
	for (int attempts = 0;  attempts < HOST_MCU_PACKET_SEND_NUM_ATTEMPTS;  attempts++)			
	{
		HostMcuInterface_sendMessage(packetType);
		
		if (HOST_MCU_VEHICLE_RTC_INFO_MESSAGE == packetType)			
		{
			break;
		}
		
		HostMcuInterface_handleResponse();
		
		if (1 == hostMcuResponseReceived)
		{
			hostMcuResponseReceived = 0;
			break;
		}
	}
}
#endif

static void HostMcuInterface_processVehicleRtcInfoPacket(HostMcuRxPacket_t *hostMcuRx)
{
  //sprintf(tempPrintStr, "\r\HostMcuInterface_processVehicleRtcInfoPacket.");
			                //Debug_TextOut(0, tempPrintStr);
  
    memset(&bollardInfo, 0, sizeof(BollardInfo_t));

	/* Parse Received Data */ 	
	rxSequenceNumber = hostMcuRx->packet.info.sequenceNumber;
	
	/* Rtc Info */ 
    memcpy(bollardInfo.currentRtc.info, hostMcuRx->packet.payload.data, sizeof(RtcInfo_t));
    
    
  // capture_sequence_number =  hostMcuRx->packet.payload.data[8];
   capture_sequence_number =  hostMcuRx->packet.payload.data[9];
   //capture_sequence_number2 =  hostMcuRx->packet.payload.data[10];
   //capture_sequence_number3 =  hostMcuRx->packet.payload.data[11];
   //capture_sequence_number4 = 4;
   
   printf("\n Sequence number from MSP =========================================================> %d",capture_sequence_number);
   sprintf(tempPrintStr, "\n Sequence number from MSP ==============> %d",capture_sequence_number);
			 Debug_TextOut(0, tempPrintStr);
   
   //sprintf(tempPrintStr, "%02X %02X %02X %02X %02X ",capture_sequence_number,capture_sequence_number1,capture_sequence_number2,capture_sequence_number3,capture_sequence_number4);
			
			 //Debug_TextOut(0, tempPrintStr);
			 
			 
			 sprintf(tempPrintStr, "\nSeq Num %02X\n", capture_sequence_number);
			 Debug_TextOut(0, tempPrintStr);
   
 //  InterTileCommTile1_txInstance_seq_num(c[], capture_sequence_number);
 //chanend_t c[];
 //chanend_t seq_num;
 //outuint(seq_num, capture_sequence_number);
 //outuchar(seq_c, capture_sequence_number);
			                
		//int count;    
	
	    //for (count = 0; count < (sizeof(RtcInfo_t)+4); count++)
	    //{
			//char byteString[5];
			//sprintf(byteString, "%02X ", hostMcuRx->packet.payload.data[count]);
			////printstr(byteString);
			 //Debug_TextOut(0, byteString);
	    //}
        

    /*printf("\r\n%sRTC REQUEST DATA:", HOST_MCU_TAG);
    printf("\r\n%sareaId: %d", HOST_MCU_TAG, hostMcuRx->packet.info.deviceInfo.areaId);
    printf("\r\n%scustomerId: %d", HOST_MCU_TAG, hostMcuRx->packet.info.deviceInfo.customerId);
    printf("\r\n%sbollardId: %d", HOST_MCU_TAG, hostMcuRx->packet.info.deviceInfo.bollardId);*/
    
	/* Set Image TimeStamp */
	//SetMSP_RTC(0);
	//TODO:: add here parse set timestamp

	//TODO:: to be UNcommented
	year = bollardInfo.currentRtc.year;
	month = bollardInfo.currentRtc.month;
	date = bollardInfo.currentRtc.date;
	hour = bollardInfo.currentRtc.hour;
	minute = bollardInfo.currentRtc.minute;
	second = bollardInfo.currentRtc.second;

	/* Bollard Status Info */
    bollardInfo.statusInfo.bitmap = hostMcuRx->packet.payload.data[sizeof(RtcInfo_t)];
	
	if (bollardInfo.statusInfo.bayPosition)
    {
        //printf("\r\n%sbayPosition: LEFT SENSOR", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: bayPosition: LEFT SENSOR.");
		cameraImageCapture = 1;	
    }
    else
    {
        //printf("\r\n%sbayPosition: RIGHT SENSOR", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: bayPosition: RIGHT SENSOR.");
		cameraImageCapture = 2;
    }

    if (bollardInfo.statusInfo.bayStatus)
    {
        //printf("\r\n%sbayStatus: VEHICLE-IN", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: bayStatus: VEHICLE-IN.");
		vehicleOccupancyStatusIndication = VEHICLE_OCCUPANCY_ENTRY_STATE;
    }
    else
    {
        //printf("\r\n%sbayStatus: VEHICLE-OUT", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: bayStatus: VEHICLE-OUT.");
		vehicleOccupancyStatusIndication = VEHICLE_OCCUPANCY_EXIT_STATE;
    }

    /*printf("\r\n%sconfigFileAvailable: %d", HOST_MCU_TAG, bollardInfo.statusInfo.configFileAvailable);
    printf("\r\n%sfirmwareFileAvailable: %d", HOST_MCU_TAG, bollardInfo.statusInfo.firmwareFileAvailable);
	printf("\r\n%spowerOnInstance: %d", HOST_MCU_TAG, bollardInfo.statusInfo.powerOnInstance);
	printf("\r\n%ssensorTrigger: %d", HOST_MCU_TAG, bollardInfo.statusInfo.sensorTrigger);	
    printf("\r\n");*/

	//TBD //added on 05_07_2022
	/* Bollard Status Info */
    bollardInfo.imageTriggerType = hostMcuRx->packet.payload.data[sizeof(RtcInfo_t) + sizeof(BollardStatusBits_t)];
	imageCaptureTriggerType = bollardInfo.imageTriggerType;
	//printf("\r\n%sbollardInfo.imageTriggerType=%d.", HOST_MCU_TAG, bollardInfo.imageTriggerType);
	//printstrln("\nbollardInfo.imageTriggerType=");
	//printint(bollardInfo.imageTriggerType);
	//printstrln("\n");
	
	if (IMAGE_TRIGGER_CAPTURE_SINGLE_IMAGE == imageCaptureTriggerType)
    {
        //printf("\r\n%sSingle IMG Capture.", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: Single IMG Capture.");
    }
    else if (IMAGE_TRIGGER_MODEM_FREE_START_FTP == imageCaptureTriggerType)
    {
        //printf("\r\n%sModem Free Start FTP.", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: Modem Free Start FTP.");
    }
    else //if (IMAGE_TRIGGER_CAPTURE_MULTIPLE_IMAGES == imageCaptureTriggerType)
    {
        //printf("\r\n%sMultiple IMG Capture.", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: Multiple IMG Capture.");
    }
}

static void HostMcuInterface_processResponseImageDeletionTable(HostMcuRxPacket_t *hostMcuRx)
{
	/* Copy Received Data to Vehicle Occupancy Packet Buffer */
	//memcpy(vehicleOccupancyPacket.buffer, hostMcuRx->packet.payload.data, hostMcuRx->packet.payload.length);
	memcpy(vehicleOccupancyPacketBuffer, hostMcuRx->packet.payload.data, hostMcuRx->packet.payload.length);

	//TODO:: add here InterTile task to report to TILE[0]...to send VehicleOccupancyStatusInfo to TILE[0]...	
	/* Parse and Process Vehicle Occupancy Status Info and Take Further Action such as deleting file, rearrange the file list. */
	//ImageDeletion_processVehicleOccupancyStatusInfo(vehicleOccupancyPacket.buffer, hostMcuRx->packet.payload.length);
}

static unsigned char HostMcuInterface_processResponse(HostMcuRxPacket_t *hostMcuRx)
{
    unsigned char packetType = hostMcuRx->packet.info.packetType;
    printstr("\r\nProcessing response Packet from HOST MCU.");
     //sprintf(tempPrintStr, "\r\nProcessing response Packet from HOST MCU packet type %02X.",hostMcuRx->packet.info.packetType);
			                //Debug_TextOut(0, tempPrintStr);
    //sprintf(tempPrintStr, "\r\nCapture Sequence Number %02X.",hostMcuRx->packet.payload.data[0]);
			                //Debug_TextOut(0, tempPrintStr);

	if (((0xF0 | HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE) == packetType) ||				 
	((0xF0 | HOST_MCU_ANPR_INFO_MESSAGE) == packetType) || 				  
	((0xF0 | HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE) == packetType) || 
	((0xF0 | HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_IMAGE_FILE_HEADER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE) == packetType))
	{
        packetType -= 0xF0;
	}	

    switch (packetType)
    {
        case HOST_MCU_VEHICLE_RTC_INFO_MESSAGE:
            if (sizeof(BollardInfo_t) == hostMcuRx->packet.payload.length)
            {
                HostMcuInterface_processVehicleRtcInfoPacket(hostMcuRx);
            }
            else
            {
                return 0;
            }
            break;
        case HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE:
            break;
		case HOST_MCU_ANPR_INFO_MESSAGE:
			break;
        case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
            if (sizeof(VehicleOccupancyData_t) == hostMcuRx->packet.payload.length)
            {
                HostMcuInterface_processResponseImageDeletionTable(hostMcuRx);
            }
            else
            {
                return 0;
            }            
            break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:
			break;
        case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
            break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
            break;
		case HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE:
            break;
		case HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE:
            break;
        default:
			//printf("\r\n%sINVALID COMMAND\n", HOST_MCU_TAG);
			//printstrln(" ");
			printstr("\r\nHOST MCU: Invalid packetType.");
			//sprintf(tempPrintStr, "\r\nPacket type error.");
			//                Debug_TextOut(0, tempPrintStr);
            return 0;
    }

    return 1;
}

static void HostMcuInterface_processReceivedPackets(HostMcuRxPacket_t *hostMcuRx)
{
  //sprintf(tempPrintStr, "\r\nHostMcuInterface_processReceivedPackets.");
			        //        Debug_TextOut(0, tempPrintStr);
	
			
	//	sprintf(tempPrintStr, "\r\nHOST MCU: Packet Received.");
			              //  Debug_TextOut(0, tempPrintStr);
			                
		//int count;    
	
	    //for (count = 0; count < hostMcuRx->packetLength; count++)
	    //{
			//char byteString[5];
			//sprintf(byteString, "%02X ", hostMcuRx->buffer[count]);
			////printstr(byteString);
			 //Debug_TextOut(0, byteString);
	    //}
  
    /* Process Received Packet */
    if (0 == HostMcuInterface_processResponse(hostMcuRx))
    {
        //printf("\r\n%sInvalid Packet.\n", HOST_MCU_TAG);
		//printstrln(" ");
		printstr("\r\nHOST MCU: Invalid Packet.");
		HostMcuUtils_printByteArrayHexString(hostMcuRx->buffer, hostMcuRx->packetLength);
		
		/* sprintf(tempPrintStr, "\r\nHOST MCU: Invalid Packet.");
			                Debug_TextOut(0, tempPrintStr);
			                
		int count;    
	
	    for (count = 0; count < hostMcuRx->packetLength; count++)
	    {
			char byteString[5];
			sprintf(byteString, "%02X ", hostMcuRx->buffer[count]);
			//printstr(byteString);
			 Debug_TextOut(0, byteString);
	    } */
        
        return;
    }

    /* Reset Tx Instances */
    HostMcuInterface_resetTxInstances();

    /* Print Debug Logs */
    HostMcuInterface_printRxLogs(hostMcuRx);	
	
	/* Send Ack */
    unsigned char packetType = hostMcuRx->packet.info.packetType;

	if (((0xF0 | HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE) == packetType) ||				 
	((0xF0 | HOST_MCU_ANPR_INFO_MESSAGE) == packetType) || 				  
	((0xF0 | HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE) == packetType) || 
	((0xF0 | HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE) == packetType) ||
	((0xF0 | HOST_MCU_IMAGE_FILE_HEADER_MESSAGE) == packetType)||
	((0xF0 | HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE) == packetType)||
	((0xF0 | HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE) == packetType))
	{
        packetType -= 0xF0;
	}	

    switch (packetType)
    {
        case HOST_MCU_VEHICLE_RTC_INFO_MESSAGE:
			HostMcuInterface_sendMessage(HOST_MCU_VEHICLE_RTC_INFO_MESSAGE);		

			/* Sending Twice as First time junk data gets send.. */
			if (1 == sendRtcAckTwiceOnBootup)
			{					
				HostMcuInterface_sendMessage(HOST_MCU_VEHICLE_RTC_INFO_MESSAGE);	
				sendRtcAckTwiceOnBootup = 0;
			}
            break;
        case HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE:
            break;
		case HOST_MCU_ANPR_INFO_MESSAGE:
			break;
        case HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE:
            break;
		case HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE:	
			break;
        case HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE:
            break;
        case HOST_MCU_IMAGE_FILE_HEADER_MESSAGE:
            break;
        case HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE:
            break;
        case HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE:
            break;			
        default:
			break;
    }
}

void HostMcuInterface_txTaskHandler(unsigned char txState)
{
	if ((txState >= HOST_MCU_VEHICLE_RTC_INFO_MESSAGE) && (txState <= HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE))
	{
		HostMcuInterface_sendMessage(txState);
	}
}
//------------------------------------------

void HostMcuInterface_parseReceivedPackets(void)
{
    HostMcuRxPacket_t hostMcuRx;
	
    memcpy(hostMcuRx.buffer, hostMcuReceive.data.buffer, hostMcuReceive.frame.byteCount);
    hostMcuRx.packetLength = hostMcuReceive.frame.byteCount;
	hostMcuRx.packet.payload.length = (hostMcuReceive.data.buffer[10] << 8) | hostMcuReceive.data.buffer[9];
    //hostMcuReceive.frame.byteCount = 0;
	memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
	
	//printf("\r\nhostMcuRx.packet.payload.length: %d", hostMcuRx.packet.payload.length);

    unsigned char calculatedCrc = HostMcuInterface_calculateCrc(&hostMcuRx.buffer[2], (hostMcuRx.packetLength - 4));

    /* Compare Crc Checksum */
    if (calculatedCrc != hostMcuRx.buffer[hostMcuRx.packetLength - 2])
    {
        //printf("\r\n%sCrc Match Error(%02X %02X)", HOST_MCU_TAG, calculatedCrc, hostMcuRx.buffer[hostMcuRx.packetLength - 2]);
		char byteString[50];
		sprintf("\r\n%sCrc Match Error(%02X %02X)", HOST_MCU_TAG, calculatedCrc, hostMcuRx.buffer[hostMcuRx.packetLength - 2]);
		printstr(byteString);
		//Debug_TextOut(0, byteString);
	
        /* Print the Received Data */
        HostMcuUtils_printByteArrayHexString(hostMcuRx.buffer, hostMcuRx.packetLength);
        
        	                
		//int count;    
	
	    //for (count = 0; count < (sizeof(RtcInfo_t)+4); count++)
	    //{
			//char byteString[5];
			//sprintf(byteString, "%02X ", hostMcuRx.buffer[count]);
			////printstr(byteString);
			 //Debug_TextOut(0, byteString);
	    //}
        return;
    }

    /* Process the Received Packet */
    HostMcuInterface_processReceivedPackets(&hostMcuRx);
}

//unsigned short tick = 0;
//unsigned short rxWaitCount = 0;

unsigned char HostMcuInterface_decrementFrameReceiveTimeoutTimer(void)
{
	if (hostMcuReceive.frame.receiveStartTimer > 0)
	{
		asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveCurrentTimer));
		
		if ((((hostMcuReceive.frame.receiveCurrentTimer - hostMcuReceive.frame.receiveStartTimer) / 100) / 1000) >= HOST_MCU_FRAME_RECEIVE_TIMEOUT_IN_MS)
		{
			//printf("\r\n%sFrame Receive Timer Timedout. (%d)", HOST_MCU_TAG, (((hostMcuReceive.frame.receiveCurrentTimer - hostMcuReceive.frame.receiveStartTimer) / 100) / 1000));
			//printstrln(" ");
			/*printstr("\r\nHOST MCU: Frame Receive Timer Timedout. ");
			printstrln("(");
			printint((((hostMcuReceive.frame.receiveCurrentTimer - hostMcuReceive.frame.receiveStartTimer) / 100) / 1000));
			printstrln(")");*/
			char byteString[50];
			printf("\r\n%sFrame Receive Timer Timedout. (%d)", HOST_MCU_TAG, (((hostMcuReceive.frame.receiveCurrentTimer - hostMcuReceive.frame.receiveStartTimer) / 100) / 1000));
			printstr(byteString);			
			memset(&hostMcuReceive, 0, sizeof(UartReceive_t));
			return 0;
		}
	}
	
	//-----------------------------------------------------------------------------
	/*if (++rxWaitCount > 10000)
	{
		rxWaitCount = 0;
		tick++;
		printf("\r\nSysTick %d", tick);
	}*/
	//-----------------------------------------------------------------------------
	
	return 1;
}

void HostMcuInterface_handleUartReceiveData(unsigned char rxByte)
{
  // sprintf(tempPrintStr, "\r\HostMcuInterface_handleUartReceiveData.");
//			                Debug_TextOut(0, tempPrintStr);
   
    if (hostMcuReceive.frame.byteCount++ >= (sizeof(HostMcuRxPacketStructure_t)))
    {
        memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
    }

    hostMcuReceive.data.buffer[hostMcuReceive.frame.byteCount - 1] = rxByte;
   //  sprintf(tempPrintStr, "\r\HostMcuReceive.frame.byteCount is %d",hostMcuReceive.frame.byteCount);
	//		                Debug_TextOut(0, tempPrintStr);

    switch (hostMcuReceive.frame.byteCount)// 
    {
        case 1:
            /* Header Byte 1 */
            if (HOST_MCU_PROTOCOL_FRAME_HEADER == rxByte)
            {
                hostMcuReceive.frame.headerByte = 1;				
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
            else
            {
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
            }
            break;
        case 2:
            /* Sequence Number */
			if (!((hostMcuReceive.frame.headerByte) && (rxByte >= 0)))
            {
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
            }
			else
			{
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
			}
            break;
        case 3:
            /* AID Lower Byte  */
            if (hostMcuReceive.frame.headerByte)
            {
                rxDeviceInfo.areaId = rxByte;
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
			else
			{
				memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 4:
            /* AID  Higher Byte */
            if (hostMcuReceive.frame.headerByte)
            {
				rxDeviceInfo.areaId |= (rxByte << 8);
				mspUartAreaId = rxDeviceInfo.areaId;
				configDeviceInfo.areaId = rxDeviceInfo.areaId;
				
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
				
				////if (rxDeviceInfo.areaId != configDeviceInfo.areaId)
				if((rxDeviceInfo.areaId != 1)&&(rxDeviceInfo.areaId != 99))	
				{
					memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
				}
            }
			else
			{
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 5:
            /* CID Lower Byte */
            if (hostMcuReceive.frame.headerByte)
            {
                rxDeviceInfo.customerId = rxByte;
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
			else
			{
				memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 6:
            /* CID Higher Byte */
            if (hostMcuReceive.frame.headerByte)
            {
				rxDeviceInfo.customerId |= (rxByte << 8);
				mspUartCustomerId = rxDeviceInfo.customerId;
				configDeviceInfo.customerId = rxDeviceInfo.customerId;
				
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
				
				////if (rxDeviceInfo.customerId != configDeviceInfo.customerId)
				if(rxDeviceInfo.customerId > 65535)
				{
					memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
				}
            }
			else
			{
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 7:
            /* MID Lower Byte */
            if (hostMcuReceive.frame.headerByte)
            {
                rxDeviceInfo.bollardId = rxByte;
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
			else
			{
				memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 8:
            /* MID Higher Byte */
            if (hostMcuReceive.frame.headerByte)
            {
				rxDeviceInfo.bollardId |= (rxByte << 8);
				mspUartbollardId = rxDeviceInfo.bollardId;
				configDeviceInfo.bollardId = rxDeviceInfo.bollardId;
				
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
				
				////if (rxDeviceInfo.bollardId != configDeviceInfo.bollardId)
				if(rxDeviceInfo.bollardId > 65535)
				{
					memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
				}
            }
			else
			{
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
			}
            break;
        case 9:
            /* Packet Type */
            if (!((hostMcuReceive.frame.headerByte) &&
                 ((HOST_MCU_VEHICLE_RTC_INFO_MESSAGE == rxByte) || 
				 ((0xF0 | HOST_MCU_IMAGE_CAPTURE_COMPLETE_MESSAGE) == rxByte) ||				 
                  ((0xF0 | HOST_MCU_ANPR_INFO_MESSAGE) == rxByte) || 				  
				  ((0xF0 | HOST_MCU_IMAGE_DELETION_TABLE_MESSAGE) == rxByte) || 
				  ((0xF0 | HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE) == rxByte) ||
				  ((0xF0 | HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE) == rxByte) ||
				  ((0xF0 | HOST_MCU_IMAGE_FILE_HEADER_MESSAGE) == rxByte) ||
				  ((0xF0 | HOST_MCU_CAMERA_POWER_OFF_REQUEST_MESSAGE) == rxByte) ||
				  ((0xF0 | HOST_MCU_CAMERA_POWER_ON_REQUEST_MESSAGE) == rxByte))))
            {
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
            }
            else
            {
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
            break;
        case 10:
            /* Frame Length Lower Byte */
            if (hostMcuReceive.frame.headerByte)
            {
                hostMcuReceive.frame.length = rxByte;
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
            }
            else
            {
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
            }
            break;
        case 11:
            /* Frame Length Higher Byte */
            if (hostMcuReceive.frame.headerByte)
            {
                hostMcuReceive.frame.length |= (unsigned short)rxByte << 8;
				asm volatile("gettime %0" : "=r" (hostMcuReceive.frame.receiveStartTimer));	/* Start Timer */
				
                /* Total Frame Length = Header + PayLoad + Checksum + Footer */
                /* (Checksum + Footer) = 2 */
                hostMcuReceive.frame.length += (hostMcuReceive.frame.byteCount + 2);
                if (hostMcuReceive.frame.length > (sizeof(HostMcuRxPacketStructure_t)))
                {
                    memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
                }
            }
            else
            {
                memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
            }
            break;
        default:
             //sprintf(tempPrintStr, "\r\HostMcuInterface_handleUartReceiveData. Default Case");
			                //Debug_TextOut(0, tempPrintStr);
      //  printf("\nHostMcuInterface_handleUartReceiveData. Default Case");
            if ((hostMcuReceive.frame.headerByte) && (hostMcuReceive.frame.byteCount > 11) && (hostMcuReceive.frame.byteCount <= hostMcuReceive.frame.length))
            {
                /* Footer Byte */
				if (hostMcuReceive.frame.byteCount >= hostMcuReceive.frame.length)
                {
					if (HOST_MCU_PROTOCOL_FRAME_FOOTER == rxByte)
                    {
                        /* Complete Frame Received */
						//printf("\r\n%sPacket Received", HOST_MCU_TAG);						
						/*printstrln(" ");
						printstr("HOST MCU: Packet Received.");*/
						printf("\r\n RECIEVED");
						char new;
						for(new = 0; new < hostMcuReceive.frame.byteCount; new++)
						{
							printf(" %02X", hostMcuReceive.data.buffer[new]);
						}
						printf("\r\n");
	
						hostMcuReceive.frame.receiveStartTimer = 0;
						hostMcuReceive.frame.receiveCurrentTimer = 0;
						
						/*if (HOST_MCU_VEHICLE_RTC_INFO_MESSAGE == hostMcuReceive.data.buffer[8])
						{
							HostMcuInterface_parseReceivedPackets();
						}
						else
						{
							hostMcuResponseReceived = 1;
						} */
						if (HOST_MCU_START_OF_FILE_TRANSFER_MESSAGE == hostMcuReceive.data.buffer[8])
						{
							FTP_Ready_Flag_From_MCU = 1;
							FTP_End_Flag_To_MCU = 0;
						}

						if (HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE == hostMcuReceive.data.buffer[8])
						{
							FTP_Ready_Flag_From_MCU = 0;
							FTP_End_Flag_To_MCU = 1;
						}						
						
						hostMcuResponseReceived = 1;
                    }
                    //memset(&hostMcuReceive.frame, 0, sizeof(UartRxFrameParas_t));
                }
            }
            break;
    }
}


#ifndef MODEM_CTRL_LIBG2_FTP_H_
#define MODEM_CTRL_LIBG2_FTP_H_

/*** Include Files ***/

//#include "msp.h"
//#include "../Main Module/LibG2_main.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"

#include "LibG2_debug.h"
#include "modem_ctrl.h"
#include "ff.h"

#include "platform/driver_instances.h"

#include "Camera_Main/jpeg_c_lib_test.h"

#include "Camera_Main/host_mcu/host_mcu_types.h"

/*** Macro Definitions ***/
#define FTP_PACKET_SIZE 1500
#define MAX_FTP_TRANSFER_ATTEMPTS 3	//TBD	//added on 26_11_2021
#define FTP_MAX_RESPONSE_TIMEOUT 250
#define MODEM_AT_CMD_RESPONSE_TIMEOUT 5

#define SECOND_SENSOR_TRIGGER_DETECTED  -6

#define RESTART_FILE_TRANSFER_SESSION -8	//TBD	//added on 26_11_2021

#define APPEND_PACKET_ERROR -9


/*** Type Definitions ***/
typedef enum {
    ERROR_NONE,
    ERROR_NOT_IN_TRANSPARENT_MODE,
    ERROR_MODEM_RESPONSE
} FtpTransferErrorCodes_t;

#ifdef __cplusplus
extern "C" {
#endif


/*** Function Prototypes ***/

uint8_t Ftp_checkModemStatus(void);
uint8_t Ftp_connectServer(void);
uint8_t Ftp_openSession(void);
uint8_t Ftp_closeSession(void);
uint8_t Ftp_enterCommandMode(char *fileName,uint8_t commandMode,uint8_t number);
uint8_t Ftp_sendPacketData(uint8_t packetNumber, uint8_t numPackets, uint8_t *data, uint16_t size);
uint8_t Ftp_sendFile(char *fileName, uint8_t *fileData, uint16_t fileSize);
uint8_t Ftp_activateGprs(void);
void find_image_file(void);

void ftp_ctrl_create( UBaseType_t priority , chanend_t txCmd);

//int SendImageFile_viaFTP(uint8_t instance);
int SendImageFile_viaFTP(uint8_t instance, chanend_t txCmd);

extern ImageInfo_t imageCaptureTile0;

extern void InterTileCommTile0_sendPacketToHostMcuViaTile1(chanend_t txCmd, unsigned char packetType);
//extern unsigned char InterTileCommTile0_waitResponseFromHostMcuViaTile1(chanend txCmd, unsigned char packetType, unsigned char rxStatus);
//extern void InterTileCommTile0_rxInstance(chanend sensorTriggerInstance, chanend imgCaptureDone, chanend jpegDone, chanend txCmd);
//extern void InterTileCommTile1_sendResponse(chanend_t txCmd, unsigned char packetType, unsigned char status, unsigned char *data);
extern int ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(chanend_t txCmd);


/*extern uint8_t mdm_send_AT_command( const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms );
extern uint8_t mdm_find_response( void * response_buf, const char * string_to_find );	
extern uint8_t Ftp_sendATcommand(   const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms );*/

extern uint16_t Modem_out_UART( const char * tx_buf,
                         uint16_t     cmdlen );
#ifdef __cplusplus
}
#endif


#endif /* COMMUNICATION_MODULES_FTP_H_ */

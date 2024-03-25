
#ifndef HOST_MCU_INTERFACE_H_
#define HOST_MCU_INTERFACE_H_

/*** Include Files ***/

//#include "protocol_frame_host_mcu.h"


/*** Macro Definitions ***/

/*** Type Definitions ***/

#ifdef __cplusplus
extern "C" {
#endif


/*** Function Prototypes ***/

void HostMcuInterface_initialize(void);
//void HostMcuInterface_sendMessage(unsigned char packetType);
void HostMcuInterface_sendDataPacket(unsigned char packetType);
void HostMcuInterface_handleUartReceiveData(unsigned char rxByte);
void HostMcuInterface_parseReceivedPackets(void);
unsigned char HostMcuInterface_waitForResponse(void);
unsigned char HostMcuInterface_decrementFrameReceiveTimeoutTimer(void);
void HostMcuInterface_txTaskHandler(unsigned char txState);

//void Tx_Task(void);
//void HostMcuInterface_handleResponseTimeout(void);
//void HostMcuInterface_serviceTasks(void);


#ifdef __cplusplus
}
#endif


#endif /* HOST_MCU_INTERFACE_H_ */

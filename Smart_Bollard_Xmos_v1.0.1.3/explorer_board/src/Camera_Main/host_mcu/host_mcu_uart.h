
#ifndef HOST_MCU_UART_H_
#define HOST_MCU_UART_H_


/*** Include Files ***/

/*** Macro Definitions ***/

/*** Type Definitions ***/

#ifdef __cplusplus
extern "C" {
#endif

/*** Function Prototypes ***/

void HostMcuUart_initialize(void);
void HostMcuUart_sendBuffer(unsigned char* buffer, unsigned short length);
//void HostMcuUart_ReceiveData(chanend c_decoupler);
//void HostMcuUart_txData(void);


#ifdef __cplusplus
}
#endif


#endif /* HOST_MCU_UART_H_ */

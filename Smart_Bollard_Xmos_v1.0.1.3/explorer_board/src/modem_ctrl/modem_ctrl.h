// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef MODEM_CTRL_MODEM_CTRL_H_
#define MODEM_CTRL_MODEM_CTRL_H_

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Library headers */
#include "rtos_support.h"

#include "LibG2_debug.h"
#include <print.h>

#include "uart.h"
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/interrupt_wrappers.h>

#include "LibG2_ftp.h"

void modem_ctrl_create( UBaseType_t priority );

void DelayMs(uint32_t count);
void DelayUs(uint32_t count);

typedef enum
{
    MDM_ERR_NONE = 0,
    MDM_ERR_CMD_FAILED,
    MDM_ERR_SEND_DATA_FAILED,
    MDM_ERR_OPEN_IP_FAILED,
    MDM_ERR_MISC_FAILURE,
    MDM_ERR_UNKNOWN_BAUD,
    MDM_ERR_INIT_FAILED,
    MDM_ERR_UART_SEND_FAILED,
    MDM_ERR_UART_RECV_FAILED,
    MDM_CONNECT,
    MDM_SEND_DATA,
    MDM_NO_CARRIER,
    MDM_ERR_UNKNOWN,
    MDM_FTP_BUSY,
    MDM_FTP_ERROR,
    MDM_ERR_OPEN_USB_FAILED,
    MDM_SECOND_SENSOR_TRIGGER_DETECTED,
    MDM_APPEND_PACKET_FAILED,
	MDM_ERR_COMM_TIMEOUT,
	MDM_ERR_NOT_CONNECTED,
	MDM_RESTART_FILE_TRANSFER_SESSION
} MdmErrCodes;

typedef enum
{
    SOCERR_NONE,
    SOCERR_BUSY,
    SOCERR_OPEN,
    SOCERR_CLOSE,
    SOCERR_SEND_NONE,
    SOCERR_SEND_SOME,
    SOCERR_SEND_ALL,
    SOCERR_SEND_TIMEOUT,
    SOCERR_RECVD_NONE,
    SOCERR_RECVD_SOME,
    SOCERR_RECVD_BUF_FULL,
    SOCERR_RECV_TIMEOUT,
    SOCERR_NOT_BOUND,
    SOCERR_IP_STK_OPEN,
    SOCERR_IP_STK_CLOSE
} SocketError;

uint8_t mdm_send_AT_command( const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms );

/**************************************************************************/
/*  Name        : mdm_find_response                                       */
/*  Parameters  : void *, const char *                                    */
/*  Returns     : int16_t                                                 */
/*  Function    : String Comparison to identify errors in modem response  */
/*------------------------------------------------------------------------*/
uint8_t mdm_find_response( void * response_buf, const char * string_to_find );		

uint8_t Ftp_sendATcommand(   const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms );

uint16_t Modem_out_UART( const char * tx_buf,
                         uint16_t     cmdlen );
						 
uint8_t mdm_send_data( uint8_t * data_p, uint16_t data_sz, uint16_t timeout );
#if ON_TILE(0)
typedef enum {
    MODEM_TXD = 0
} txdUARTSelection;
extern void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c);
extern void MODEM_UART_ReceiveData(void);
extern void MSP_UART_ReceiveData(void);
extern void XMOS_UART_ReceiveData(void);

#endif

#define FALSE	0
#define TRUE	1
#define RX_BUFFER_SIZE									   1500
#define GPRS_GEN_BUF_SZ_VSMALL    					       64
#define GPRS_GEN_BUF_SZ_SMALL    					       128
#define GPRS_GEN_BUF_SZ_MED        					       256
#define GPRS_GEN_BUF_SZ_LARGE    					       512
#define GPRS_GEN_BUF_SZ_VLARGE   					       1500
#define DEFAULT_TIME_OUT         					       5
#define DEFAULT_RETRIES          					       2
extern uint16_t glMdmUart_bytes_recvd;
extern uint8_t  glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern uint16_t glXMOSUart_bytes_recvd;
extern uint8_t  glXMOSUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];

static uint8_t      			glTemp_buf2[GPRS_GEN_BUF_SZ_VSMALL];

#endif /* MODEM_CTRL_H_ */

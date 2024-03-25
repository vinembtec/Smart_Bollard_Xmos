// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#include "modem_ctrl.h"
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/interrupt_wrappers.h>
#include "uart.h"
#include "rtos_osal.h"
#include "rtos_uart_tx.h"

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

extern uint16_t			            glMdmUart_bytes_recvd;
extern uint8_t             			glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
uint8_t   rx_ans[ GPRS_GEN_BUF_SZ_MED ];
uint8_t Modem_No_Response_Count = 0;
TaskHandle_t modem_handler_task = NULL;
uart_tx_t* uart;
/////////////////////////////////////////////////////////////



uint8_t mdm_find_response( void * response_buf, const char * string_to_find )
{
	return ( strstr( (char*) response_buf, string_to_find) == NULL ) ? FALSE : TRUE ;
}


/**************************************************************************/
//! Send data to server
//! \param char * pointer to the data packet to be sent
//! \param uint16_t size of the data packet to be sent
//! \param uint16_t timeout to wait for before declaring a success or failure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_send_data( uint8_t * data_p, uint16_t data_sz, uint16_t timeout )
{
	uint8_t        result = MDM_ERR_NONE;
	uint8_t     i;
	
	////Special_Test_AT_Commands((uint8_t*) data_p,  data_sz);
//	if(CC_Transaction_Retry_Request_Interval)  //@r complete if statement just added for testing
//	{
//		Debug_Output1(0,"mdm_send_data (data_sz) : %d",data_sz);
//		for(i=0;i<data_sz;i++)
//		{
//			Debug_Output2(0,"data_p[%d] : %x",i, *data_p++);
//		}
//		for(i = 0; i<data_sz; i++)
//		{
//			*data_p--;
//		}
//	}

	/*if((mdm_comm_status!=5)||((data_p[0]==0x41)&&(data_p[1]==0x54)))
	{
		//for(i=0; i<data_sz; i++)
		//	Debug_Output1(0, "data:%x", data_p[i]);
		Modem_Test_Flag = FALSE;
		return MDM_ERR_SEND_DATA_FAILED;
	}
	else*/
	{
		memset( glMdmUart_recv_buf, 0, sizeof( glMdmUart_recv_buf ) );
		memset( rx_ans, 0, sizeof( rx_ans ) );
		glMdmUart_bytes_recvd = 0;

		//UART_transmitData_TILE_0(0,'\0');
		//UART_transmitData_TILE_0(0,'\0');
		//rtos_uart_tx_write(uart_tx_ctx, (uint8_t *)'\0', 1);
		DelayMs(2000);    //1710:shruthi

		/*UART_transmitData_TILE_0(0,'+');
		UART_transmitData_TILE_0(0,'+');
		UART_transmitData_TILE_0(0,'+');*/
		
		rtos_uart_tx_write(uart_tx_ctx, data_p, data_sz);
		
		//delay_milliseconds(24);
		//rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//DelayMs(20);
		//delay_milliseconds(24);
		//rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//DelayMs(20);
		//delay_milliseconds(24);
		//rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//delay_milliseconds(24);
		
		//DelayMs(20);    //1710:shruthi

		/*UART_transmitData_TILE_0(0,'+');
		UART_transmitData_TILE_0(0,'+');
		UART_transmitData_TILE_0(0,'+');*/
		
		////rtos_uart_tx_write(uart_tx_ctx, data_p, data_sz);		
		/*
		//delay_milliseconds(24);
		rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//delay_milliseconds(24);
		rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//delay_milliseconds(24);
		rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		//delay_milliseconds(24);
		*/
		
		/*DelayMs(1);
		rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		DelayMs(1);
		rtos_uart_tx_write(uart_tx_ctx, data_p, 1);
		DelayMs(1);*/
		//result = Modem_out_UART( (uint8_t*) data_p, data_sz );

		/*if ( result != MDM_ERR_NONE)
		{
			////Modem_Test_Flag = FALSE;
			return MDM_ERR_SEND_DATA_FAILED;
		}*/

		//sprintf( (char*)glTemp_buf,"Bytes Xmit %d", data_sz );
		//send_diag_text_to_MB( glIn_diag_mode, 3, (const char*) glTemp_buf );

		if (data_sz == 3)    //0509: it is an escape sequence
		{
			for ( i = 0; i < timeout; i++ )    // wait till started to receive response from server
			{
				if ( glMdmUart_bytes_recvd > 0 )    // break when start getting bytes
				{
					Debug_TextOut( 0, "MODEM RESPONSE RECIVED" );
					break;
				}
				DelayMs(100);
			}

			if ( glMdmUart_bytes_recvd == 0 )
			{
				Debug_TextOut( 0, "MODEM RESPONSE NOT RECIVED" );
				////Modem_Test_Flag = FALSE;
				return MDM_ERR_SEND_DATA_FAILED;
			}
		}
		
		Debug_TextOut( 0, (const char*)glMdmUart_recv_buf );
		Debug_Output1( 0, "glMdmUart_bytes_recvd = %d", glMdmUart_bytes_recvd );
		
		////Modem_Test_Flag = FALSE;
		return MDM_ERR_NONE;
	}

}
	
uint8_t mdm_send_AT_command( const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms )		
{
	
	uint16_t   result = MDM_ERR_NONE;
	uint16_t  cmd_lng = 0;
	uint16_t  i,j;
	
	memset ( resp_buf_ptr, 0, resp_buf_sz );
	cmd_lng = strlen(tx_buf);
#if 0
	////if ( Debug_Verbose_Level > 4 )
	 {
	    if((strcmp((char*)glTemp_buf2, (char*)glMdmUart_recv_buf))==1)
	    {
            if ( glMdmUart_bytes_recvd > 0 )
            {
                //Debug_TextOut(0,"glMdmUart_bytes_recvd > 0");
                glMdmUart_recv_buf[glMdmUart_bytes_recvd] = 0;
                //liberty_
				sprintf( (char*) glTemp_buf2, "%s", glMdmUart_recv_buf );
                Debug_TextOut( 0, (const char*) glTemp_buf2 );
            }
	    }
            //Debug_TextOut( 0, (const char*) glMdmUart_recv_buf );
            //Debug_TextOut(0,"glMdmUart_bytes_recvd <============ 0");
            //liberty_
			sprintf( (char*) glTemp_buf2, "%s", tx_buf );
            Debug_TextOut( 0, (const char*) glTemp_buf2 );
	}
#endif

	//printstr("\r\n");
	//rtos_printf("tx_buf = %s , cmd_lng = %d\n", tx_buf,cmd_lng);
	//printstr("\r\n");

	glMdmUart_bytes_recvd = 0;
	memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
	//delay_milliseconds(100);
	
	result = Modem_out_UART( tx_buf,  cmd_lng );
#if 1	
	
	for ( i = 0;  i < max_retries;  ++i )
	{
		delay_milliseconds(100);
		for ( j = 0;  j < timeout_ms;  ++j )    // wait till started to receive response from server
		{
		DelayMs(1);
		if((glMdmUart_bytes_recvd > 1)&&(glMdmUart_recv_buf[0]!='\0')) 
		{
			DelayMs(20);
			break;	//Recieved some data from Modem	
		}
		}
	}
	
	for ( i = 0; i < glMdmUart_bytes_recvd; i++ )
	{
		glMdmUart_recv_buf[i] = glMdmUart_recv_buf[i] & 0x7F;
	}
	
	if(glMdmUart_bytes_recvd==0)result = MDM_ERR_CMD_FAILED;
	
	
	//printint(glMdmUart_bytes_recvd);
	//printstr("\r\n");	
									
	//printstr(glMdmUart_recv_buf);
	//printstr("\r\n");
	
	////if (Debug_Verbose_Level > 4)
		{
			memcpy( glTemp_buf2, glMdmUart_recv_buf, glMdmUart_bytes_recvd );
			glTemp_buf2[glMdmUart_bytes_recvd] = '\0';
			Debug_TextOut( 0, (const char*)glTemp_buf2 );
		}	
		
	if (mdm_find_response (glMdmUart_recv_buf, "ERROR" ) == TRUE )
		result = MDM_ERR_CMD_FAILED;
	else if ( resp_buf_ptr != NULL )
		memcpy ( resp_buf_ptr, glMdmUart_recv_buf, resp_buf_sz );
#endif	
	return result;
}

uint8_t Ftp_sendATcommand( const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms )		
{
	
	uint16_t   result = MDM_ERR_NONE;
	uint16_t  cmd_lng = 0;
	uint16_t  i,j;
	uint8_t reinit_Flag = 0;
	
	//return MDM_ERR_NONE;
	
	memset ( resp_buf_ptr, 0, resp_buf_sz );
	cmd_lng = strlen(tx_buf);
#if 0
	////if ( Debug_Verbose_Level > 4 )
	 {
	    if((strcmp((char*)glTemp_buf2, (char*)glMdmUart_recv_buf))==1)
	    {
            if ( glMdmUart_bytes_recvd > 0 )
            {
                //Debug_TextOut(0,"glMdmUart_bytes_recvd > 0");
                glMdmUart_recv_buf[glMdmUart_bytes_recvd] = 0;
                //liberty_
				sprintf( (char*) glTemp_buf2, "%s", glMdmUart_recv_buf );
                Debug_TextOut( 0, (const char*) glTemp_buf2 );
            }
	    }
            //Debug_TextOut( 0, (const char*) glMdmUart_recv_buf );
            //Debug_TextOut(0,"glMdmUart_bytes_recvd <============ 0");
            //liberty_
			sprintf( (char*) glTemp_buf2, "%s", tx_buf );
            Debug_TextOut( 0, (const char*) glTemp_buf2 );
	}
#endif

	//printstr("\r\n");
	//rtos_printf("tx_buf = %s , cmd_lng = %d\n", tx_buf,cmd_lng);
	//printstr("\r\n");

	//rtos_printf("tx_buf = %s\n", tx_buf);

	glMdmUart_bytes_recvd = 0;
	memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
	delay_milliseconds(10);
	
	result = Modem_out_UART( tx_buf,  cmd_lng );
#if 1	
	
	for ( i = 0;  i < max_retries;  ++i )
	{
		delay_milliseconds(100);
		for ( j = 0;  j < timeout_ms;  ++j )    // wait till started to receive response from server
		{
		DelayMs(1);
		if((glMdmUart_bytes_recvd > 1)&&(glMdmUart_recv_buf[0]!='\0'))
			{
				DelayMs(20);
				if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR: 608" ) == TRUE )
				{
					Debug_TextOut(0, "Modem Already Activated Message Response Received");
					result = MDM_ERR_NONE;
					reinit_Flag = TRUE;
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR" ) == TRUE )
				{
					Debug_TextOut(0, "Modem ERROR Response Received");
					result = MDM_ERR_CMD_FAILED;
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "OK" ) == TRUE )
				{
					Debug_TextOut(0, "Modem OK Response Received");
					result = MDM_ERR_NONE;
					reinit_Flag = TRUE;
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "CONNECT" ) == TRUE )
				{
					Debug_TextOut(0, "Modem CONNECT Response Received");
					result = MDM_ERR_NONE;//MDM_CONNECT;
					reinit_Flag = TRUE;
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "NO CARRIER" ) == TRUE )
				{
					if (mdm_find_response (glMdmUart_recv_buf, "+++" ) == TRUE )
					{
						result = MDM_ERR_NONE;
						reinit_Flag = TRUE;
					}
					else
					{
						result = MDM_NO_CARRIER;
					}
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, ">" ) == TRUE )
				{
					result = MDM_SEND_DATA;
					reinit_Flag = TRUE;
					//delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "#FTPAPPEXT:" ) == TRUE )
				{
					result = MDM_ERR_NONE;
					reinit_Flag = TRUE;
					//Debug_TextOut(0, "FTPAPPEXT RESPONSE");
					//delay_milliseconds(20);
					break;
				}
			}		
		/*if((glMdmUart_bytes_recvd > 1)&&(glMdmUart_recv_buf[0]!='\0')) 
		{
			DelayMs(20);
			break;	//Recieved some data from Modem	
		}*/
		}
	}
	
	for ( i = 0; i < glMdmUart_bytes_recvd; i++ )
	{
		glMdmUart_recv_buf[i] = glMdmUart_recv_buf[i] & 0x7F;
	}
	
	if(glMdmUart_bytes_recvd==0)result = MDM_ERR_CMD_FAILED;
	
	
	//printint(glMdmUart_bytes_recvd);
	//printstr("\r\n");	
									
	//printstr(glMdmUart_recv_buf);
	//printstr("\r\n");
	
	////if (Debug_Verbose_Level > 4)
		{
			memcpy( glTemp_buf2, glMdmUart_recv_buf, glMdmUart_bytes_recvd );
			glTemp_buf2[glMdmUart_bytes_recvd] = '\0';
			Debug_TextOut( 0, (const char*)glTemp_buf2 );
		}	
		
	if ((mdm_find_response (glMdmUart_recv_buf, "ERROR" ) == TRUE ) && (reinit_Flag == FALSE))
		result = MDM_ERR_CMD_FAILED;
	else if ( resp_buf_ptr != NULL )
		memcpy ( resp_buf_ptr, glMdmUart_recv_buf, resp_buf_sz );
#endif	
	return result;
}

uint8_t FTP_mdm_send_AT_command(const char * tx_buf,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms )
{
    uint8_t glTemp_buf2[GPRS_GEN_BUF_SZ_VSMALL];
    uint8_t minimumExpectedData = 6;
    uint8_t result = MDM_ERR_NONE;
    uint16_t cmd_lng = (int16_t) strlen(tx_buf);
    uint16_t i, j;
    uint8_t bytes_compare = 0;
	
    if (0 == strncmp((const char *)tx_buf, "AT#FTPAPPEXT", strlen("AT#FTPAPPEXT")))
    {
        minimumExpectedData = 4;
    }

    memset(resp_buf_ptr, 0, resp_buf_sz);

    /*if (mdm_comm_status == 5)
    {
        return MDM_ERR_CMD_FAILED;    //0510
    }*/

    ////sys_mdm_p.cmd_sent = true;

    //if ( Debug_Verbose_Level > 4 )
    {
        /*if ( glMdmUart_bytes_recvd > 0 )
        {
            glMdmUart_recv_buf[glMdmUart_bytes_recvd] = 0;
            //liberty_sprintf( (char*) glTemp_buf2, "%s", glMdmUart_recv_buf );
            liberty_sprintf( (char*) glTemp_buf2, "%s", &glMdmUart_recv_buf[2]);
            Debug_TextOut( 0, (const char*) glTemp_buf2 );
        }*/

        liberty_sprintf( (char*) glTemp_buf2, "%s", tx_buf );
        Debug_TextOut( 0, (const char*) glTemp_buf2 );
    }

/////////////////////////////////////glMdmUart_bytes_recvd
	//Debug_Output1( 0, "glMdmUart_recv_buf before (%s)", (const char*)glMdmUart_recv_buf);
	memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
	glMdmUart_bytes_recvd = 0;
	delay_milliseconds(200);
	//Debug_Output1( 0, "glMdmUart_recv_buf after (%s)", (const char*)glMdmUart_recv_buf);
	result = Modem_out_UART( (const char*) tx_buf,  cmd_lng );
	
	for ( i = 0;  i < max_retries;  ++i )
	{
		delay_milliseconds(100);
		for ( j = 0;  j < timeout_ms;  ++j )    // wait till started to receive response from server
		{
		DelayMs(1);
		if(glMdmUart_bytes_recvd > 1)
			{
				delay_milliseconds(20);
				if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR: 608" ) == TRUE )
				{
					//Debug_TextOut(0, "Modem Already Activated Message Response Received");
					result = MDM_ERR_NONE;
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR" ) == TRUE )
				{
					//Debug_TextOut(0, "Modem ERROR Response Received");
					result = MDM_ERR_CMD_FAILED;
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "OK" ) == TRUE )
				{
					//Debug_TextOut(0, "Modem OK Response Received");
					result = MDM_ERR_NONE;
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "CONNECT" ) == TRUE )
				{
					//Debug_TextOut(0, "Modem CONNECT Response Received");
					result = MDM_CONNECT;
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "NO CARRIER" ) == TRUE )
				{
					if (mdm_find_response (glMdmUart_recv_buf, "+++" ) == TRUE )
					{
						result = MDM_ERR_NONE;
					}
					else
					{
						result = MDM_NO_CARRIER;
					}
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, ">" ) == TRUE )
				{
					result = MDM_SEND_DATA;
					delay_milliseconds(20);
					break;
				}
				else if (mdm_find_response (glMdmUart_recv_buf, "#FTPAPPEXT:" ) == TRUE )
				{
					result = MDM_ERR_NONE;
					delay_milliseconds(20);
					break;
				}
			}
		}
		delay_milliseconds(100);
	}
#if 0	
	for ( i = 0;  i < max_retries;  ++i )
	//for (j = 0;  j < timeout_ms;  j++)    // wait till started to receive response from server
	{
	delay_milliseconds(100);
		
	//if(glMdmUart_bytes_recvd > 0) break;	//Recieved some data from Modem	
	
        //if (glMdmUart_bytes_recvd >= minimumExpectedData)
	if(glMdmUart_bytes_recvd > 0)
        {
            if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR: 608" ) == TRUE )
            {
                //Debug_TextOut(0, "Modem Already Activated Message Response Received");
                result = MDM_ERR_NONE;
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, "+CME ERROR" ) == TRUE )
            {
                //Debug_TextOut(0, "Modem ERROR Response Received");
                result = MDM_ERR_CMD_FAILED;
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, "OK" ) == TRUE )
            {
                //Debug_TextOut(0, "Modem OK Response Received");
                result = MDM_ERR_NONE;
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, "CONNECT" ) == TRUE )
            {
                //Debug_TextOut(0, "Modem CONNECT Response Received");
                result = MDM_CONNECT;
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, "NO CARRIER" ) == TRUE )
            {
                if (mdm_find_response (glMdmUart_recv_buf, "+++" ) == TRUE )
                {
                    result = MDM_ERR_NONE;
                }
                else
                {
                    result = MDM_NO_CARRIER;
                }
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, ">" ) == TRUE )
            {
                result = MDM_SEND_DATA;
                break;
            }
            else if (mdm_find_response (glMdmUart_recv_buf, "#FTPAPPEXT:" ) == TRUE )
            {
                result = MDM_ERR_NONE;
                break;
            }
        }	
	}
#endif
////////////////////////////////////
#if 1
    //if (j < timeout_ms)
    {
        Debug_TextOut(0, " ");
        Debug_Display(0, "MODEM RX: ");
        memcpy( glTemp_buf2, glMdmUart_recv_buf, glMdmUart_bytes_recvd );
        glTemp_buf2[glMdmUart_bytes_recvd] = '\0';
        Debug_TextOut(0, (const char*)&glTemp_buf2[2]);
        /*for (i = 0; i < glMdmUart_bytes_recvd; i++ )
        {
            Debug_Output1(0, "%02X ", glTemp_buf2[i]);
        }*/
        //LogManager_printByteArrayHexString(glMdmUart_recv_buf, glMdmUart_bytes_recvd);
    }
#endif

    for ( i = 0; i < glMdmUart_bytes_recvd; i++ )
    {
        glMdmUart_recv_buf[i] = glMdmUart_recv_buf[i] & 0x7F;
    }

    if ( glMdmUart_bytes_recvd <= 0 )
    {
		Modem_No_Response_Count++;
        result = MDM_ERR_CMD_FAILED;
        Debug_TextOut( 0, "AT FAIL" );
    }
    else
    {
		Modem_No_Response_Count = 0;
        Debug_TextOut( 0, "AT OK" );
        //if (Debug_Verbose_Level > 4)
        {
            //memcpy( glTemp_buf2, glMdmUart_recv_buf, glMdmUart_bytes_recvd );
            memcpy( glTemp_buf2, &glMdmUart_recv_buf[2], (glMdmUart_bytes_recvd-2) );
            glTemp_buf2[glMdmUart_bytes_recvd] = '\0';
            Debug_TextOut( 0, (const char*)glTemp_buf2 );
        }

        if ( resp_buf_ptr != NULL )
            memcpy ( resp_buf_ptr, glMdmUart_recv_buf, resp_buf_sz );
    }
	if(Modem_No_Response_Count >= 4)
	        {
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;
				Modem_No_Response_Count = 0;
	        }
    ////sys_mdm_p.cmd_sent = false;

    return result;
}

uint16_t Modem_out_UART( const char * tx_buf,
                             uint16_t     cmdlen )
{
	uint8_t   result = MDM_ERR_NONE;
	////uint8_t tx_buf_new[1500];
	uint16_t cmdlen_new = cmdlen;
	
	////memcpy( tx_buf_new, tx_buf, cmdlen );
	////xSemaphoreTake(uart_mutex, portMAX_DELAY);
	//memcpy( modemTXD, tx_buf, cmdlen );
	uint16_t modem_uart_bytes_counter;
	
	/*if(cmdlen > 1000)
	{
	for(int kt = 0; kt <= 12; kt++)
	{
		printhex(tx_buf[kt]);
		printstr(" ");
	}
	printstr("\r\n");
	}*/
	
	rtos_uart_tx_write(uart_tx_ctx, tx_buf, cmdlen);
	#if 0
	for(modem_uart_bytes_counter=0; modem_uart_bytes_counter<cmdlen_new; modem_uart_bytes_counter++)
	{
		#if ON_TILE(0)
		//uart_tx(&uart, tx_buf[modem_uart_bytes_counter]);
		//int state = rtos_osal_critical_enter();
		{
			rtos_uart_tx_write(uart_tx_ctx, data, size);
			//UART_transmitData_TILE_0(MODEM_TXD, tx_buf_new[modem_uart_bytes_counter]);
		}
		//rtos_osal_critical_exit(state);
		#endif
		//DelayUs(1);
		//vTaskDelay(pdMS_TO_TICKS(5000));
	    //UART_transmitData(MODEM_TXD, tx_buf[modem_uart_bytes_counter]);
	}
	#endif
	////xSemaphoreGive(uart_mutex);
	//DelayMs(1);	
	return SOCERR_NONE;	
}

////////////////////////////////////////////////////////

volatile int uart_working = 1;

HIL_UART_TX_CALLBACK_ATTR void uart_callback(uart_callback_code_t callback_info){
    //debug_printf("ISR callback 0x%x\n", callback_info);
    uart_working = 0;
}
/*
DEFINE_INTERRUPT_PERMITTED(uart_isr_grp, void, uart_demo, uart_tx_t* uart)
{
    char tx_msg[] = "\x55\0\xaa"; //U = 0x55
    // char tx_msg[] = {0x55};

    port_t p_uart_tx = XS1_PORT_1F;//WIFI_CLK;
    hwtimer_t tmr = hwtimer_alloc();
    // tmr = 0;

    char tx_buff[64];

    uart_tx_init(uart, p_uart_tx, 115200, 8, UART_PARITY_NONE, 1,
                tmr, tx_buff, sizeof(tx_buff), uart_callback);

    uart_tx_blocking_init(uart, p_uart_tx, 921600, 8, UART_PARITY_NONE, 1, tmr);
    uart_tx_blocking_init(uart, p_uart_tx, 1843200, 8, UART_PARITY_NONE, 1, tmr);
    uart_tx_blocking_init(uart, p_uart_tx, 2764800, 8, UART_PARITY_NONE, 1, tmr);


    for(int i=0; i<sizeof(tx_msg); i++){
        uart_tx(uart, tx_msg[i]);
        debug_printf("uart sent 0x%x (%c)\n", tx_msg[i], tx_msg[i]);

    }
    
    while(uart_working);
    exit(0);
}*/

extern void HostMcuUart_sendData(unsigned char uartPort, unsigned char txByte);
static void modem_ctrl_t0(void *arg)
{
	//rtos_uart_tx_start(uart_tx_ctx);
    char tx_msg[1];
	tx_msg[0] = 'A';
    //uint8_t tx_buff[MAX_TEST_VECT_SIZE];

    
	uint8_t  result = 0;
	//soc_peripheral_t dev = arg;
	//char tx_msg[] = "\x55\0\xaa"; //U = 0x55

    //char tx_buff[1500];
	//port_t p_uart_tx = XS1_PORT_1F;
	//hwtimer_t tmr = hwtimer_alloc();
	
	
	//hwtimer_free(tmr);
    //uart_tx_init(&uart, p_uart_tx, 460800, 8, UART_PARITY_NONE, 1,
    //            tmr, tx_buff, sizeof(tx_buff), uart_callback);

    //uart_tx_blocking_init(&uart, p_uart_tx, 460800, 8, UART_PARITY_NONE, 1, tmr);
	//uart_tx_blocking_init(uart, p_uart_tx, 921600, 8, UART_PARITY_NONE, 1, tmr); 460800
    //uart_tx_blocking_init(uart, p_uart_tx, 1843200, 8, UART_PARITY_NONE, 1, tmr);
    //uart_tx_blocking_init(uart, p_uart_tx, 2764800, 8, UART_PARITY_NONE, 1, tmr);
	
	/*for(int i=0; i<sizeof(tx_msg); i++){
        uart_tx(&uart, tx_msg[i]);
        debug_printf("uart sent 0x%x (%c)\n", tx_msg[i], tx_msg[i]);

    }*/
	//SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
	DelayMs(2000);
	//FileOpen("IMG_FL_8042_32092_20220101_100000_01.JPEG",1);
	
	rtos_printf("modem_ctrl_t0 task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
	for(;;)
	{
		//uart_tx(&uart, 0xAA);
		//DelayMs(2000);
		//UART_transmitData_TILE_0(0, 0x55);
		////DelayMs(2000);
		//find_image_file();
		////result = Ftp_sendATcommand( "AT+COPS?\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//DelayMs(500);
		/*printint(result);
		printstr("\r\n");
		printstr(rx_ans);
		printstr("\r\n");*/
		
		
		////result = Ftp_sendATcommand( "AT#RFSTS\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//DelayMs(500);
		/*printint(result);
		printstr("\r\n");
		printstr(rx_ans);
		printstr("\r\n");*/
		
		////result = Ftp_sendATcommand( "AT#SERVINFO\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//DelayMs(500);
		/*printint(result);
		printstr("\r\n");
		printstr(rx_ans);
		printstr("\r\n");*/
		
		////result = Ftp_sendATcommand( "AT#MONI\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//DelayMs(500);
		/*printint(result);
		printstr("\r\n");
		printstr(rx_ans);
		printstr("\r\n");*/	
		//result = Ftp_sendATcommand( "AT#ABCDEFGHIJKLMNOPQRSTUVWXYZ\r", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		
		for(int v=0;v<1500;v++)
		{
			//hwtimer_set_trigger_time(tmr,0);
			//hwtimer_wait_until(tmr,100000);
			//hwtimer_delay(tmr,10000);
			//rtos_printf("11hwtimer_get_trigger_time(tmr): %ld\n", hwtimer_get_trigger_time(tmr));
			//delay_microseconds(500);
			//int state = rtos_osal_critical_enter();
			{
				//tx_msg[0] = 'A';
				//rtos_uart_tx_write(uart_tx_ctx, tx_msg, 1);
				//HostMcuUart_sendData(0, 'A');
				//UART_transmitData_TILE_0(0, 'A');
			}			
			//rtos_osal_critical_exit(state);
			//rtos_printf("22hwtimer_get_trigger_time(tmr): %ld\n", hwtimer_get_trigger_time(tmr));
			//DelayMs(1);
			//uart_working = 1;
			//uart_tx(&uart, 'A');
			//while(uart_working);
			//sleep_until_next_transition(&uart);
		}
		//UART_transmitData_TILE_0(0, '\r');
		//UART_transmitData_TILE_0(0, '\n');
		//result = Ftp_sendATcommand( "AT#SERVINFO\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		////DelayMs(1000);
		
		rtos_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		rtos_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

		debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
		debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		
		//DelayMs(5000);
		//FileOpen("xmos.jpg",1);
		//FileOpen("IMG_FL_8042_32092_20220101_100000_01.jpeg",1);
		DelayMs(5000);
	}
}


void modem_ctrl_create( UBaseType_t priority )
{
	
	xTaskCreate(modem_ctrl_t0, "modem_ctrl_t0", portTASK_STACK_DEPTH(modem_ctrl_t0), NULL, tskIDLE_PRIORITY, &modem_handler_task);			
	//xTaskCreate(modem_ctrl_t0, "modem_ctrl_t0", portTASK_STACK_DEPTH(modem_ctrl_t0), dev, priority, &modem_handler_task);	 //tskIDLE_PRIORITY		
}

void DelayMs(uint32_t count)
{
	vTaskDelay(pdMS_TO_TICKS(count));
}

void DelayUs(uint32_t count)
{
	vTaskDelay(pdMS_TO_TICKS(count));
}

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <platform.h>
#include <stdint.h>
#include <timer.h>
#include <xmos_flash.h>
#include <print.h>
#include <stdio.h>

#define BIT_RATE_MODEM 115200//460800//115200//921600//115200
#define BIT_TIME_MODEM XS1_TIMER_HZ / BIT_RATE_MODEM

/*UART Config*/
#define FALSE 0
#define TRUE  1
#define MODEM_BAUD_RATE 		115200//921600//115200//230400//460800//921600
#define MSP_BAUD_RATE 			115200
#define XMOS_DEBUG_BAUD_RATE 	115200


#if ON_TILE(0)
uint8_t Modem_TX_Busy = 0;
typedef enum {
    MODEM_TXD = 0
} txdUARTSelection;
void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c);
void MODEM_UART_ReceiveData(void);
void MSP_UART_ReceiveData(void);
void XMOS_UART_ReceiveData(void);

// TILE 0 PORT CONFIGURATION
in port  UART_4BIT_RXD = on tile[0] : XS1_PORT_4C; // X0D14/X0D15/X0D20/X0D21
//in port  RXD = on tile[0] : XS1_PORT_1E;	//X0D12
//out port TXD = on tile[0] : XS1_PORT_1F;	//X0D13

//on tile[0]:out port RXD = XS1_PORT_1E;	//X1D00 (X1D03->old)
//on tile[0]:in port TXD = XS1_PORT_1F;	//X1D10 (X0D15->old)

in  port RXD_TILE0                     = RXD_PORT_TILE0;
out port TXD_TILE0                     = TXD_PORT_TILE0;

/*UART Config*/
#define RX_BUFFER_SIZE									   1500
#define GPRS_GEN_BUF_SZ_VSMALL    					       64
#define GPRS_GEN_BUF_SZ_SMALL    					       128
#define GPRS_GEN_BUF_SZ_MED        					       256
#define GPRS_GEN_BUF_SZ_LARGE    					       512
#define GPRS_GEN_BUF_SZ_VLARGE   					       1500
#define DEFAULT_TIME_OUT         					       5
#define DEFAULT_RETRIES          					       2

#if 1
void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c)
{
#if 0
	int clocks = XS1_TIMER_HZ / MODEM_BAUD_RATE;
    int dt2 = (clocks * 3)>>1; //one and a half bit times
    int dt = clocks;
    int t;
	unsigned char b;
	unsigned char v = 1;
	while (v)
	{
			clocks = XS1_TIMER_HZ / MSP_BAUD_RATE;
			b = c;
			//pos = 1;
			//current_val = 0xFF;
			//current_val &= ~(1 << pos);
			TXD <: 0 @ t; //send start bit and timestamp (grab port timer value)
			t += clocks;
			#pragma loop unroll(8)
			for(int i = 0; i < 8; i++){
				//TXD @ t <: >> b; //timed output with post right shift
				//current_val &= ~(1 << pos);
				//current_val |= ((b & 1) << pos);
				////TXD @ t <: b;
				////b >>= 1;
				////t += clocks;
				TXD @ t <: >> b; //timed output with post right shift
				t += clocks;
			}			
			//current_val &= ~(1 << pos);
			TXD @ t <: 1; //send stop bit
			t += clocks;
			//current_val |= ((1) << pos);
			TXD @ t <: 1; //wait until end of stop bit	
			v = 0;
	}
#endif

#if 0
	int clocks = 0;
	int t;
    unsigned char b;
	unsigned char current_val = 0;
	unsigned char pos = 0;	
	//switch(n)
	//{
		//case XMOS_DEBUG_TXD:
			clocks = XS1_TIMER_HZ / MODEM_BAUD_RATE;
			b = c;
			pos = 0;
			//current_val = 0xFF;
			current_val &= ~(1 << pos);
			TXD <: current_val @ t; //send start bit and timestamp (grab port timer value)
			t += clocks;
			#pragma loop unroll(8)
			for(int i = 0; i < 8; i++){
				//TXD @ t <: >> b; //timed output with post right shift
				current_val &= ~(1 << pos);
				current_val |= ((b & 1) << pos);
				TXD @ t <: current_val;
				b >>= 1;
				t += clocks;
			}			
			current_val &= ~(1 << pos);
			TXD @ t <: current_val; //send stop bit
			t += clocks;
			current_val |= ((1) << pos);
			TXD @ t <: current_val; //wait until end of stop bit		
		//break;
#endif
			
#if 0
	//int clocks = BIT_TIME_MODEM;//XS1_TIMER_HZ / MODEM_BAUD_RATE;
    //int dt2 = (clocks * 3)>>1; //one and a half bit times
    //int dt = clocks;
    unsigned int t;
    unsigned char b;
	//unsigned char current_val = 0;
	//unsigned char pos = 0;	
	//switch(n)
	//{
		//case MODEM_TXD:
			//clocks = XS1_TIMER_HZ / MODEM_BAUD_RATE;
			//Modem_TX_Busy = TRUE;
			//TXD when pinseq(0) :> int _ @ t;
			b = c;
			TXD <: 0 @ t; //send start bit and timestamp (grab port timer value)
			t += BIT_TIME_MODEM;
			#pragma loop unroll(8)
			for(int i = 0; i < 8; i++) {
				TXD @ t <: >> b; //timed output with post right shift
				t += BIT_TIME_MODEM;//dt;	
			}
			TXD @ t <: 1; //send stop bit
			t += BIT_TIME_MODEM;//(clocks + dt2);
			TXD @ t <: 1; //wait until end of stop bit	
			//t += (BIT_TIME_MODEM *5);//(clocks + dt2);
			//TXD @ t <: 1; //wait until end of stop bit	
		//break;	
	//}
#endif

#if 1
	//output_gpio_if p_txd = TXD;
	//p_txd.output(1);
	timer tmr;
    int t,k;
	unsigned char v = 1;
	while(v)
	{	  
	  TXD_TILE0 <: 1;
	  tmr :> t;
      t += BIT_TIME_MODEM * 5;
	  tmr when timerafter(t) :> int _;
	  TXD_TILE0 <: 1;
	  
	  // Output start bit
      //p_txd.output(0);
	  TXD_TILE0 <: 0;
      tmr :> t;
      t += BIT_TIME_MODEM;
      unsigned byte = c;
      // Output data bits
	  #pragma loop unroll(8)
      for (int j = 0; j < 8/*bits_per_byte*/; j++) {
        tmr when timerafter(t) :> int _;
        //p_txd.output(byte & 1);
		TXD_TILE0 <: (byte & 0x01);
        byte >>= 1;
        t += BIT_TIME_MODEM;
      }
      // Output parity
      /*if (parity != UART_PARITY_NONE) {
        tmr when timerafter(t) :> void;
        p_txd.output(parity32(data, parity));
        t += bit_time;
      }*/
      // Output stop bits
      tmr when timerafter(t) :> int _;
	  TXD_TILE0 <: 1;
      t += BIT_TIME_MODEM;
      tmr when timerafter(t) :> int _;
	  TXD_TILE0 <: 1;
	  v = 0;
	}
#endif

}
#endif

unsafe
{
uint16_t glMdmUart_bytes_recvd = 0;
uint8_t  glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
uint16_t glMSPUart_bytes_recvd = 0;
uint8_t  glMSPUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
uint16_t glXMOSUart_bytes_recvd = 0;
uint8_t  glXMOSUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
}

void MODEM_UART_ReceiveData(void) {
	int clocks = XS1_TIMER_HZ / MODEM_BAUD_RATE;
    int dt2 = (clocks * 3)>>1; //one and a half bit times
    int dt = clocks;
    int t;
    unsigned int data = 0;
	printf("MODEM_UART_ReceiveData \n");
	//rtos_printf("MODEM_UART_ReceiveData task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
    while (1) {
        (RXD_TILE0) when pinseq(0) :> int _ @ t; //wait until falling edge of start bit
        t += dt2;
#pragma loop unroll(8)
        for(int i = 0; i < 8; i++) {
			(RXD_TILE0) @ t :> >> data; //sample value when port timer = t
            					//inlcudes post right shift				
            t += dt;
        }
        data >>= 24;			//shift into MSB
        //cOut <: (unsigned char) data; //send to client
        (RXD_TILE0) @ t :> int _;
		
		if((glMdmUart_bytes_recvd >= GPRS_GEN_BUF_SZ_VLARGE)||(glMdmUart_recv_buf[0]=='\0')) //IMP STEP
			glMdmUart_bytes_recvd = 0;				
		glMdmUart_recv_buf[glMdmUart_bytes_recvd] = (unsigned char)data;
		glMdmUart_bytes_recvd++;		
        data = 0;	
    }
}


#if 1
void XMOS_UART_ReceiveData(void) {
	int clocks = XS1_TIMER_HZ / XMOS_DEBUG_BAUD_RATE;
    int dt2 = (clocks * 3)>>1; //one and a half bit times
    int dt = clocks;
    int t;
    unsigned int data = 0;
	unsigned pos = 0;
	unsigned int UART_4BIT_RXD_TMP = 0;
	printf("XMOS_UART_ReceiveData \n"); //pinseq_at(unsigned val, unsigned time); pin :> current
	//rtos_printf("XMOS_UART_ReceiveData task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
unsafe
{
    while (1) {
UART_4BIT_RXD :> UART_4BIT_RXD_TMP;
printf("XMOS UART_4BIT_RXD_TMP = %ld \n",UART_4BIT_RXD_TMP);
(UART_4BIT_RXD) when pinseq((unsigned int)UART_4BIT_RXD_TMP & (~(1<< pos))) :> int _ @ t;
//printf("XMOS_UART_ReceiveData \n");
UART_4BIT_RXD :> UART_4BIT_RXD_TMP;
        t += dt2;
#pragma loop unroll(8)
        for(int i = 0; i < 8; i++) {
			(UART_4BIT_RXD) @ t :> >> data;// >> data; //sample value when port timer = t
            					    //inlcudes post right shift		 (unsigned int)MSP_RXD & 1				
            t += dt;
        }
		data >>= 0;
		data = ((data)&1)|
		(((data>>4)&1)<<1)|
		(((data>>8)&1)<<2)|
		(((data>>12)&1)<<3)|
		(((data>>16)&1)<<4)|
		(((data>>20)&1)<<5)|
		(((data>>24)&1)<<6)|
		(((data>>28)&1)<<7);
		
        (UART_4BIT_RXD) @ t :> int _;
		
		if((glXMOSUart_bytes_recvd >= GPRS_GEN_BUF_SZ_VLARGE)||(glXMOSUart_recv_buf[0]=='\0')) //IMP STEP
			glXMOSUart_bytes_recvd = 0;				
		glXMOSUart_recv_buf[glXMOSUart_bytes_recvd] = (unsigned char)data;
		glXMOSUart_bytes_recvd++;	
        data = 0;	
    }
	}	
}
#endif

#if 1
void MSP_UART_ReceiveData(void) {
	int clocks = XS1_TIMER_HZ / MSP_BAUD_RATE;
    int dt2 = (clocks * 3)>>1; //one and a half bit times
    int dt = clocks;
    int t;
    unsigned int data = 0;
	unsigned pos = 1;
	unsigned int UART_4BIT_RXD_TMP = 0;
	printf("MSP_UART_ReceiveData \n"); //pinseq_at(unsigned val, unsigned time); pin :> current
	//rtos_printf("MSP_UART_ReceiveData task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
unsafe
{
    while (1) {
UART_4BIT_RXD :> UART_4BIT_RXD_TMP;
//printf("MSP UART_4BIT_RXD_TMP = %ld \n",UART_4BIT_RXD_TMP);
(UART_4BIT_RXD) when pinseq((unsigned int)UART_4BIT_RXD_TMP & (~(1<< pos))) :> int _ @ t;
//printf("MSP_UART_ReceiveData \n");
UART_4BIT_RXD :> UART_4BIT_RXD_TMP;
        t += dt2;
#pragma loop unroll(8)
        for(int i = 0; i < 8; i++) {
			(UART_4BIT_RXD) @ t :> >> data;// >> data; //sample value when port timer = t
            					    //inlcudes post right shift		 (unsigned int)MSP_RXD & 1				
            t += dt;
        }
		data >>= 1;
		data = ((data)&1)|
		(((data>>4)&1)<<1)|
		(((data>>8)&1)<<2)|
		(((data>>12)&1)<<3)|
		(((data>>16)&1)<<4)|
		(((data>>20)&1)<<5)|
		(((data>>24)&1)<<6)|
		(((data>>28)&1)<<7);
		
        (UART_4BIT_RXD) @ t :> int _;
		
		if((glMSPUart_bytes_recvd >= GPRS_GEN_BUF_SZ_VLARGE)||(glMSPUart_recv_buf[0]=='\0')) //IMP STEP
			glMSPUart_bytes_recvd = 0;				
		glMSPUart_recv_buf[glMSPUart_bytes_recvd] = (unsigned char)data;
		glMSPUart_bytes_recvd++;	
        data = 0;	
    }
	}	
}
#endif
#endif
//////////////////////////////////////////////////////////////////
#if ON_TILE(1)
typedef enum {
	MSP_TXD = 1,
    XMOS_DEBUG_TXD
} txdUARTSelection;
void UART_transmitData_TILE_1(txdUARTSelection n,char c);

// TILE 1 PORT CONFIGURATION
////port UART_4BIT_TXD = on tile[1] : XS1_PORT_4A; //  X0D14/X0D15/X0D20/X0D21

in  port RXD_TILE1                     = RXD_PORT_TILE1;
out port TXD_TILE1                     = TXD_PORT_TILE1;

#if 1
void UART_transmitData_TILE_1(txdUARTSelection n,unsigned char c)
{
#if 0	
	int clocks = 0;
	int t;
    unsigned char b;
	unsigned char current_val = 0;
	unsigned char pos = 0;	
	switch(n)
	{
		case XMOS_DEBUG_TXD:
			clocks = XS1_TIMER_HZ / XMOS_DEBUG_BAUD_RATE;
			b = c;
			pos = 0;
			current_val = 0xFF;
			current_val &= ~(1 << pos);
			UART_4BIT_TXD <: current_val @ t; //send start bit and timestamp (grab port timer value)
			t += clocks;
			#pragma loop unroll(8)
			for(int i = 0; i < 8; i++){
				//TXD @ t <: >> b; //timed output with post right shift
				current_val &= ~(1 << pos);
				current_val |= ((b & 1) << pos);
				UART_4BIT_TXD @ t <: current_val;
				b >>= 1;
				t += clocks;
			}			
			current_val &= ~(1 << pos);
			UART_4BIT_TXD @ t <: current_val; //send stop bit
			t += clocks;
			current_val |= ((1) << pos);
			UART_4BIT_TXD @ t <: current_val; //wait until end of stop bit		
		break;
		
		case MSP_TXD:
			clocks = XS1_TIMER_HZ / MSP_BAUD_RATE;
			b = c;
			pos = 1;
			current_val = 0xFF;
			current_val &= ~(1 << pos);
			UART_4BIT_TXD <: current_val @ t; //send start bit and timestamp (grab port timer value)
			t += clocks;
			#pragma loop unroll(8)
			for(int i = 0; i < 8; i++){
				//TXD @ t <: >> b; //timed output with post right shift
				current_val &= ~(1 << pos);
				current_val |= ((b & 1) << pos);
				UART_4BIT_TXD @ t <: current_val;
				b >>= 1;
				t += clocks;
			}			
			current_val &= ~(1 << pos);
			UART_4BIT_TXD @ t <: current_val; //send stop bit
			t += clocks;
			current_val |= ((1) << pos);
			UART_4BIT_TXD @ t <: current_val; //wait until end of stop bit		
		break;
		
	}
#endif

#if 0 //V1.3
	//output_gpio_if p_txd = TXD;
	//p_txd.output(1);
	int clocks = 0;
	//int t;
    unsigned char b;
	unsigned char current_val = 0;
	unsigned char pos = 0;	
	clocks = XS1_TIMER_HZ / XMOS_DEBUG_BAUD_RATE;
	
	timer tmr;
    int t,k;
	unsigned char v = 1;
	while(v)
	{	  
	  UART_4BIT_TXD <: 1;
	  tmr :> t;
      t += clocks * 5;
	  tmr when timerafter(t) :> int _;
	  UART_4BIT_TXD <: 1;
	  
	  // Output start bit
      //p_txd.output(0);
	  UART_4BIT_TXD <: 0;
      tmr :> t;
      t += clocks;
      unsigned byte = c;
      // Output data bits
	  #pragma loop unroll(8)
      for (int j = 0; j < 8/*bits_per_byte*/; j++) {
        tmr when timerafter(t) :> int _;
        //p_txd.output(byte & 1);
		UART_4BIT_TXD <: (byte & 0x01);
        byte >>= 1;
        t += clocks;
      }
      // Output parity
      /*if (parity != UART_PARITY_NONE) {
        tmr when timerafter(t) :> void;
        p_txd.output(parity32(data, parity));
        t += bit_time;
      }*/
      // Output stop bits
      tmr when timerafter(t) :> int _;
	  UART_4BIT_TXD <: 1;
      t += clocks;
      tmr when timerafter(t) :> int _;
	  UART_4BIT_TXD <: 1;
	  v = 0;
	}
#endif

#if 1 //V1.4
	//output_gpio_if p_txd = TXD;
	//p_txd.output(1);
	int clocks = 0;
	//int t;
    unsigned char b;
	unsigned char current_val = 0;
	unsigned char pos = 0;	
	clocks = XS1_TIMER_HZ / XMOS_DEBUG_BAUD_RATE;
	
	timer tmr;
    int t,k;
	unsigned char v = 1;
	while(v)
	{	  
	  TXD_TILE1 <: 1;
	  tmr :> t;
      t += clocks * 5;
	  tmr when timerafter(t) :> int _;
	  TXD_TILE1 <: 1;
	  
	  // Output start bit
      //p_txd.output(0);
	  TXD_TILE1 <: 0;
      tmr :> t;
      t += clocks;
      unsigned byte = c;
      // Output data bits
	  #pragma loop unroll(8)
      for (int j = 0; j < 8/*bits_per_byte*/; j++) {
        tmr when timerafter(t) :> int _;
        //p_txd.output(byte & 1);
		TXD_TILE1 <: (byte & 0x01);
        byte >>= 1;
        t += clocks;
      }
      // Output parity
      /*if (parity != UART_PARITY_NONE) {
        tmr when timerafter(t) :> void;
        p_txd.output(parity32(data, parity));
        t += bit_time;
      }*/
      // Output stop bits
      tmr when timerafter(t) :> int _;
	  TXD_TILE1 <: 1;
      t += clocks;
      tmr when timerafter(t) :> int _;
	  TXD_TILE1 <: 1;
	  v = 0;
	}
#endif
}
#endif
#endif
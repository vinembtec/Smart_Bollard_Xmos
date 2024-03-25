//// Copyright 2019-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "fs_support.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "mem_analysis/mem_analysis.h"
//#include "example_pipeline/example_pipeline.h"
#include "filesystem/filesystem_demo.h"
//#include "gpio_ctrl/gpio_ctrl.h"
//#include "uart/uart_demo.h"

#include "Camera_Main/Camera_Main.h"
#include "rtos_intertile.h"

#include "modem_ctrl/modem_ctrl.h"
#include "modem_ctrl/LibG2_debug.h"
#include "modem_ctrl/LibG2_ftp.h"

#include "uart.h"

#include "Camera_Main/jpeg-6b/jmorecfg.h"

#include "Camera_Main/host_mcu/host_mcu_types.h"

//#include "Camera_Main/intertile_comm/intertile_comm_tile0.h"

#include <xcore/channel.h>
#include <xcore/parallel.h>
#include <stdlib.h>
#include <string.h>

//#define FROM_RAM

////extern void camera_main(chanend_t txCmd);
//extern void capture_thread(chanend_t tile0TxCmd);
//extern void capture_thread(chanend_t tile0TxCmd, chanend_t instance[]);
extern void capture_thread(chanend_t instance[]);
extern void InterTileCommTile0_rxInstance(chanend_t instance[]);



#if ON_TILE(1)
#define MAX_IMG_ROWS (1088)
#define MAX_IMG_COLS (1928)
#define FULL_IMAGE_SIZE (MAX_IMG_ROWS * MAX_IMG_COLS)
#define NUM_IMAGE_BLOCKS 544//136
#define IMAGE_BLOCK_SIZE (FULL_IMAGE_SIZE / NUM_IMAGE_BLOCKS)
#define NUM_ROWS_PER_IMAGE_BLOCK (IMAGE_BLOCK_SIZE / MAX_IMG_COLS)
//__attribute__((section(".ExtMem_data"))) int x[2];	//added on 24_03_2022 
//__attribute__((section(".ExtMem_data"))) JSAMPLE imageBuffer[JSAMPLE imageBuffer[IMAGE_BLOCK_SIZE * 3];	/* Points to large array of R,G,B-order data */
//__attribute__((section(".ExtMem_data"))) unsigned short rawBuffer[IMAGE_BLOCK_SIZE * 3];
#endif

#if 0
#if ON_TILE(0)
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif
#endif

#if 0
#if ON_TILE(1)
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif
#endif

#if 1
#if ON_TILE(1)
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif
#endif

#if ON_TILE(0)
//unsigned char tile0VehicleOccupancyPacketBuffer[101];
//extern int ImageDeletion_processVehicleOccupancyStatusInfo(uint8_t *buffer, uint16_t length);
/*typedef enum {
    MODEM_TXD = 0
} txdUARTSelection;*/
extern void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c);
extern void MODEM_UART_ReceiveData(void);
extern void MSP_UART_ReceiveData(void);
extern void XMOS_UART_ReceiveData(void);
#ifndef FROM_RAM
extern int shared_flash_write();
extern int sf_write_test();
#endif

#define GPRS_GEN_BUF_SZ_VLARGE   					       1500
extern uint16_t glMdmUart_bytes_recvd;
extern uint8_t  glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern uint16_t glMSPUart_bytes_recvd;
extern uint8_t  glMSPUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern uint16_t glXMOSUart_bytes_recvd;
extern uint8_t  glXMOSUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
#endif

#if ON_TILE(1)
typedef enum {
	MSP_TXD = 1,
    XMOS_DEBUG_TXD
} txdUARTSelection;
extern void UART_transmitData_TILE_1(txdUARTSelection n,char c);
#endif

#define PORT_LEDS_X1D4C0 0
#define PORT_LEDS_X1D4C1 1
#define PORT_LEDS_X1D4C2 2
#define PORT_LEDS_X1D4C3 3

void setGpio(uint8_t position);
void resetGpio(uint8_t position);
int readGpio(void);

#if 0
/*VERSION V1.3*/
int readGpio(void)
{
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4A);
	return tempGpio;
}

void setGpio(uint8_t position)
{
	return;
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4C);
	tempGpio |= (1<<0);
	tempGpio |= (1<<3);
	port_out(XS1_PORT_4C, tempGpio);
}

void resetGpio(uint8_t position)
{
	return;
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4C);
	tempGpio &= ~(1<<0);
	tempGpio &= ~(1<<3);
	port_out(XS1_PORT_4C, tempGpio);
}
#endif

#if 1
/*VERSION V1.4 AND GREATER*/
int readGpio(void)
{
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4C);
	return tempGpio;
}

void setGpio(uint8_t position)
{
	////return;
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4C);
	tempGpio |= (1<<position);
	port_out(XS1_PORT_4C, tempGpio);
}

void resetGpio(uint8_t position)
{
	////return;
	int tempGpio;
	tempGpio = port_in(XS1_PORT_4C);
	tempGpio &= ~(1<<position);
	port_out(XS1_PORT_4C, tempGpio);
}
#endif

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    //for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
    //configASSERT(0);
}

void vApplicationIdleHook( void )
{
}

/*
#if ON_TILE(0)
unsigned char InterTileCommTile0_sendPacketToHostMcuViaTile1(chanend_t txCmd, unsigned char packetType)
{
}
#endif
*/
	
void startup_task(/*void *arg*/chanend_t c[])
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
    platform_start();

#if ON_TILE(0)
    /* Initialize filesystem  */
    //rtos_fatfs_init(qspi_flash_ctx);

    /* Create the filesystem demol task */
    //filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
	
	//modem_ctrl_create(appconfUART_TXD_TASK_PRIORITY);
	
    ////ftp_ctrl_create(appconfUART_TXD_TASK_PRIORITY,c);
	
    /*xTaskCreate((TaskFunction_t) MODEM_UART_ReceiveData,
                "MODEM_UART_ReceiveData",
                portTASK_STACK_DEPTH(MODEM_UART_ReceiveData),
                NULL,
                appconfUART_RXD1_TASK_PRIORITY,
                NULL);*/
				
	/*xTaskCreate((TaskFunction_t) MSP_UART_ReceiveData,
                "MSP_UART_ReceiveData",
                RTOS_THREAD_STACK_SIZE(MSP_UART_ReceiveData),
                NULL,
                appconfUART_RXD2_TASK_PRIORITY,
                NULL);*/

	/*xTaskCreate((TaskFunction_t) XMOS_UART_ReceiveData,
                "XMOS_UART_ReceiveData",
                RTOS_THREAD_STACK_SIZE(XMOS_UART_ReceiveData),
                NULL,
                appconfUART_TASK_PRIORITY,
                NULL);	*/				
#endif

#if ON_TILE(1)
    /* Initialize filesystem  */
    //rtos_fatfs_init(qspi_flash_ctx);

	filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
	/* Create the gpio control task */
    //gpio_ctrl_create(appconfGPIO_TASK_PRIORITY);

			
	//capture_thread(c);
	
	////filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
    /* Create audio pipeline */
    //example_pipeline_init(appconfAUDIO_PIPELINE_TASK_PRIORITY);
	
	/*xTaskCreate((TaskFunction_t) capture_thread,
                "capture_thread",
                (portTASK_STACK_DEPTH(capture_thread)),
                c,
                appconfUART_RXD2_TASK_PRIORITY,
                NULL);*/
#endif
	
#if 0
#if ON_TILE(0)	
	int i;
	for (i = 0; i < 5; i++)
	{
		int a = 5;
		//UART_transmitData_TILE_0(MODEM_TXD, (i+'0'));
		UART_transmitData_TILE_0(MODEM_TXD, (a+'0'));
		printf("111receive_data: %d\n", 0);
	}
#endif	
#if ON_TILE(1)
	int i;
	for (i = 0; i < 5; i++)
	{
		int b = 6;
		int c = 7;
		//UART_transmitData_TILE_0(MODEM_TXD, (i+'0'));
		UART_transmitData_TILE_1(MSP_TXD, (b+'0'));
		UART_transmitData_TILE_1(XMOS_DEBUG_TXD, (c+'0'));
		printf("222receive_data: %d\n", 0);
	}	
#endif
#endif
	
	for (;;) {	
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		//xmos_to_msp_comm_send_txd_intertile0to1(intertile_ctx);
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

//void start_FreeRTOS_Task(void);

void start_FreeRTOS_Task(void)
{
	//vTaskDelay(pdMS_TO_TICKS(2000));
	Debug_TextOut(0, "TILE 1 DEVICE BOOTED");
	vTaskStartScheduler();
	Debug_TextOut(0, "TILE 1 DEVICE BOOTED 1 TRUE");
	return;
}

void rtos_fatfs_init_tast_call(void);
void rtos_fatfs_init_tast_call(void)
{
	////f_unmount ("");
	rtos_fatfs_re_init(qspi_flash_ctx);
}

void tile_common_init(chanend_t c[])
{
    platform_init(c[0]);
	platform_start();
#if ON_TILE(0)	

#ifdef FROM_RAM
//shared_flash_write();
sf_write_test();
#endif
#ifndef FROM_RAM

    /*xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                (portTASK_STACK_DEPTH(startup_task)),
                c,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);*/
                
               // chanend seq_num;

    /* Initialize filesystem  */
    rtos_fatfs_init(qspi_flash_ctx);
	////rtos_fatfs_free_memory(qspi_flash_ctx);
	
	/* Initialize uart TX  */
	rtos_uart_tx_start(uart_tx_ctx);

    /* Create the filesystem demol task */
    //filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
	
	//modem_ctrl_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
	
    ftp_ctrl_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY,c[6]);
	
    xTaskCreate((TaskFunction_t) MODEM_UART_ReceiveData,
                "MODEM_UART_ReceiveData",
                2*portTASK_STACK_DEPTH(MODEM_UART_ReceiveData),
                NULL,
                appconfUART_RXD1_TASK_PRIORITY,
                NULL);

    /* Create uart demo tasks and receivers */
    //uart_demo_create(appconfUART_TXD_TASK_PRIORITY);
	
	/*xTaskCreate((TaskFunction_t) MSP_UART_ReceiveData,
                "MSP_UART_ReceiveData",
                RTOS_THREAD_STACK_SIZE(MSP_UART_ReceiveData),
                NULL,
                appconfUART_RXD2_TASK_PRIORITY,
                NULL);*/

	/*xTaskCreate((TaskFunction_t) XMOS_UART_ReceiveData,
                "XMOS_UART_ReceiveData",
                RTOS_THREAD_STACK_SIZE(XMOS_UART_ReceiveData),
                NULL,
                appconfUART_TASK_PRIORITY,
                NULL);	*/	
				
	xTaskCreate((TaskFunction_t) InterTileCommTile0_rxInstance,
                "InterTileCommTile0_rxInstance",
                2* portTASK_STACK_DEPTH(InterTileCommTile0_rxInstance),
                c,
                appconfUART_RXD2_TASK_PRIORITY,
                NULL);
	Debug_TextOut(0, "TILE 0 DEVICE BOOTED");
	#endif			
#endif	
#if ON_TILE(1)
	//------------------------------------------
	//added on 20_07_2022 //--to be Removed
	/*rtos_gpio_t *gpio_ctx;
	const rtos_gpio_port_id_t led_port = 18;	//XS1_PORT_4C port

	rtos_printf("enable led port %d\n", led_port);
	rtos_gpio_port_enable(gpio_ctx, led_port);

	rtos_gpio_port_out(gpio_ctx, led_port, 1);*/
	//------------------------------------------

    /*xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                (portTASK_STACK_DEPTH(startup_task)),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);*/
				#ifndef FROM_RAM
    /* Initialize filesystem  */
    rtos_fatfs_init(qspi_flash_ctx);
	
	////filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
	/* Create the gpio control task */
    //gpio_ctrl_create(appconfGPIO_TASK_PRIORITY);

			
	//capture_thread(c);
	
	//filesystem_demo_create(appconfFILESYSTEM_DEMO_TASK_PRIORITY);
    /* Create audio pipeline */
    //example_pipeline_init(appconfAUDIO_PIPELINE_TASK_PRIORITY);

	xTaskCreate((TaskFunction_t) capture_thread,
                "capture_thread",
                2* (portTASK_STACK_DEPTH(capture_thread)),
                c,
                appconfCAPTURE_TASK_PRIORITY,
                NULL);
				
	//------------------------------------------
	//added on 20_07_2022 //--to be Removed
	//rtos_gpio_port_out(gpio_ctx, led_port, 0);
	//------------------------------------------	
	Debug_TextOut(0, "TILE 1 DEVICE BOOTED");
	#endif
#endif
			
	printf("\r\ntile_common_init(txCmd);%ld ,Tile(%d) ", c,THIS_XCORE_TILE);			

	////resetGpio(PORT_LEDS_X1D4C1);	
	////port_out(XS1_PORT_4C, 0x00);	
 	//chanend_free(c);
	vTaskStartScheduler();
}
#if 0
#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1[], chanend_t c2, chanend_t c3) {
	(void) c0;
    (void) c2;
    (void) c3;		

    tile_common_init(c1);
	Debug_TextOut(0, "TILE 0 DEVICE BOOTED");
	vTaskStartScheduler();		
	//chanend_free(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0[], chanend_t c1, chanend_t c2, chanend_t c3) {
    (void) c1;
    (void) c2;
    (void) c3;  
	
	////setGpio(PORT_LEDS_X1D4C1);
	////port_out(XS1_PORT_4C, 0x09);

	/*xTaskCreate((TaskFunction_t) capture_thread,
                "capture_thread",
                (portTASK_STACK_DEPTH(capture_thread)),
                c0,
                appconfUART_TXD_TASK_PRIORITY,
                NULL);*/	
		
	platform_init(c0[0]);
	platform_start();
				
    /* Initialize filesystem  */
    rtos_fatfs_init(qspi_flash_ctx);
	
	capture_thread(c0);
	////tile_common_init(c0);	
	////delay_milliseconds(10000);
	Debug_TextOut(0, "TILE 1 DEVICE BOOTEDDDDDDDDDDDDDDDDDDDD");
	
	vTaskStartScheduler();	
    //chanend_free(c0);
}
#endif
#endif

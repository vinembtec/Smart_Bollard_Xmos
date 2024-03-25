// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "mem_analysis.h"
#if 1

static uint8_t test_vector_0[] = {0x00, 0xFF, 0xAA, 0x55};
static uint8_t test_vector_1[] = {0xDE, 0xAD, 0xBE, 0xEF};
static uint8_t test_vector_2[1000] = {0};

#if ON_TILE(0)
typedef enum {
    MODEM_TXD = 0,
	MSP_TXD,
    DEBUG_TXD
} txdUARTSelection;
extern void UART_transmitData(txdUARTSelection n,char c);
#endif

extern unsigned char txCommand_tile1_to_tile0;

int main_test_send_receive_fixed(rtos_intertile_t *intertile_ctx)
{
    printf("fixed Start\n");

    for (int i=0; i<INTERTILE_TEST_VECTOR_2_LEN; i++)
    {
        test_vector_2[i] = (uint8_t)(0xFF & i);
    }


    //for (int i=0; i<INTERTILE_TEST_ITERS; i++)
    {
        size_t test_len = INTERTILE_TEST_VECTOR_2_LEN;
        uint8_t *test_buf = test_vector_2;

        printf("fixed Test iteration %d\n", 0);

        #if ON_TILE(INTERTILE_RX_TILE)
        {
            printf("fixed TX %u\n", test_len);
            rtos_intertile_tx(intertile_ctx,
                              INTERTILE_RPC_PORT,
                              test_buf,
                              test_len);
            printf("fixed TX done\n");
        }
        #endif

        #if ON_TILE(INTERTILE_TX_TILE)
        {
            uint8_t *rx_buf = NULL;
            size_t bytes_rx = rtos_intertile_rx(intertile_ctx,
                                                INTERTILE_RPC_PORT,
                                                (void**)&rx_buf,
                                                RTOS_OSAL_WAIT_MS(10));
            if (rx_buf == NULL)
            {
                printf("fixed RX returned NULL buffer\n");
                return -1;
            }

            if (bytes_rx != test_len)
            {
                printf("fixed RX timed out.  Got %u expected %u\n", bytes_rx, test_len);
                rtos_osal_free(rx_buf);
                return -1;
            } else {
                printf("fixed RX passed.  Got %u expected %u\n", bytes_rx, test_len);
				printf("fixed RX passed data.  Got %s \n", rx_buf);
            }

            for (size_t j=0; j< bytes_rx; j++)
            {
                if (test_buf[j] != rx_buf[j])
                {
                    printf("fixed RX failed at index %u.  Got %u expected %u\n", j, rx_buf[j], test_buf[j]);
                    rtos_osal_free(rx_buf);
                    return -1;
                }
            }
           rtos_osal_free(rx_buf);
        }
        #endif
    }

    printf("fixed Done\n");
    return 0;
}

int main_test_send_receive_variable(rtos_intertile_t *intertile_ctx)
{
    printf("variable Start\n");

    for (int i=0; i<INTERTILE_TEST_VECTOR_2_LEN; i++)
    {
        test_vector_2[i] = 'R';//(uint8_t)(0xFF & i);
    }	
	test_vector_2[0] = 'V';
	test_vector_2[1] = 'I';
	test_vector_2[2] = 'V';
	test_vector_2[3] = 'E';
	test_vector_2[4] = 'K';

    //for (int i=0; i<INTERTILE_TEST_ITERS; i++)
    {
        size_t test_len = INTERTILE_TEST_VECTOR_2_LEN;
        uint8_t *test_buf = test_vector_2;

        printf("variable Test iteration %d\n", 0);


        #if ON_TILE(INTERTILE_TX_TILE)
        {
            printf("variable TX len %u\n", test_len);

            rtos_intertile_tx_len(intertile_ctx,
                                  INTERTILE_RPC_PORT,
                                  test_len);

            printf("variable TX data of len %u\n", test_len);

            size_t tx = rtos_intertile_tx_data(intertile_ctx,
                                               test_buf,
                                               test_len);
            if (tx != test_len)
            {
                printf("variable TX failed.  send %d expected %d\n", tx, test_len);
                return -1;
            }

            printf("variable TX done\n");
        }
        #endif

        #if ON_TILE(INTERTILE_RX_TILE)
        {
            uint8_t *rx_buf = NULL;
            size_t len_rx = 0;

            len_rx = rtos_intertile_rx_len(intertile_ctx,
                                           INTERTILE_RPC_PORT,
                                           RTOS_OSAL_WAIT_MS(10));

            rx_buf = (uint8_t*)rtos_osal_malloc(len_rx * sizeof(uint8_t));

            if (rx_buf == NULL)
            {
                printf("variable RX failed.  Tried to malloc %d bytes\n", len_rx);
                return -1;
            }

            size_t bytes_rx = rtos_intertile_rx_data(intertile_ctx,
                                                (void*)rx_buf,
                                                len_rx);

            if (bytes_rx != test_len)
            {
                printf("variable RX failed.  Got %u expected %u\n", bytes_rx, test_len);
                rtos_osal_free(rx_buf);
                return -1;
            } else {
                printf("variable RX passed.  Got %u expected %u\n", bytes_rx, test_len);
				//printf("1variable RX passed.  Data %s\n", (void*)rx_buf);
				//printf("2variable RX passed.  Data %02X\n", rx_buf[0]);
				//printf("2variable RX passed.  Data %02X\n", rx_buf[1]);
				/*for (int i = 0; i < INTERTILE_TEST_VECTOR_2_LEN; i++)
				{
					UART_transmitData(MODEM_TXD, rx_buf[i]);
					printf("111receive_data: %d\n",rx_buf[i]);
				}*/	
            }

            for (size_t j=0; j< bytes_rx; j++)
            {
                if (test_buf[j] != rx_buf[j])
                {
                    printf("variable RX failed at index %u.  Got %u expected %u\n", j, rx_buf[j], test_buf[j]);
                    rtos_osal_free(rx_buf);
                    return -1;
                }
            }
           rtos_osal_free(rx_buf);
        }
        #endif
    }

    printf("variable Done\n");
    return 0;
}
#endif

int xmos_to_msp_comm_send_txd_intertile0to1(rtos_intertile_t *intertile_ctx)
{
    printf("variable Start\n");

    for (int i=0; i<INTERTILE_TEST_VECTOR_2_LEN; i++)
    {
        test_vector_2[i] = 'R';//(uint8_t)(0xFF & i);
    }	
	test_vector_2[0] = 'V';
	test_vector_2[1] = 'I';
	test_vector_2[2] = 'V';
	test_vector_2[3] = 'E';
	test_vector_2[4] = 'K';

    //for (int i=0; i<INTERTILE_TEST_ITERS; i++)
    {
        size_t test_len = INTERTILE_TEST_VECTOR_2_LEN;
        uint8_t *test_buf = test_vector_2;
		uint8_t *test_buf2 = test_vector_2;
		
        printf("variable Test iteration %d\n", 0);

#if 1
        #if ON_TILE(INTERTILE_TX_TILE)
        {
			test_vector_2[0] = 'E';
			test_vector_2[1] = 'M';
			test_vector_2[2] = 'Q';
			test_vector_2[3] = 'O';
			test_vector_2[4] = 'S';
			
			printf("variable TX len %u\n", test_len);
			
            printf("variable TX len to RX %S\n", test_buf2);

            rtos_intertile_tx_len(intertile_ctx,
                                  INTERTILE_RPC_PORT,
                                  test_len);

            printf("variable TX data of len %u\n", test_len);

            size_t tx = rtos_intertile_tx_data(intertile_ctx,
                                               test_buf,
                                               test_len);
            if (tx != test_len)
            {
                printf("variable TX failed.  send %d expected %d\n", tx, test_len);
                return -1;
            }

            printf("variable TX done\n");
        }
        #endif
#endif
#if 1
        #if ON_TILE(INTERTILE_RX_TILE)
        {
			printf("variable RX len to TX %S\n", test_buf2);
			
            uint8_t *rx_buf = NULL;
            size_t len_rx = 0;
            len_rx = rtos_intertile_rx_len(intertile_ctx,
                                           INTERTILE_RPC_PORT,
                                           RTOS_OSAL_WAIT_MS(10));

            rx_buf = (uint8_t*)rtos_osal_malloc(len_rx * sizeof(uint8_t));

            if (rx_buf == NULL)
            {
                printf("variable RX failed.  Tried to malloc %d bytes\n", len_rx);
                return -1;
            }

            size_t bytes_rx = rtos_intertile_rx_data(intertile_ctx,
                                                (void*)rx_buf,
                                                len_rx);

            if (bytes_rx != test_len)
            {
                printf("variable RX failed.  Got %u expected %u\n", bytes_rx, test_len);
                rtos_osal_free(rx_buf);
                return -1;
            } else {
                printf("variable RX passed.  Got %u expected %u\n", bytes_rx, test_len);
            }

            for (size_t j=0; j< bytes_rx; j++)
            {
                if (test_buf[j] != rx_buf[j])
                {
                    printf("variable RX failed at index %u.  Got %u expected %u\n", j, rx_buf[j], test_buf[j]);
                    rtos_osal_free(rx_buf);
                    return -1;
                }
            }
           rtos_osal_free(rx_buf);
        }
        #endif
#endif
    }
    printf("variable Done\n");
    return 0;
}

static void mem_analysis( void *arg )
{
	const char* task_name = ( const char* ) arg;

	if( strcmp( task_name, "heap" ) == 0 )
	{
		for( ;; )
		{
			debug_printf("\tMinimum heap free: %d\n\n", xPortGetMinimumEverFreeHeapSize());
			debug_printf("\tCurrent heap free: %d\n\n", xPortGetFreeHeapSize());
			vTaskDelay( pdMS_TO_TICKS( 1000 ) );
		}
	}

	int free_stack_words;
	TaskHandle_t task = NULL;

	for( ;; )
	{
		/* Always check to allow for tasks that can be deleted to be analyzed */
		task = xTaskGetHandle( task_name );

		if( task != NULL )
		{
			free_stack_words = uxTaskGetStackHighWaterMark( task );
			debug_printf("\t%s free stack words: %d\n\n", task_name, free_stack_words);
		}
		vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

void mem_analysis_create( const char* task_name )
{
    xTaskCreate( mem_analysis, "mem_an\n", RTOS_THREAD_STACK_SIZE(mem_analysis), ( void * ) task_name, appconfMEM_ANALYSIS_TASK_PRIORITY, NULL );
}

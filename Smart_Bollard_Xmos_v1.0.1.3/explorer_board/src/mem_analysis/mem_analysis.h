// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MEM_ANALYSIS_H_
#define MEM_ANALYSIS_H_

#include "rtos_intertile.h"

#include <platform.h>
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#define INTERTILE_TX_TILE 0
#define INTERTILE_RX_TILE 1
#define INTERTILE_TEST_VECTOR_2_LEN   1000
void mem_analysis_create( const char* task_name );
int main_test_send_receive_fixed(rtos_intertile_t *intertile_ctx);
int main_test_send_receive_variable(rtos_intertile_t *intertile_ctx);
int xmos_to_msp_comm_send_txd_intertile0to1(rtos_intertile_t *intertile_ctx);
#endif /* MEM_ANALYSIS_H_ */

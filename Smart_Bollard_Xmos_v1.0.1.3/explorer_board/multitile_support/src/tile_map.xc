// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// XMOS Public License: Version 1

#include "platform.h"
#include <xs1.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
//void main_tile0(chanend c0, chanend c1[], chanend c2, chanend c3);
//void main_tile1(chanend c0[], chanend c1, chanend c2, chanend c3);
void main_tile2(chanend c0, chanend c1, chanend c2, chanend c3);
void main_tile3(chanend c0, chanend c1, chanend c2, chanend c3);

void tile_common_init(chanend c[]);
void start_FreeRTOS_Task(void);
extern void capture_thread(chanend instance[]);
	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	extern void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 );
	extern void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 );
	extern void Debug_Display( int8_t minimumDebugLevel, const char * pszText );
	//////////////////////////////////

#if (XSCOPE_HOST_IO_ENABLED == 1)
#ifndef XSCOPE_HOST_IO_TILE
#define XSCOPE_HOST_IO_TILE 0
#endif
void init_xscope_host_data_user_cb(chanend c_host);
#endif
}

#define PRINT_FN(...) printf(__VA_ARGS__); printf("\n");

void setup_ddr();
on tile[1]: port PORT_LEDS_X1D4C = XS1_PORT_4C;

#define PORT_LEDS_X1D4C0 0
#define PORT_LEDS_X1D4C1 1
#define PORT_LEDS_X1D4C2 2
#define PORT_LEDS_X1D4C3 3

extern void setGpio(uint8_t position);
extern void resetGpio(uint8_t position);
extern int readGpio(void);

#if 1
void main_tile0(chanend ?c0, chanend c1[], chanend ?c2, chanend ?c3) {
	//(void) c0;
    //(void) c2;
    //(void) c3;		
    tile_common_init(c1);
	start_FreeRTOS_Task();
}

void main_tile1(chanend c0[], chanend ?c1, chanend ?c2, chanend ?c3) {
    //(void) c1;
    //(void) c2;
    //(void) c3;  
	setGpio(PORT_LEDS_X1D4C0);	
	tile_common_init(c0);	
	start_FreeRTOS_Task();
}
#endif

int main(void) {
#if (XSCOPE_HOST_IO_ENABLED == 1)
  chan c_xscope_host;
#endif
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1))
  chan c_t0_t1[7];
#endif
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1))
  chan c_t0_t2;
#endif
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t0_t3;
#endif
#if ((PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1) && \
     (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1))
  chan c_t1_t2;
#endif
#if ((PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t1_t3;
#endif
#if ((PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t2_t3;
#endif
  par {  
#if (XSCOPE_HOST_IO_ENABLED == 1)
    //xscope_host_data(c_xscope_host);
    //on tile[XSCOPE_HOST_IO_TILE] : init_xscope_host_data_user_cb(c_xscope_host);
#endif
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
    on tile[0] : main_tile0(
                    null,
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                    c_t0_t1,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                    c_t0_t2,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                    c_t0_t3
#else
                    null
#endif
                );
#endif

    on tile[1] : setup_ddr();
	
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
    on tile[1] : main_tile1(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                    c_t0_t1,
#else
                    null,
#endif
                    null,
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                    c_t1_t2,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                    c_t1_t3
#else
                    null
#endif
                );
#endif

#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
    on tile[2] : main_tile2(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                    c_t0_t2,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                    c_t1_t2,
#else
                    null,
#endif
                    null,
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                    c_t2_t3
#else
                    null
#endif
                );
#endif

#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
    on tile[3] : main_tile3(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                    c_t0_t3,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                    c_t1_t3,
#else
                    null,
#endif
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                    c_t2_t3
#else
                    null,
#endif
                    null);
#endif
  }
  return 0;
}

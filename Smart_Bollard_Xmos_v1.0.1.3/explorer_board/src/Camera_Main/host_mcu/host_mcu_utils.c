
/*** Include Files ***/

#include <stdio.h>
#include <print.h>
#include "host_mcu_utils.h"


/*** Macro Definitions ***/

/*** Type Definitions ***/

/*** Function Prototypes ***/

/*** Variable Declarations ***/

/*** Function Definitions ***/

void HostMcuUtils_printByteArrayHexString(unsigned char *byteArray, unsigned short size)
{
    int count;

    /*for (count = 0; count < size; count++)
    {
        printf("%02X ", byteArray[count]);
    }
    printf("\r\n");
	*/

    for (count = 0; count < size; count++)
    {
		char byteString[5];
		sprintf(byteString, "%02X ", byteArray[count]);
		printstr(byteString);
    }
	printstrln(" ");
}


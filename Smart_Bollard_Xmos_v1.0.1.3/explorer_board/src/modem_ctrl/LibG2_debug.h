/*
 * LibG2_debug.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef MODEM_CTRL_LIBG2_DEBUG_H_
#define MODEM_CTRL_LIBG2_DEBUG_H_

#include "stdarg.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "FreeRTOS.h"	
//#include "../modem_ctrl/main.h"

typedef enum
{
    e_ZEROPAD   = 1,    // Pad with zero
    e_SIGN      = 2,    // Unsigned/signed long
    e_PLUS      = 4,    // Show plus
    e_SPACE     = 8,    // Space if plus
    e_LEFT      = 16,   // Left justified
    e_SPECIAL   = 32,   // 0x
    e_LargeHEX  = 64    // Use 'ABCDEF' instead of 'abcdef'
} enuFormatFlags;

static const char *lower_digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void Debug_TextOut( int8_t minVerbosity, const char * pszText );
void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 );
void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 );
void Debug_Display( int8_t minimumDebugLevel, const char * pszText );
uint16_t liberty_sprintf(char *ptrBuf, const char *ptrFmt, ...);
//uint16_t liberty_vsprintf( char *ptrBuf, const char *ptrFmt, va_list args );
uint16_t mpb_IS_DIGIT(char ascii);
char mpb_TO_UPPER(char ascii);
uint16_t mpb_strnlen(const char *pszStr, uint16_t maxLng);
static uint16_t do_atoi_and_skip(const char **ppStr);

static char * do_format_eaddr(char *ptrStr, uint8_t *addr, int16_t size, int16_t precision, int16_t type);
static char * do_format_iaddr(char *ptrStr, uint8_t *addr, int16_t size, int16_t precision, int16_t type);
static char * do_format_number(char *ptrStr, int32_t num, uint16_t base, int16_t size, int16_t precision, int16_t type);


#endif /* COMMON_MODULES_LIBG2_DEBUG_H_ */

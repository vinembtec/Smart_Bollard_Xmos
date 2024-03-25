//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_debug.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_debug_api
//! @{
//
//*****************************************************************************

#include "LibG2_debug.h"

static char 				lf_found = true, debug_msg[100], time_str[21];
uint8_t 					Debug_Verbose_Level = 0;

////#if ON_TILE(1)
uint16_t msg_lng;
extern int year;
extern int month;
extern int date;
extern int hour;
extern int minute;
extern int second;
////#endif

#if ON_TILE(1)
typedef enum {
	MSP_TXD = 1,
    XMOS_DEBUG_TXD
} txdUARTSelection;
extern void UART_transmitData_TILE_1(txdUARTSelection n,char c);
#endif

#if ON_TILE(0)
typedef enum {
    MODEM_TXD = 0
} txdUARTSelection;
extern void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c);
#endif

uint16_t debug_out_UART(const char *debug_uart_buf)
{
	uint16_t debug_uart_bytes_counter;
	uint16_t debug_uart_length = 0;
	debug_uart_length = (uint16_t) strlen(debug_uart_buf);
	for(debug_uart_bytes_counter=0; debug_uart_bytes_counter<debug_uart_length; debug_uart_bytes_counter++)
	{
		#if ON_TILE(0)
		UART_transmitData_TILE_0(0,debug_uart_buf[debug_uart_bytes_counter]);
		#endif		
		////UART_transmitData(EUSCI_A1_BASE, debug_uart_buf[debug_uart_bytes_counter]);
		#if ON_TILE(1)
		UART_transmitData_TILE_1(2,debug_uart_buf[debug_uart_bytes_counter]);
		#endif
		////UART_transmitData_TILE_1(2,'A');
	}
	//DelayMs(1);
	return debug_uart_length;
}


/**************************************************************************/
//! Prints text message in debug log
//! \param uint8_t minVerbosity - verbose level to check while printing this message
//! \param const char * pointer to the text string to print
//! \return void
/**************************************************************************/
void Debug_TextOut( int8_t minVerbosity, const char * pszText )
{
	Debug_Display( minVerbosity, pszText );
	//Debug_Display( minVerbosity, "\r\n" );
}

/**************************************************************************/
//! Prints text message with one variable argument in debug log
//! \param uint8_t minVerbosity - verbose level to check while printing this message
//! \param const char * pointer to the text string to print
//! \param uint32_t someValue - argument to be included into the text while printing
//! \return void
/**************************************************************************/
void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue )
{
	if ( Debug_Verbose_Level >= minVerbosity )
	{
		strcpy( debug_msg + liberty_sprintf(debug_msg, pszFormat, someValue), "\r\n" );
		Debug_Display( minVerbosity, debug_msg );
	}
}

/**************************************************************************/
//! Prints text message with two variable argument in debug log
//! \param uint8_t minVerbosity - verbose level to check while printing this message
//! \param const char * pointer to the text string to print
//! \param uint32_t arg1 - argument to be included into the text while printing
//! \param uint32_t arg2 - argument to be included into the text while printing
//! \return void
/**************************************************************************/
void Debug_Output2( int8_t minVerbosity, const char * pszFormat, uint32_t arg1, uint32_t arg2 )
{
	if ( Debug_Verbose_Level >= minVerbosity )
	{
		strcpy( debug_msg + liberty_sprintf(debug_msg, pszFormat, arg1, arg2), "\r\n" );
		Debug_Display( minVerbosity, debug_msg );
	}
}

/**************************************************************************/
//! Prints text message with six variable argument in debug log
//! \param uint8_t minVerbosity - verbose level to check while printing this message
//! \param const char * pointer to the text string to print
//! \param uint32_t a1 - argument to be included into the text while printing
//! \param uint32_t a2 - argument to be included into the text while printing
//! \param uint32_t a3 - argument to be included into the text while printing
//! \param uint32_t a4 - argument to be included into the text while printing
//! \param uint32_t a5 - argument to be included into the text while printing
//! \param uint32_t a6 - argument to be included into the text while printing
//! \return void
/**************************************************************************/
void Debug_Output6( int8_t minVerbosity, const char * pszFormat, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6 )
{
	if ( Debug_Verbose_Level >= minVerbosity )
	{
		strcpy( debug_msg + liberty_sprintf(debug_msg, pszFormat, a1, a2, a3, a4, a5, a6), "\r\n" );
		Debug_Display( minVerbosity, debug_msg );
	}
}

/**************************************************************************/
//! initializes UART, adds RTC and then Outputs data to debug port
//! \param uint8_t minVerbosity - verbose level to check while printing this message
//! \param const char * pointer to the text string to print
//! \return void
/**************************************************************************/
void Debug_Display( int8_t minimumDebugLevel, const char * pszText )
{
	rtos_printf("Debug: %s\n", pszText);
	
////#if ON_TILE(1)	
#if 1
	//uint16_t year, month, date, hours, minutes, seconds, msg_lng;
	//UART_enableModule(EUSCI_A1_BASE);
	/*if ( Debug_Verbose_Level < minimumDebugLevel )
	{
		debug_out_UART("");
		UART_disableModule(EUSCI_A1_BASE);
		return;
	}*/

	//if(Debug_Verbose_Level > 0)    // timestamp prefix
	{
		////if (lf_found)    // LNFD was sent in prior message
		{
			/*year    = (uint16_t) RTCYEAR;
			month   = (uint16_t) RTCMON;
			date    = (uint16_t) RTCDAY;
			hours   = (uint16_t) RTCHOUR;
			minutes = (uint16_t) RTCMIN;
			seconds = (uint16_t) RTCSEC;*/
			if ( month > 12 )
			{
				year++;
				month = 1;
			}

			liberty_sprintf( time_str, "%04d_%02d_%02d %02d:%02d:%02d ", year, month, date, hour, minute, second );
			debug_out_UART (time_str);
		}
	}

	lf_found = ( strchr( pszText, '\n') != NULL ) ? true : false;

	msg_lng = debug_out_UART ( pszText );
	
	debug_out_UART ( "\r\n");
	if ( (pszText == debug_msg) && (sizeof(debug_msg) <= msg_lng) )
	{
		debug_out_UART ( "\r\n========= DANGER! sizeof(debug_msg) ========\r\n" );
	}
	//UART_disableModule(EUSCI_A1_BASE);
	return;
#endif
////#endif
}

/**************************************************************************/
//! Internal driver function2 for debug log, used to convert numbers to ASCII
//! \param char * pointer to the text string to print
//! \param char * ptrFmt pointer to add and convert to ASCII string to print
//! \param va_list args - variable list of arguments to convert
//! \return int16_t length
/**************************************************************************/
int16_t liberty_vsprintf( char *ptrBuf, const char *ptrFmt, va_list args )
{
	int16_t     fmt_flags;            // Flags to do_format_number()
	int16_t     field_width;          // Width of output field
	int16_t     precision;            // Min. # of digits for integers; number of chars for from string
	int16_t     qualifier;            // 'h', 'l', or 'L' for integer fields
	int16_t     index, base, len;
	uint32_t num;
	void    *ptr_void;
	char    *psz_str;
	char    *ptr_str;


	for ( psz_str = ptrBuf;  *ptrFmt != 0;  ++ptrFmt)
	{
		if (*ptrFmt != '%')
		{
			*psz_str++ = *ptrFmt;
			continue;
		}

		// Process flags
		fmt_flags = 0;
		repeat:
		switch ( *(++ptrFmt) )
		{
		case '-': fmt_flags |= e_LEFT;      goto repeat;

		case '+': fmt_flags |= e_PLUS;      goto repeat;

		case ' ': fmt_flags |= e_SPACE;     goto repeat;

		case '#': fmt_flags |= e_SPECIAL;   goto repeat;

		case '0': fmt_flags |= e_ZEROPAD;   goto repeat;

		default:    break;
		}

		field_width = -1;   // Get field width

		if ( mpb_IS_DIGIT(*ptrFmt) )
		{
			field_width = do_atoi_and_skip(&ptrFmt);
		}
		else if ( *ptrFmt == '*' )
		{
			++ptrFmt;

			field_width = va_arg(args, int);

			if (field_width < 0)
			{
				field_width = -field_width;
				fmt_flags |= e_LEFT;
			}
		}

		precision = -1;     // Get the precision

		if ( *ptrFmt == '.' )
		{
			++ptrFmt;

			if (mpb_IS_DIGIT(*ptrFmt))
			{
				precision = do_atoi_and_skip(&ptrFmt);
			}
			else if ( *ptrFmt == '*' )
			{
				++ptrFmt;
				precision = va_arg(args, int);
			}

			if (precision < 0)
			{
				precision = 0;
			}
		}

		qualifier = -1;     // Get the conversion qualifier

		if ( (*ptrFmt == 'h') || (*ptrFmt == 'l') || (*ptrFmt == 'L') )
		{
			qualifier = *ptrFmt;
			ptrFmt++;
		}

		base = 10;      // Default base

		switch (*ptrFmt)
		{
		case 'c':
			if ( (fmt_flags & e_LEFT) == 0 )
			{
				while (--field_width > 0) *psz_str++ = ' ';
			}

			*psz_str++ = (uint8_t) va_arg(args, int);

			while (--field_width > 0)
			{
				*psz_str++ = ' ';
			}

			continue;

		case 's':
			ptr_str = va_arg(args, char *);

			if (!ptr_str)
			{
				ptr_str = "<NULL>";
			}

			len = mpb_strnlen(ptr_str, precision);

			if ( (fmt_flags & e_LEFT) == 0 )
			{
				while (len < field_width--) *psz_str++ = ' ';
			}

			for (index = 0; index < len; ++index)
			{
				*psz_str++ = *ptr_str++;
			}

			while (len < field_width--)
			{
				*psz_str++ = ' ';
			}

			continue;

		case 'p':
			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				fmt_flags |= e_ZEROPAD;
			}
			psz_str = do_format_number(psz_str, (uint32_t) va_arg(args, void *), 16, field_width, precision, fmt_flags);
			continue;

		case 'n':
			if (qualifier == 'l')
			{
				ptr_void = (void*) va_arg(args, uint32_t *);
				*((uint32_t*)ptr_void) = (uint32_t) (psz_str - ptrBuf);
			}
			else
			{
				ptr_void = (void*) va_arg(args, uint16_t *);
				*((uint16_t*)ptr_void) = (uint16_t) (psz_str - ptrBuf);
			}

			continue;

		case 'A':
			fmt_flags |= e_LargeHEX;
			// fall thru
		case 'a':
			if (qualifier == 'l')
				psz_str = do_format_eaddr(psz_str, va_arg(args, uint8_t *), field_width, precision, fmt_flags);
			else
				psz_str = do_format_iaddr(psz_str, va_arg(args, uint8_t *), field_width, precision, fmt_flags);
			continue;

			// Integer number formats - set up the flags and "break"
		case 'o':
			base = 8;
			break;

		case 'X':
			fmt_flags |= e_LargeHEX;
			// fall thru
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			fmt_flags |= e_SIGN;
			// fall thru
		case 'u':
			break;

		case 'E':
		case 'G':
		case 'e':
		case 'f':
		case 'g':
			/*#ifndef MPB_NOFLOAT_FMT
                    psz_str = flt(psz_str, va_arg(args, double), field_width, precision, *ptrFmt, fmt_flags | e_SIGN);
                #endif*/
			continue;

		default:
			if (*ptrFmt != '%')
			{
				*psz_str++ = '%';
			}

			if (*ptrFmt)
				*psz_str++ = *ptrFmt;
			else
				--ptrFmt;

			continue;
		}

		if (qualifier == 'l')
		{
			num = va_arg(args, uint32_t);
		}
		else if (qualifier == 'h')
		{
			if (fmt_flags & e_SIGN)
				num = va_arg(args, uint16_t);
			else
				num = va_arg(args, uint16_t);
		}
		else if (fmt_flags & e_SIGN)
			num = va_arg(args, uint16_t);
		else
			num = va_arg(args, uint16_t);

		psz_str = do_format_number(psz_str, num, base, field_width, precision, fmt_flags);
	}

	*psz_str = '\0';

	return (uint16_t) (psz_str - ptrBuf);
}

/**************************************************************************/
//! Internal driver function1 for debug log, used to convert numbers to ASCII
//! \param char * pointer to the text string to print
//! \param char * ptrFmt pointer to add and convert to ASCII string to print
//! \param .....
//! \return int16_t length
/**************************************************************************/
uint16_t liberty_sprintf(char *ptrBuf, const char *ptrFmt, ...)
{
	va_list args;
	uint16_t     lng;

	va_start(args, ptrFmt);

	lng = liberty_vsprintf(ptrBuf, ptrFmt, args);

	va_end(args);

	return lng;
}

/**************************************************************************/
//! Internal driver function3 for debug log, used to convert digits to ASCII
/**************************************************************************/
uint16_t mpb_IS_DIGIT(char ascii)
{
	return ( ((ascii) >= '0') && ((ascii) <= '9') );
}

/**************************************************************************/
//! Internal driver function4 for debug log, used to convert characters to upper case ASCII
/**************************************************************************/
//.................................................................
//returns upper-case code for ascii character
//.................................................................
char mpb_TO_UPPER(char ascii)
{
	return (char) ( (((ascii) < 'a') || ((ascii) > 'z')) ? (char) (ascii) : (char) (((ascii) + 'A') - 'a') );
}

/**************************************************************************/
//! Internal driver function5 for debug log, used to get the string length
/**************************************************************************/
uint16_t mpb_strnlen(const char *pszStr, uint16_t maxLng)
{
	uint16_t length = (pszStr != NULL) ? 0 : maxLng;

	while ( (*pszStr != '\0') && (++length < maxLng > 0) )
	{
		++pszStr;
	}

	return length;
}

/**************************************************************************/
//! Internal driver function6 for debug log
/**************************************************************************/
static uint16_t do_atoi_and_skip(const char **ppStr)
{
	uint16_t val = 0;
	while ( mpb_IS_DIGIT(**ppStr) )
	{
		val *= 10;
		val += (**ppStr) - '0';
		*ppStr += 1;
	}
	return val;
}

/**************************************************************************/
//! Internal driver function7 for debug log
/**************************************************************************/
static char * do_format_number(char *ptrStr, int32_t num, uint16_t base, int16_t size, int16_t precision, int16_t type)
{
	char       tmp[66];
	char       pad  = (type & e_ZEROPAD) ? '0' : ' ';
	char       sign = 0;
	const char *dig = ((type & e_LargeHEX) ? upper_digits: lower_digits);
	uint16_t       index = 0;

	if ( (base < 2) || (base > 36) )
	{
		return 0;
	}

	if (type & e_LEFT)
	{
		type &= ~e_ZEROPAD;
	}

	if (type & e_SIGN)
	{
		if (num < 0)
		{
			size -= 1;     sign = '-';   num  = -num;
		}
		else if (type & e_PLUS)
		{
			size -= 1;     sign = '+';
		}
		else if (type & e_SPACE)
		{
			size -= 1;     sign = ' ';
		}
	}

	if (type & e_SPECIAL)
	{
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size -= 1;
	}

	if (num == 0)
	{
		tmp[index++] = '0';
	}
	else
	{
		while ( num != 0 )
		{
			tmp[index++] = dig[((uint32_t) num) % (unsigned) base];

			num = ((uint32_t) num) / (unsigned) base;
		}
	}

	if (index > precision)
	{
		precision = index;
	}

	size -= precision;

	if ( (type & (e_ZEROPAD | e_LEFT)) == 0 )
	{
		while ( size-- > 0 )
		{
			*ptrStr++ = ' ';
		}

	}

	if (sign)
	{
		*ptrStr++ = sign;
	}

	if (type & e_SPECIAL)
	{
		if (base == 8)
		{
			*ptrStr++ = '0';
		}
		else if (base == 16)
		{
			*ptrStr++ = '0';
			*ptrStr++ = lower_digits[33];
		}
	}

	if ( (type & e_LEFT) == 0 )
	{
		while ( size-- > 0 )
		{
			*ptrStr++ = pad;
		}
	}

	while ( index < precision-- )
	{
		*ptrStr++ = '0';
	}

	while ( index-- > 0 )
	{
		*ptrStr++ = tmp[index];
	}

	while (size-- > 0)
	{
		*ptrStr++ = ' ';
	}

	return ptrStr;
}

/**************************************************************************/
//! Internal driver function8 for debug log
/**************************************************************************/
static char * do_format_eaddr(char *ptrStr, uint8_t *addr, int16_t size, int16_t precision, int16_t type)
{
	char       tmp[24];
	const char *dig = (type & e_LargeHEX) ? upper_digits : lower_digits;
	uint16_t        index, len;

	for ( index = len = 0;  index < 6;  ++index )
	{
		if (index != 0)
		{
			tmp[len++] = ':';
		}

		tmp[len++] = dig[ addr[index] >> 4 ];

		tmp[len++] = dig[ addr[index] & 0x0F ];
	}

	if ( (type & e_LEFT) == 0 )
	{
		while ( len < size-- )
		{
			*ptrStr++ = ' ';
		}
	}

	for ( index = 0; index < len; ++index )
	{
		*ptrStr++ = tmp[index];
	}

	while (len < size--)
	{
		*ptrStr++ = ' ';
	}

	return ptrStr;
}

/**************************************************************************/
//! Internal driver function9 for debug log
/**************************************************************************/
static char * do_format_iaddr(char *ptrStr, uint8_t *addr, int16_t size, int16_t precision, int16_t type)
{
	char tmp[24];
	uint16_t index, num, len;

	for ( index = len = 0;  index < 4;  ++index)
	{
		if (index != 0)
		{
			tmp[len++] = '.';
		}

		num = addr[index];

		if (num == 0)
		{
			tmp[len++] = lower_digits[0];
		}
		else
		{
			if (num >= 100)
			{
				tmp[len++] = lower_digits[num / 100];

				num = num % 100;

				tmp[len++] = lower_digits[num / 10];

				num = num % 10;
			}
			else if (num >= 10)
			{
				tmp[len++] = lower_digits[num / 10];

				num = num % 10;
			}

			tmp[len++] = lower_digits[num];
		}
	}

	if ( (type & e_LEFT) == 0 )
	{
		while (len < size--)
		{
			*ptrStr++ = ' ';
		}
	}

	for (index = 0;  index < len;  ++index)
	{
		*ptrStr++ = tmp[index];
	}

	while (len < size--)
	{
		*ptrStr++ = ' ';
	}

	return ptrStr;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


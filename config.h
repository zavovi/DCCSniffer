/*
 * config.h
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Macros & Constants definitions
*******************************************************************************/

/* Clock frequency */
#ifndef F_OSC
	#define F_OSC 16000000
#endif

/* DCC print use for analyzer or debug */
#ifndef DCC_PRINT
	#define DCC_PRINT 1
#endif

/* DCC print errors */
#ifndef DCC_PRINT_ERROR
	#define DCC_PRINT_ERROR 0
#endif

/* DCC print menu and wait for option */
#ifndef DCC_PRINT_MENU
	#define DCC_PRINT_MENU 0
#endif

/* Maximum size of buffer for received DCC bits */
#ifndef DCC_MAX_BUFFER
	#define DCC_MAX_BUFFER	800
#endif

/* Ignore IDLE packet - if true, then no print IDLE packet and no call callback */
#ifndef DCC_IGNORE_IDLE
	#define DCC_IGNORE_IDLE	1
#endif

/* Number of uart for print */
#ifndef DCC_DBG_UART
	#define DCC_DBG_UART	0
#endif

/* Baud rate of uart for print */
#ifndef DCC_DBG_UART_BAUD
	#define DCC_DBG_UART_BAUD	115200
#endif

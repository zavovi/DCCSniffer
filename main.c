/*
 * main.c
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Includes
*******************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "dcc_hw.h"
#include "buffer.h"
#include "dcc_machine.h"
#if DCC_PRINT
#include "dcc_print.h"
#endif
#include "assert.h"
#include "macros.h"
#include "uart.h"

/*******************************************************************************
* Function definitions
*******************************************************************************/
static dcc_error_t packet_callback(dcc_packet_t * packet);

/*******************************************************************************
* Local variables
*******************************************************************************/

/*******************************************************************************
* Public API functions
*******************************************************************************/
int main(void)
{
    uint8_t cnt_preamble = 0;
	uint8_t ignore_idle = DCC_IGNORE_IDLE;
	unsigned int led = 0;
	char c;

	/* Initialize DCC machine */
	dcc_machine_init(packet_callback);	

	/* Initialize DCC hardware */
	dcc_hw_init();


#if DCC_PRINT
	/* Initialize UART */
	init_print(DCC_DBG_UART, DCC_DBG_UART_BAUD);

#if DCC_PRINT_MENU
	/* Print start menu with options */
	print_menu();
	
	uint8_t option = print_wait_option();
	if(option == '4')
		ignore_idle = 1;
	else
		ignore_idle = 0;

#endif

	/* Print Table header - DCC sniffer */
	print_header();
#endif
	
	DDRB = 32;
	PORTB = 32;
	while (1) 
	{ 		
		c = get_char();
		switch(c)
		{
		case 'i':
			ignore_idle ^= 1;
			break;
		}

		/* Parse DCC packet */
		uint8_t cnt = dcc_parse(&cnt_preamble);
		
		if(led >= 50000)
		{
			PORTB ^= 32;
			led = 0;
		}
		led++;

		if(cnt > 0)
			/* Process one DCC packet */
			dcc_process(cnt, cnt_preamble, ignore_idle);
	}
}

/*******************************************************************************
* Private API functions
*******************************************************************************/

/* Callback for parsed packet */
static dcc_error_t packet_callback(dcc_packet_t * packet)
{
	ASSERT(packet != NULL);

	//TODO: compare address	

	switch(packet->type)
	{	
	case PCKT_TYPE_NONE:
		break;
	case PCKT_TYPE_IDLE:
		break;
    case PCKT_TYPE_RESET:
		break;
    case PCKT_TYPE_TRAIN:
		break;
    case PCKT_TYPE_TRAIN_F:
		break;
    case PCKT_TYPE_ACCESSORY:
		break;
    case PCKT_TYPE_ACCESSORY_EX:
		break;
    case PCKT_TYPE_PROGRAM:
		break;
	}

	return OK;
}

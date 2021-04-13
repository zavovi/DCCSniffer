/*
 * uart.c
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
#include "../config.h"

/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/

/*******************************************************************************
* Public API functions
*******************************************************************************/

/* Initialize UART */
void uart_init(uint8_t uart, uint32_t baud)
{	
	//TODO: 
	//uint16_t ubrr = (uint16_t) ((F_OSC/(8*(float)baud))-1);

	uint16_t ubrr = 16;

	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t) ubrr;

	UCSR0A = (1<<U2X0);

	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3<<UCSZ00);
}

/* Write one char to UART */
void uart_write_char(unsigned char c)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	
	/* Put data into buffer, sends the data */
	UDR0 = c;

}

/* Write string to UART */
void uart_write_str(char * str, uint8_t len)
{
	for(int i=0; i<len; i++)
		uart_write_char(str[i]);
}

/* Get char from UART - non-blocking */
int get_char(void)
{
	/* No char in buffer */
	if( !(UCSR0A & (1<<RXC0)) )
		return -1;

	/* Return received char */
	return UDR0;
}

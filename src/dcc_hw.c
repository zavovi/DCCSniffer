/*
 * dcc_hw.c
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
#include "../config.h"
#include "buffer.h"
#include "dcc_machine.h"
#if DCC_PRINT
	#include "dcc_print.h"
#endif

/* times for log.1 and log.0 in us */
#define TIME_LOG0_MAX	9900
#define TIME_LOG0_MIN	95
#define TIME_LOG1_MAX	64
#define TIME_LOG1_MIN	52

#define LOG0	0
#define LOG1 	1

/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/


static uint8_t bit_tmp=2;

/*******************************************************************************
* Public API functions
*******************************************************************************/

void dcc_hw_init(void)
{
	/* PCINT */
	PCMSK0 	|= (1 << PCINT3);	//enable only pin change interrupt 3
	PCIFR 	|= (1 << PCIF0);	//clear any old pcint interrupts
	PCICR	|= (1 << PCIE0); //enable only the pin change interrupts

	/* TIMER 1 for measure input DCC signal*/
	TCCR1A=(0<<WGM11)|(0<<WGM10);			
	TCCR1B=(0<<WGM13)|(1<<WGM12);
	TCCR1B|=(0<<CS12)|(1<<CS11)|(0<<CS10);
	OCR1A=0xFF;	//9: 10 us
	TCNT1 = 0;

	/* enable interrupts */
	sei();
}

/*******************************************************************************
* Private API functions
*******************************************************************************/


SIGNAL(PCINT0_vect){
	uint32_t time = 0;
	uint8_t bit = 2;
	time = (uint32_t)((2*TCNT1)/3); //for using with 12MHz XTAL
	TCNT1 = 0;

	if(time > TIME_LOG1_MIN && time < TIME_LOG1_MAX)
	{
		bit = LOG1;
	}else if(time > TIME_LOG0_MIN && time < TIME_LOG0_MAX)
	{
		bit = LOG0;
	}
	else
	{
#if DCC_PRINT
		print_error(ERR_DCC_CORRUPTED);
#endif
	}
  
	if(bit_tmp == bit){
		/* Add bit to buffer */
		buf_add(bit);
		bit_tmp = 2;
	}else{
		bit_tmp = bit;
	}
}

ISR(TIMER1_COMPA_vect)
{
}

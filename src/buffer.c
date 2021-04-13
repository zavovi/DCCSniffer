/*
 * buffer.c
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Includes
*******************************************************************************/
#include <avr/io.h>
#include "../config.h"


/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/

typedef struct dcc_buffer_s
{
    uint8_t buffer[DCC_MAX_BUFFER];
    uint32_t length;
    uint32_t head;
    uint32_t tail;
} dcc_buffer_t;

dcc_buffer_t buffer = 
{
	.length = DCC_MAX_BUFFER,
	.head = 0,
	.tail = 0,
};

/*******************************************************************************
* Public API functions
*******************************************************************************/
int buf_add(uint8_t data)
{
	int next = buffer.head + 1;
	if (next >= buffer.length)
		next = 0;

	/* Cicular buffer is full */
	if (next == buffer.tail)
		return -1;  // quit with an error

	buffer.buffer[buffer.head] = data;
	buffer.head = next;

	return 0;
}

int buf_get(void)
{

	// if the head isn't ahead of the tail, we don't have any characters
	if (buffer.head == buffer.tail)
		return -1;  // quit with an error

	uint8_t data = buffer.buffer[buffer.tail];
	buffer.buffer[buffer.tail] = 0;  // clear the data (optional)

	int next = buffer.tail + 1;
	if (next >= buffer.length)
		next = 0;

	buffer.tail = next;

	return (int)data;
}

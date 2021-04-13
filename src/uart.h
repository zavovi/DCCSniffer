/*
 * uart.h
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Macros & Constants definitions
*******************************************************************************/

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Function definitions
*******************************************************************************/

/* Initialize UART */
void uart_init(uint8_t uart, uint32_t baud);

/* Write one char to UART */
void uart_write_char(char c);

/* Write string to UART */
void uart_write_str(char * str, uint8_t len);

/* Get char from UART - non-blocking */
int get_char(void);

/*
 * dcc_print.h
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

/* Initialize print and UART */
void init_print(uint8_t uart, uint32_t baud);

/* Print init menu */
void print_menu(void);

/* Print header */
void print_header(void);

/* Print error */
void print_error(dcc_error_t err);

/* Print packet */
void print_packet(dcc_packet_t * packet, uint8_t * dcc_data, uint8_t data_size);

/* Print assert */
void assert_print(const char* file, const uint32_t line);

/* Wait for readed option */
uint8_t print_wait_option(void);

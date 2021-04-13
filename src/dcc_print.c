/*
 * dcc_print.c
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*
    +-------+------------------------------------------------------------------------------------------------------+-------------+------+
    |   #   |                                              DCC PACKET                                              | PACKET TYPE | ADDR |
    +-------+------------------------------------------------------------------------------------------------------+-------------+------+
    |    0  | 1111111111 11111111 00000000 11111111                                                                |    IDLE     |    0 |
    |.......|......................................................................................................|.............|......|
    |    1  | 1111111111 11111111 00000000 11111111                                                                |    IDLE     |    0 |
    |.......|......................................................................................................|.............|......|
    |    2  | 1111111111 00000001 01000010 01000011                                                                |    TRAIN    |    1 |
    |       | Direction: 0, Speed: 2                                                                               |             |      |
    |.......|......................................................................................................|.............|......|
    |    3  | ERROR: DCC signal corrupted.                                                                         |             |      |
    |.......|......................................................................................................|.............|......|
    
*/

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "dcc_machine.h"
#include "assert.h"
#include "uart.h"
#include "../config.h"

#define MAX_CHAR_PRINT  150
#if DCC_PRINT
#define sniff_print(str, num) uart_write_str(str, num)
#else
#define sniff_print(str, num)
#endif

typedef enum
{
	PRINT_OPT_NO_PRINT,
	PRINT_OPT_ALL_PRINT,
	PRINT_OPT_PRINT_PACKET,
	PRINT_OPT_PRINT_PACKET_DESC,
	PRINT_OPT_PRINT_SKIP_IDLE,
} print_option_t;

static const char *bit_rep[16] = 
{
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

#define PCKT_STR_TYPE_NONE			"-----------"
#define PCKT_STR_TYPE_IDLE			"   IDLE    "
#define PCKT_STR_TYPE_RESET			"   RESET   "
#define PCKT_STR_TYPE_TRAIN			"   TRAIN   "
#define PCKT_STR_TYPE_TRAIN_F		"TRAIN_FUNC "
#define PCKT_STR_TYPE_ACCESSORY		" ACCESSORY "
#define PCKT_STR_TYPE_ACCESSORY_EX	"ACCESSORY_X"
#define PCKT_STR_TYPE_PROGRAM		" WRITE CV  "

#if DCC_PRINT_ERROR
static const char *err_str[]= 
{
	[ERR_DCC_CORRUPTED] =      "DCC signal corrupted!",
	[ERR_PCKT_CORRUPTED] =     "DCC packet corrupted!",
	[ERR_CRC] =				   "Bad CRC count of DCC packet!",
};
#endif

static const char *print_opt_str[]= 
{
	[PRINT_OPT_NO_PRINT] =      	"No print.",
	[PRINT_OPT_ALL_PRINT] =     	"Print all parsed values from packet (two lines). Print all IDLE packet.",
	[PRINT_OPT_PRINT_PACKET] =  	"Print only packet data, type and address (one line)",
	[PRINT_OPT_PRINT_SKIP_IDLE] =  	"Not print IDLE packets.",
};

uint8_t print_en = 1;	/* Enable printing packets */
uint8_t print_desc = 1;	/* Enable printing description */

/*******************************************************************************
* Function definitions
*******************************************************************************/

static void packet_print_train(dcc_packet_t * packet);
static void packet_print_train_f(dcc_packet_t * packet);
static void packet_print_acc(dcc_packet_t * packet);
static void packet_print_acc_ex(dcc_packet_t * packet);
static void packet_print_prog(dcc_packet_t * packet);

static void print_multi(char c, uint32_t len);

static unsigned int print_number(unsigned int number, uint8_t digits);

/*******************************************************************************
* Local variables
*******************************************************************************/
unsigned int line_num = 0;

/*******************************************************************************
* Public API functions
*******************************************************************************/

/* Initialize print and UART */
void init_print(uint8_t uart, uint32_t baud)
{
	/* Initialize UART */
	uart_init(uart, baud);
	
	/*
		***********************************************************************************************************************************
		*                                                   DCC sniffer & DCC decoder v1.0                                                *
		* Autor: Vilem Zavodny                                                                                                            *
		* WEB:   http://zavavov.vzap.eu                                                                                                   *
		***********************************************************************************************************************************
	*/
	
	print_multi('\n', 2);
	print_multi('*', 133);
	sniff_print("\n", 1);
	
	sniff_print("*", 1);
	print_multi(' ', 51);
	sniff_print("DCC sniffer & DCC decoder v1.0", 30);
	print_multi(' ', 50);
	sniff_print("*\n", 2);

	sniff_print("* ", 2);
	sniff_print("Autor: Vilem Zavodny", 20);
	print_multi(' ', 110);
	sniff_print("*\n", 2);

	sniff_print("* ", 2);
	sniff_print("WEB:   http://zavavov.vzap.eu", 29);
	print_multi(' ', 101);
	sniff_print("*\n", 2);

	print_multi('*', 133);
	print_multi('\n', 3);
}

/* Print init menu */
void print_menu(void)
{
	sniff_print("\n", 1);
	sniff_print("Press x for stop printing and show menu again.\n\n", 48);
	sniff_print("Options:\n", 9);
	for(int i=0; i<=PRINT_OPT_PRINT_SKIP_IDLE; i++)
	{
		sniff_print(" #", 2);
		print_number(i, 0);
		sniff_print(" - ", 3);
		sniff_print((char*)print_opt_str[i], strlen(print_opt_str[i]));
		sniff_print("\n", 1);
	}

	sniff_print("Select: #", 9);
}

/* Print header */
void print_header(void)
{
	/* Disable printing */
	if(!print_en)
		return;

	/*
		+-----+------------------------------------------------------------------------------------------------------+-------------+------+
		|  #  |                                              DCC PACKET                                              | PACKET TYPE | ADDR |
		+-----+------------------------------------------------------------------------------------------------------+-------------+------+
	*/

    print_multi('\n', 2);
	
	sniff_print("+", 1);
	print_multi('-', 7);
	sniff_print("+", 1);
	print_multi('-', 102);
	sniff_print("+", 1);
	print_multi('-', 13);
	sniff_print("+", 1);
	print_multi('-', 6);
	sniff_print("+\n", 2);

	sniff_print("|   #   |", 9);
	print_multi(' ', 46);
	sniff_print("DCC PACKET", 10);
	print_multi(' ', 46);
	sniff_print("| PACKET TYPE | ADDR |\n", 23);

	sniff_print("+", 1);
	print_multi('-', 7);
	sniff_print("+", 1);
	print_multi('-', 102);
	sniff_print("+", 1);
	print_multi('-', 13);
	sniff_print("+", 1);
	print_multi('-', 6);
	sniff_print("+\n", 2);

}

/* Print error */
void print_error(dcc_error_t err)
{
#if DCC_PRINT_ERROR
    uint16_t len = 0;
#endif
	/* Disable printing */
	if(!print_en)
		return;

#if DCC_PRINT_ERROR
	/* Number of line */
	sniff_print("| ", 2);
	print_number(line_num, 5);
	sniff_print(" | ", 3);
	line_num++;

    /* Error */
	sniff_print("ERROR: ", 7);
	len += 7;
	sniff_print(err_str[err], strlen(err_str[err]));
	len += strlen(err_str[err]);
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

	/* |             |      |\n */
	sniff_print("|", 1);
	print_multi(' ', 13);
	sniff_print("|", 1);
	print_multi(' ', 6);
	sniff_print("|\n", 2); 
        
	/*
		|.....|......................................................................................................|.............|......|
	*/
	sniff_print("|", 1);
	print_multi('.', 7);
	sniff_print("|", 1);
	print_multi('.', 102);
	sniff_print("|", 1);
	print_multi('.', 13);
	sniff_print("|", 1);
	print_multi('.', 6);
	sniff_print("|\n", 2);
#endif
}

/* Print packet */
void print_packet(dcc_packet_t * packet, uint8_t * dcc_data, uint8_t data_size)
{
	uint8_t pream = 0;
	
	ASSERT(packet != NULL);
	ASSERT(dcc_data != NULL);
	
	/* Disable printing */
	if(!print_en)
		return;
	    
    /* Number of line */
	sniff_print("| ", 2);
	print_number(line_num, 5);
	sniff_print(" | ", 3);
	line_num++;

    /* Preamble */
	if(packet->preamble < 20)
	{
		pream = 13;
		sniff_print("{preamble (", 11);
		pream += print_number((int)packet->preamble, 0);
		sniff_print(")}", 2);
	}
	else
	{
		pream = 18;
		sniff_print("{long-preamble (", 16);
		pream += print_number((int)packet->preamble, 0);
		sniff_print(")}", 2);
    }   
    sniff_print(" 0 ", 3);    
    
    /* Packet bytes */
    for(int i=0; i<=data_size; i++)
    {
		sniff_print((char*)bit_rep[(dcc_data[i] >> 4)], 4);
		sniff_print((char*)bit_rep[(dcc_data[i] & 0x0F)], 4);
		sniff_print(" ", 1);   

		if(i <data_size)
			sniff_print("0 ", 2);   
		else
			sniff_print("1 ", 2);   
			
    }    
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(100-(pream+2+((8+3)*(data_size+1)))); i++)
        sniff_print(" ", 1);
    
    /* Packet type */
	sniff_print("| ", 2); 
	switch(packet->type)
	{
	case PCKT_TYPE_NONE:
		sniff_print(PCKT_STR_TYPE_NONE, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_IDLE:
		sniff_print(PCKT_STR_TYPE_IDLE, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_RESET:
		sniff_print(PCKT_STR_TYPE_RESET, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_TRAIN:
		sniff_print(PCKT_STR_TYPE_TRAIN, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_TRAIN_F:
		sniff_print(PCKT_STR_TYPE_TRAIN_F, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_ACCESSORY:
		sniff_print(PCKT_STR_TYPE_ACCESSORY, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_ACCESSORY_EX:
		sniff_print(PCKT_STR_TYPE_ACCESSORY_EX, strlen(PCKT_STR_TYPE_NONE));
		break;
	case PCKT_TYPE_PROGRAM:
		sniff_print(PCKT_STR_TYPE_PROGRAM, strlen(PCKT_STR_TYPE_NONE));
		break;
	default:
		break;
	}
	sniff_print(" |", 2); 
    
    /* Packet address */	
	print_number(packet->address, 5);
	sniff_print(" |", 2);
    
    /* End of line */
    sniff_print("\n", 1);  

	/* Print by type */
    if(print_desc && packet->type)
    {
		switch(packet->type)
   		{
		case PCKT_TYPE_TRAIN:
			packet_print_train(packet);
			break;
		case PCKT_TYPE_TRAIN_F:
			packet_print_train_f(packet);
			break;
		case PCKT_TYPE_ACCESSORY:
			packet_print_acc(packet);
			break;
		case PCKT_TYPE_ACCESSORY_EX:
			packet_print_acc_ex(packet);
			break;
		case PCKT_TYPE_PROGRAM:
			packet_print_prog(packet);
			break;
		default:
			break;
		}
    }

	/*
		|.....|......................................................................................................|.............|......|
	*/
	sniff_print("|", 1);
	print_multi('.', 7);
	sniff_print("|", 1);
	print_multi('.', 102);
	sniff_print("|", 1);
	print_multi('.', 13);
	sniff_print("|", 1);
	print_multi('.', 6);
	sniff_print("|\n", 2);
    
}

/* Print assert */
void assert_print(const char* file, const uint32_t line)
{
	sniff_print("\nASSERT in file: ", 15);
	sniff_print((char*)file, strlen(file));
	sniff_print(" line: ", 7);
	print_number((int)line, 0);
	sniff_print("\n", 1);

	for(;;);
}

/* Wait for readed option */
uint8_t print_wait_option(void)
{
    char print_str[MAX_CHAR_PRINT];
    uint16_t len = 0;
	int c;
	
	while(1)
	{
		c = get_char();
		if(c == -1)
			continue;
			
		len = snprintf(print_str, MAX_CHAR_PRINT, "%c\n", c);
		sniff_print(print_str, len);

		/* Stop printing and show menu again */
		if(c == 'x' || c == 'X')
		{
			
			sniff_print("Selected end of printing.\n", 26);
			return c;
		}

	/*	switch(c-48)
		{
		case PRINT_OPT_NO_PRINT:
			len = snprintf(print_str, MAX_CHAR_PRINT, "Selected option %d (%s).\n", c-48, print_opt_str[c-48]);
			sniff_print(print_str, len);
			print_en = 0;
			return c;
		case PRINT_OPT_ALL_PRINT:
			len = snprintf(print_str, MAX_CHAR_PRINT, "Selected option %d (%s).\n", c-48, print_opt_str[c-48]);
			sniff_print(print_str, len);
			print_en = 1;
			print_desc = 1;
			return c;
		case PRINT_OPT_PRINT_PACKET:
			len = snprintf(print_str, MAX_CHAR_PRINT, "Selected option %d (%s).\n", c-48, print_opt_str[c-48]);
			sniff_print(print_str, len);
			print_en = 1;
			print_desc = 0;
			return c;
		case PRINT_OPT_PRINT_SKIP_IDLE:
			len = snprintf(print_str, MAX_CHAR_PRINT, "Selected option %d (%s).\n", c-48, print_opt_str[c-48]);
			sniff_print(print_str, len);
			print_en = 1;
			print_desc = 1;
			return c;
		}*/
	}

}

/*******************************************************************************
* Private API functions
*******************************************************************************/

/* Print train packet */
static void packet_print_train(dcc_packet_t * packet)
{
    uint16_t len = 0;

	ASSERT(packet != NULL);
    

    sniff_print("|       | ", 10);

    /* Train info */
	if(packet->train.C == -1)
	{		
        sniff_print("Direction: ", 11);
		len = 11;
		len += print_number((int)packet->train.direction, 0);
		sniff_print(", Speed: ", 9);
		len += 9;
		len += print_number((int)packet->train.speed, 0);
	}
	else
	{
        sniff_print("Direction: ", 11);
		len = 11;
		len += print_number((int)packet->train.direction, 0);
        sniff_print(", C: ", 5);
		len += 5;
		len += print_number((int)packet->train.C, 0);
		sniff_print(", Speed: ", 9);
		len += 9;
		len += print_number((int)packet->train.speed, 0);
	}
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

    sniff_print("|             |      |\n", 23); 

}

/* Print train function packet */
static void packet_print_train_f(dcc_packet_t * packet)
{
    uint16_t len = 0;

	ASSERT(packet != NULL);
    
    sniff_print("|       | ", 10);

	for(int i=0; i<30; i++)
	{
		if(packet->train.F[i] != -1)
		{
			sniff_print("F", 1);
			len += 1;
			len += print_number(i, 0);
			sniff_print("=", 1);
			len += 1;
			len += print_number((int)packet->train.F[i], 0);
			sniff_print(", ", 2);
			len += 2;
		}
	}
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

	/* |             |      |\n */
	sniff_print("|", 1);
	print_multi(' ', 13);
	sniff_print("|", 1);
	print_multi(' ', 6);
	sniff_print("|\n", 2); 

}

/* Print accessory packet */
static void packet_print_acc(dcc_packet_t * packet)
{
    uint16_t len = 0;

	ASSERT(packet != NULL);
    

    sniff_print("|       | ", 10);
    
    /* Accessory info */
	sniff_print("Switch: ", 8);
	len += 8;
	len += print_number(packet->acc.ddd, 0);
	sniff_print(", State: ", 9);
	len += 9;
	len += print_number((int)packet->acc.c, 0);
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

	/* |             |      |\n */
	sniff_print("|", 1);
	print_multi(' ', 13);
	sniff_print("|", 1);
	print_multi(' ', 6);
	sniff_print("|\n", 2); 

}

/* Print accessory extended packet */
static void packet_print_acc_ex(dcc_packet_t * packet)
{
    uint16_t len = 0;

	ASSERT(packet != NULL);
    

    sniff_print("|       | ", 10);
    
    /* Accessory extended info */
	sniff_print("Extended data: ", 15);
	len += 15;
	sniff_print((char*)bit_rep[(packet->acc.ext_data >> 4)], 4);
	sniff_print((char*)bit_rep[(packet->acc.ext_data & 0x0F)], 4);
	len += 8;
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

	/* |             |      |\n */
	sniff_print("|", 1);
	print_multi(' ', 13);
	sniff_print("|", 1);
	print_multi(' ', 6);
	sniff_print("|\n", 2); 

}

/* Print programming packet */
static void packet_print_prog(dcc_packet_t * packet)
{
    uint16_t len = 0;

	ASSERT(packet != NULL);
    

    sniff_print("|       | ", 10);
    
    /* Programming info */
	sniff_print("CV: ", 4);
	len += 4;
	len += print_number(packet->prog.cv, 0);
	sniff_print(", Value: ", 9);
	len += 9;
	len += print_number(packet->prog.val, 0);
    
    /* Print spaces after DCC packet */
    for(int i=0; i<(101-len); i++)
        sniff_print(" ", 1);

	/* |             |      |\n */
	sniff_print("|", 1);
	print_multi(' ', 13);
	sniff_print("|", 1);
	print_multi(' ', 6);
	sniff_print("|\n", 2); 

}

static void print_multi(char c, uint32_t len)
{
	for(int i=0; i<len; i++)
		sniff_print(&c, 1);
}

static unsigned int print_number(unsigned int number, uint8_t digits)
{
	uint8_t t, s, d, j, dt;

	ASSERT(number < 100000);

	dt = (uint8_t)(number/10000);
	t = (uint8_t)(number/1000 - dt*10000);
	s = (uint8_t)((number - dt*10000 - t*1000)/100);
	d = (uint8_t)((number - dt*10000 - t*1000 - s*100)/10);
	j = (uint8_t)(number - dt*10000 - t*1000 - s*100 - d*10);
	
	if(dt)
	{
		if(digits >= 5)
			print_multi(' ', digits-5);
		else
			digits = 5;

		print_multi(dt+48, 1);
		print_multi(t+48, 1);
		print_multi(s+48, 1);
		print_multi(d+48, 1);
		print_multi(j+48, 1);
	}
	else if(t)
	{
		if(digits >= 4)
			print_multi(' ', digits-4);
		else
			digits = 4;

		print_multi(t+48, 1);
		print_multi(s+48, 1);
		print_multi(d+48, 1);
		print_multi(j+48, 1);
	}
	else if(s)
	{
		if(digits >= 3)
			print_multi(' ', digits-3);
		else
			digits = 3;

		print_multi(s+48, 1);
		print_multi(d+48, 1);
		print_multi(j+48, 1);
	}
	else if(d)
	{
		if(digits >= 2)
			print_multi(' ', digits-2);
		else
			digits = 2;

		print_multi(d+48, 1);
		print_multi(j+48, 1);
	}
	else
	{
		if(digits >= 1)
			print_multi(' ', digits-1);
		else
			digits = 1;

		print_multi(j+48, 1);
	}

	return digits;
}

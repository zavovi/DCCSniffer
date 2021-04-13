/*
 * dcc_machine.c
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "../config.h"
#include "dcc_machine.h"
#if DCC_PRINT
#include "dcc_print.h"
#endif
#include "assert.h"

/* MASKS for DCC packet */
#define MASK_ADDR_1 0b10000000
#define MASK_ADDR_2 0b11000000
#define MASK_PROG   0b11110000
#define MASK_ACC_EXTENDED 0b10000000

#define MASK_ACC_ADDR1	  0b00111111
#define MASK_ACC_ADDR2	  0b01110000
#define MASK_ACC_DDD	  0b00000111
#define MASK_ACC_C		  0b00001000
#define MASK_ACC_EXT_DATA 0b00011111

#define MASK_TRAIN_EX_ADDR	0b00111111
#define MASK_TRAIN_SPEED	0b11000000
#define MASK_TRAIN_F1		0b11100000
#define MASK_TRAIN_F2		0b11110000
#define MASK_TRAIN_SPEED1_VAL 	0b00001111
#define MASK_TRAIN_DIR1_VAL 	0b00100000
#define MASK_TRAIN_C_VAL 		0b00010000
#define MASK_TRAIN_SPEED2_VAL 	0b01111111
#define MASK_TRAIN_DIR2_VAL 	0b10000000

#define BITS_TO_SEGMENT	7

/*******************************************************************************
* Function definitions
*******************************************************************************/

static void packet_type_idle(dcc_packet_t * packet);
static void packet_type_reset(dcc_packet_t * packet);
static void packet_type_train(dcc_packet_t * packet, uint8_t * data, uint8_t cnt);
static void packet_type_accessory(dcc_packet_t * packet, uint8_t * data, uint8_t cnt);
static void packet_type_prog(dcc_packet_t * packet, uint8_t * data, uint8_t cnt);

/*******************************************************************************
* Local variables
*******************************************************************************/

typedef enum
{
	DCC_STEP_PREAMBLE,
	DCC_STEP_DATA,
} dcc_step_t;

/* Structure for parsing DCC signal */
typedef struct dcc_parse_s
{
	dcc_step_t step;
	uint8_t num_bit_step;
	uint8_t count;
} dcc_parse_t;

/* DCC packet parsing */
typedef struct dcc_s
{
	uint8_t data[10];
	packet_callback_t callback;
} dcc_t;


static dcc_t dcc;
static dcc_parse_t dcc_parse_st = 
{
	.step = DCC_STEP_PREAMBLE,
	.num_bit_step = 0,
	.count = 0,
};

/*******************************************************************************
* Public API functions
*******************************************************************************/

void dcc_machine_init(packet_callback_t callback)
{
	/* Clear DCC structure */
	memset(&dcc, 0, sizeof(dcc_t));
	dcc.callback = callback;
}



void dcc_process(uint8_t cnt, uint8_t cnt_preamble, uint8_t ignore_idle)
{
	uint8_t crc, crc_tmp;
	dcc_error_t err = OK;
    dcc_packet_t packet;

	/* Clear DCC packet */
	memset(&packet, 0, sizeof(dcc_packet_t));	
    packet.preamble = cnt_preamble;

	/* Not enough data received */
	if(cnt < 2)
	{
		err = ERR_PCKT_CORRUPTED;
		goto END;
	}

	crc = dcc.data[cnt];

	/* count CRC of received data */
	for(int i=0; i<cnt; i++)
	{
		if(i == 0)
			crc_tmp = dcc.data[i];
		else
			crc_tmp ^= dcc.data[i];
	}

	/* Check CRC */
	if(crc_tmp != crc)
	{
		err = ERR_CRC;
		goto END;
    }

    /* === Type of packet === */    

    /* IDLE packet */
	if(dcc.data[0] == 0xFF && dcc.data[1] == 0x00 && crc == 0xFF)
    {
        packet.type = PCKT_TYPE_IDLE;
		packet_type_idle(&packet);
    }
    /* Reset Packet */
	else if(dcc.data[0] == 0x00 && dcc.data[1] == 0x00)
    {
        packet.type = PCKT_TYPE_RESET;
		packet_type_reset(&packet);
    }
    /* Train Packet */
	else if( ((dcc.data[0]&MASK_ADDR_1) == 0b00000000 || (dcc.data[0]&MASK_ADDR_2) == 0b11000000) && cnt_preamble < 20 )
    {
        packet.type = PCKT_TYPE_TRAIN;
        packet_type_train(&packet, dcc.data, cnt);
    }
	/* Accessory Packet */
	else if((dcc.data[0]&MASK_ADDR_2) == 0b10000000)
    {
        packet.type = PCKT_TYPE_ACCESSORY;
        packet_type_accessory(&packet, dcc.data, cnt);
    }
	/* Program Packet */
	else if((dcc.data[0]&MASK_PROG) == 0b01110000)
    {
        packet.type = PCKT_TYPE_PROGRAM;
        packet_type_prog(&packet, dcc.data, cnt);
    }

END:
	/* Ignore IDLE packet - no print, no callback */
	if(ignore_idle && packet.type == PCKT_TYPE_IDLE)	
	{
	}
	else
	{
		if(err != OK)
		{
			#if DCC_PRINT
			/* Print error */
		    print_error(err);
			#endif 
		}
		else
		{
			#if DCC_PRINT
			/* Print packet */
		    print_packet(&packet, dcc.data, cnt);
			#endif 

			if(dcc.callback)
				dcc.callback(&packet);
		}
   }

	memset(&dcc, 0, sizeof(dcc_t));
}

uint32_t dcc_parse(uint8_t * cnt_preamble)
{
	int x;
	uint8_t bit_dcc;
	if((x = buf_get()) != -1)
	{
		bit_dcc = (uint8_t)x;
		switch(dcc_parse_st.step)
		{
		/* Receive preamble data */
		case DCC_STEP_PREAMBLE:
			if(bit_dcc == 1){
				dcc_parse_st.num_bit_step++;
			}
			if(bit_dcc == 0 && dcc_parse_st.num_bit_step >= 10){
            	*cnt_preamble = dcc_parse_st.num_bit_step;
				dcc_parse_st.num_bit_step = 0;
				dcc_parse_st.count = 0;
				dcc_parse_st.step = DCC_STEP_DATA;
			}else if(bit_dcc == 0 && dcc_parse_st.num_bit_step < 10){
				dcc_parse_st.num_bit_step = 0;
				#if DCC_PRINT
				/* Print error */
			    //print_error(ERR_DCC_CORRUPTED);
				#endif 
			}
			break;
		/* Receive all DCC data bytes */
		case DCC_STEP_DATA:
			if(dcc_parse_st.num_bit_step > BITS_TO_SEGMENT && bit_dcc == 0) {
				dcc_parse_st.num_bit_step = 0;
				dcc_parse_st.count++;
			}else if(dcc_parse_st.num_bit_step > BITS_TO_SEGMENT && bit_dcc == 1){
				dcc_parse_st.step = DCC_STEP_PREAMBLE;
				dcc_parse_st.num_bit_step = 0;
				return dcc_parse_st.count;
			}else{
				dcc.data[dcc_parse_st.count] |= (bit_dcc << (7-dcc_parse_st.num_bit_step));
				dcc_parse_st.num_bit_step++;
			}
			break;
		}
	}
	return 0;
}

/*******************************************************************************
* Private API functions
*******************************************************************************/

static void packet_type_idle(dcc_packet_t * packet)
{
	ASSERT(packet != NULL);
}

static void packet_type_reset(dcc_packet_t * packet)
{
	ASSERT(packet != NULL);
}

static void packet_type_train(dcc_packet_t * packet, uint8_t * data, uint8_t cnt)
{
	int p = 0;
	ASSERT(data != NULL);
	ASSERT(packet != NULL);

	for(int i=0; i<30; i++)
		packet->train.F[i] = -1;

	packet->train.C = -1;

	/* Classic train packet */
	if((data[0]&MASK_ADDR_1) == 0b00000000)
	{
		packet->address = data[p++];
	}
	/* Train packet with extended address */
	else if((data[0]&MASK_ADDR_2) == 0b11000000)
	{
		packet->address = ((data[p]&MASK_TRAIN_EX_ADDR)<<8) + data[p+1];	
		p+=2;
	}

	/* Train speed classic */
	if((data[p]&MASK_TRAIN_SPEED) == 0b01000000)
	{
		packet->train.speed = data[p]&MASK_TRAIN_SPEED1_VAL;
		packet->train.direction = (data[p]&MASK_TRAIN_DIR1_VAL)>>5;
		packet->train.C = (data[p]&MASK_TRAIN_C_VAL)>>4;
	}
	/* Train speed extended */
	else if(data[p] == 0b00111111)
	{
		p++;
		packet->train.speed = data[p]&MASK_TRAIN_SPEED2_VAL;
		packet->train.direction = (data[p]&MASK_TRAIN_DIR2_VAL)>>7;
	}
	/* Train functions F0-F4 */
	else if((data[p]&MASK_TRAIN_F1) == 0b10000000)
	{
		packet->type = PCKT_TYPE_TRAIN_F;
		packet->train.F[0] = (data[p]&0b00010000)>>4;
		packet->train.F[4] = (data[p]&0b00001000)>>3;
		packet->train.F[3] = (data[p]&0b00000100)>>2;
		packet->train.F[2] = (data[p]&0b00000010)>>1;
		packet->train.F[1] = (data[p]&0b00000001);   
	}
	/* Train functions F5-F8 */
	else if((data[p]&MASK_TRAIN_F2) == 0b10100000)
	{
		packet->type = PCKT_TYPE_TRAIN_F;
		packet->train.F[8] = (data[p]&0b00001000)>>3;
		packet->train.F[7] = (data[p]&0b00000100)>>2;
		packet->train.F[6] = (data[p]&0b00000010)>>1;
		packet->train.F[5] = (data[p]&0b00000001);
	}
	/* Train functions F9-F12 */
	else if((data[p]&MASK_TRAIN_F2) == 0b10110000)
	{
		packet->type = PCKT_TYPE_TRAIN_F;
		packet->train.F[12] = (data[p]&0b00001000)>>3;
		packet->train.F[11] = (data[p]&0b00000100)>>2;
		packet->train.F[10] = (data[p]&0b00000010)>>1;
		packet->train.F[9] = (data[p]&0b00000001);
	}
	/* Train functions F13-F20 */
	else if(data[p] == 0b11011110)
	{
		packet->type = PCKT_TYPE_TRAIN_F;
		packet->train.F[20] = (data[p]&0b10000000)>>7;
		packet->train.F[19] = (data[p]&0b01000000)>>6;
		packet->train.F[18] = (data[p]&0b00100000)>>5;
		packet->train.F[17] = (data[p]&0b00010000)>>4;
		packet->train.F[16] = (data[p]&0b00001000)>>3;
		packet->train.F[15] = (data[p]&0b00000100)>>2;
		packet->train.F[14] = (data[p]&0b00000010)>>1;
		packet->train.F[13] = (data[p]&0b00000001);
	}
	/* Train functions F21-F28 */
	else if(data[p] == 0b11011111)
	{
		packet->type = PCKT_TYPE_TRAIN_F;
		packet->train.F[28] = (data[p]&0b10000000)>>7;
		packet->train.F[27] = (data[p]&0b01000000)>>6;
		packet->train.F[26] = (data[p]&0b00100000)>>5;
		packet->train.F[25] = (data[p]&0b00010000)>>4;
		packet->train.F[24] = (data[p]&0b00001000)>>3;
		packet->train.F[23] = (data[p]&0b00000100)>>2;
		packet->train.F[22] = (data[p]&0b00000010)>>1;
		packet->train.F[21] = (data[p]&0b00000001);
	}


}

static void packet_type_accessory(dcc_packet_t * packet, uint8_t * data, uint8_t cnt)
{
	ASSERT(data != NULL);
	ASSERT(packet != NULL);


	if((data[1]&MASK_ACC_EXTENDED) == 0b00000000)
	{
		if(cnt < 3)
			return;

		packet->type = PCKT_TYPE_ACCESSORY_EX;
		
		packet->address = (~data[1])&MASK_ACC_ADDR2;
		packet->address = (packet->address<<2);
		packet->address |= (data[0]&MASK_ACC_ADDR1);

		packet->acc.ext_data = (data[2]&MASK_ACC_EXT_DATA);
	}
	else
	{
		if(cnt < 2)
			return;
		
		packet->address = (~data[1])&MASK_ACC_ADDR2;
		packet->address = (packet->address<<2);
		packet->address |= (data[0]&MASK_ACC_ADDR1);

		packet->acc.c = (data[1]&MASK_ACC_C)>>3;
		packet->acc.ddd = (data[1]&MASK_ACC_DDD);
	}

}

static void packet_type_prog(dcc_packet_t * packet, uint8_t * data, uint8_t cnt)
{
	ASSERT(data != NULL);
	ASSERT(packet != NULL);

	//if(cnt < 4)
	//	return;
	if((data[0]&0b00001100) == 0b1100)
	{
		packet->prog.cv = (data[0]&0b11)<<8;
		packet->prog.cv |= data[1];
		packet->prog.val = data[2];

		packet->prog.cv += 1;
	}	

}

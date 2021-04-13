/*
 * dcc_machine.h
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
typedef enum
{
	PCKT_TYPE_NONE,
	PCKT_TYPE_IDLE,
    PCKT_TYPE_RESET,
    PCKT_TYPE_TRAIN,
    PCKT_TYPE_TRAIN_F,
    PCKT_TYPE_ACCESSORY,
    PCKT_TYPE_ACCESSORY_EX,
    PCKT_TYPE_PROGRAM,
} dcc_packet_type_t;

typedef enum
{
	OK,
    ERR_DCC_CORRUPTED,
    ERR_PCKT_CORRUPTED,
	ERR_CRC,
} dcc_error_t;


typedef struct dcc_packet_train_s
{
	uint8_t direction;
	uint8_t speed;
	int 	C;
	int 	F[30];
} dcc_packet_train_t;

typedef struct dcc_packet_acc_s
{
	uint8_t c;
	uint8_t ddd;
	uint8_t ext_data;
} dcc_packet_acc_t;

typedef struct dcc_packet_prog_s
{
	uint16_t cv;
	uint8_t val;
} dcc_packet_prog_t;

typedef struct dcc_packet_s
{
    uint8_t             preamble;
    dcc_packet_type_t   type;
    uint16_t            address;
	union
	{
		dcc_packet_train_t 	train;
		dcc_packet_acc_t 	acc;
		dcc_packet_prog_t 	prog;
	};
} dcc_packet_t;

/*******************************************************************************
* Function definitions
*******************************************************************************/

typedef dcc_error_t (*packet_callback_t) (dcc_packet_t * packet);

void dcc_machine_init(packet_callback_t callback);
void dcc_process(uint8_t cnt, uint8_t cnt_preamble, uint8_t ignore_idle);
uint32_t dcc_parse(uint8_t * cnt_preamble);


/*
 * macros.h
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Macros & Constants definitions
*******************************************************************************/

#define set_bit(PORT,PIN) 	PORT|=_BV(PIN)
#define clear_bit(PORT,PIN)	PORT&=~_BV(PIN)
#define BIT(DATA, NUM)		(DATA&(1<<NUM))>>NUM	//select one bit in DATA reg

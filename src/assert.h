/*
 * assert.h
 *
 *  	Created on: 12.1.2017
 *      Author: Vilem Zavodny
 *      Web: http://www.zavavov.cz
 */

/*******************************************************************************
* Macros & Constants definitions
*******************************************************************************/

#if DCC_PRINT
#define ASSERT(c)	if(!(c)) assert_print(__FILE__, __LINE__)
#else
#define ASSERT(c)
#endif

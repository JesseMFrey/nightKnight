/*
 * Nosecone.h
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#ifndef NOSECONE_H_
#define NOSECONE_H_

//include so get* macros work
#include <msp430.h>
//needed for uint16_t
#include <stdint.h>

void init_Nosecone(void);

int nosecone_mode(int mode,int val1,int val2,int t1,int t2);

#define NC_NA           (0)

void set_chute(uint16_t chute);


#define getNoseconeLED()        ((OUTMOD_7==(TA0CCTL2&OUTMOD_7)?TA0CCR2:0))
#define getChuteLED()           ((OUTMOD_7==(TA0CCTL1&OUTMOD_7)?TA0CCR1:0))

#define NC_MAX_PWM     0XFFF

enum{NC_MODE_STATIC=0,NC_MODE_FADE,NC_MODE_FLASH,NC_MODE_ONE_SHOT,NC_MODE_NUM};


#endif /* NOSECONE_H_ */

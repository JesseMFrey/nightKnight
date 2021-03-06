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

void NC_debug(void);
void chute_debug(void);

int nosecone_mode(int mode,int val1,int val2,int t1,int t2);
int nosecone_pattern_mode(int mode,int val1,int val2,int t1,int t2);
int chute_mode(int mode,int val1,int val2,int t1,int t2);

#define NC_NA           (0)

#define getNoseconeLED()        ((OUTMOD_7==(TA0CCTL2&OUTMOD_7)?TA0CCR2:0))
#define getChuteLED()           ((OUTMOD_7==(TA0CCTL1&OUTMOD_7)?TA0CCR1:0))

#define NC_MAX_PWM     0XFFF

//invalid mode
#define NC_MODE_INVALID     (NC_MODE_NUM)

enum{NC_MODE_STATIC=0,NC_MODE_FADE,NC_MODE_FLASH,NC_MODE_ONE_SHOT,NC_MODE_PATTERN,NC_MODE_NUM};


typedef struct{
    int mode;
    int val1,val2;
    int t1,t2;
} LED_PARAMS;

#endif /* NOSECONE_H_ */

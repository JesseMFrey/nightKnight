/*
 * Nosecone.c
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include "Nosecone.h"

void init_Nosecone(void)
{

    //TA2.2 and TA2.1 output pins
    P2DIR|=BIT4|BIT5;
    P2SEL|=BIT4|BIT5;


    //set timer period
    TA2CCR0=MAX_PWM;

    //Set TA2.1 to output PWM
    TA2CCTL1=OUTMOD_3;
    //set TA2.1 period to 50%
    TA2CCR1=MAX_PWM/2;


    //Set TA2.2 to output PWM
    TA2CCTL2=OUTMOD_3;
    //set TA2.1 period to 50%
    TA2CCR2=MAX_PWM/2;

    //setup TA2 to run in up mode for PWM
    TA2CTL=TASSEL_2|ID_3|MC_1|TACLR;
}


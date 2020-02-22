/*
 * Nosecone.c
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include <stdint.h>
#include "Nosecone.h"

void init_Nosecone(void)
{

    //TA2.2 and TA2.1 output pins
    P2DIR |=BIT1|BIT2;
    P2SEL0|=BIT1|BIT2;


    //set timer period
    TA0CCR0=NC_MAX_PWM;

    set_nosecone(200);
    set_chute(0);

    //setup TA0 to run in up mode for PWM
    TA0CTL=TASSEL_2|ID_3|MC_1|TACLR;
}

void set_chute(uint16_t chute)
{
    if(chute==0)
    {
        //set output to out bit and set low
        TA0CCTL2=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA0CCR2=chute;
        //set output reset/set mode
        TA0CCTL2=OUTMOD_7;
    }
}


void set_nosecone(uint16_t cone)
{
    if(cone==0)
    {
        //set output to out bit and set low
        TA0CCTL1=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA0CCR1=cone;
        //set output reset/set mode
        TA0CCTL1=OUTMOD_7;
    }
}


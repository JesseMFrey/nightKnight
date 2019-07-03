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
    P2DIR|=BIT4|BIT5;
    P2SEL|=BIT4|BIT5;


    //set timer period
    TA2CCR0=NC_MAX_PWM;

    set_nosecone(MAX_PWM/4);
    set_chute(0);

    //setup TA2 to run in up mode for PWM
    TA2CTL=TASSEL_2|ID_3|MC_1|TACLR;
}

void set_chute(uint16_t chute)
{
    if(chute==0)
    {
        //set output to out bit and set low
        TA2CCTL1=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA2CCR1=chute;
        //set output reset/set mode
        TA2CCTL1=OUTMOD_7;
    }
}


void set_nosecone(uint16_t cone)
{
    if(cone==0)
    {
        //set output to out bit and set low
        TA2CCTL2=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA2CCR2=cone;
        //set output reset/set mode
        TA2CCTL2=OUTMOD_7;
    }
}


/*
 * Nosecone.c
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include <stdint.h>
#include "Nosecone.h"

#define FADE_COUNT      1
#define FADE_INCR       1

#define MIN_FADE        400
#define MAX_FADE        MAX_PWM

enum{DIR_DOWN=0,DIR_UP=1};

int dir=DIR_DOWN;
int count=0;
uint16_t val=MAX_FADE;

void init_Nosecone(void)
{

    //TA2.2 and TA2.1 output pins
    P2DIR|=BIT4|BIT5;
    P2SEL|=BIT4|BIT5;


    //set timer period
    TA2CCR0=MAX_PWM;

    set_nosecone(0,MAX_PWM/4);

    //setup TA2 to run in up mode for PWM
    TA2CTL=TASSEL_2|ID_3|MC_1|TACLR;
    //enable CCR0 ISR
    TA2CCTL0=CCIE;
}

void set_nosecone(uint16_t chute,uint16_t cone)
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



// ============ TA2 ISR ============
// This is used for button debouncing
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER2_A0_VECTOR
__interrupt void PWM_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) PWM_ISR (void)
#else
#error Compiler not found!
#endif
{
    count+=1;
    if(count>=FADE_COUNT)
    {
        count=0;
        if(dir==DIR_UP)
        {
            if(val>=(MAX_FADE-FADE_INCR))
            {
                val=MAX_FADE;
                dir=DIR_DOWN;
            }
            else
            {
                val+=FADE_INCR;
            }
            set_nosecone(0,val);
        }
        else
        {
            if(val<(MIN_FADE+FADE_INCR))
            {
                val=MIN_FADE;
                dir=DIR_UP;
            }
            else
            {
                val-=FADE_INCR;
            }
            set_nosecone(0,val);
        }
    }
}


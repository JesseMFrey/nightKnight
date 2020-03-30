/*
 * Nosecone.c
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include <stdint.h>
#include "Nosecone.h"

static struct {
    int mode;
    int val1,val2;
    int t1,t2;
    int count;
    int state;
    int dir;
} nosecone_status;//,chute_status;

enum{NC_DIR_UP=0,NC_DIR_DOWN};



void set_chute(uint16_t chute)
{
    if(chute<=5)
    {
        //set output to out bit and set low
        TA0CCTL1=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA0CCR1=chute;
        //set output reset/set mode
        TA0CCTL1=OUTMOD_7;
    }
}


static inline void set_nosecone(uint16_t cone)
{
    if(cone<=5)
    {
        //set output to out bit and set low
        TA0CCTL2=OUTMOD_0;
    }
    else
    {
        //set chute period
        TA0CCR2=cone;
        //set output reset/set mode
        TA0CCTL2=OUTMOD_7;
    }
}


void init_Nosecone(void)
{

    //TA2.2 and TA2.1 output pins
    P2DIR |=BIT1|BIT2;
    P2SEL0|=BIT1|BIT2;


    //set timer period
    TA0CCR0=NC_MAX_PWM;

    nosecone_mode(NC_MODE_FADE,0,200,10,60);
    //nosecone_mode(NC_MODE_FLASH,0,NC_MAX_PWM,200,100);
    set_chute(0);

    //setup TA0 to run in up mode for PWM
    TA0CTL=TASSEL_2|ID_3|MC_1|TACLR|TAIE;

    //enable interrupts for CCR0
    //TA0CCTL0|=CCIE;
}

int nosecone_mode(int mode,int val1,int val2,int t1,int t2)
{
    //put in static mode while mode is being set
    nosecone_status.mode=NC_MODE_STATIC;
    //mode based init
    switch(mode)
    {
    case NC_MODE_STATIC:
        //nothing to do here
        break;
    case NC_MODE_FADE:
        nosecone_status.dir=NC_DIR_UP;
        nosecone_status.state=val1;
        nosecone_status.count=0;
        break;
    case NC_MODE_FLASH:
        nosecone_status.state=0;
        nosecone_status.count=0;
        break;
    case NC_MODE_ONE_SHOT:
        nosecone_status.state=0;
        nosecone_status.count=0;
        break;
    default:
        //unknown mode
        return 1;
    }
    //write values to structure
    nosecone_status.val1=val1;
    nosecone_status.val2=val2;
    nosecone_status.t1=t1;
    nosecone_status.t2=t2;
    //set initial value
    set_nosecone(val1);
    //set mode
    nosecone_status.mode=mode;
    return 0;
}


/*#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void nosecone_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) nosecone_ISR (void)
#else
#error Compiler not found!
#endif*/
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void nosecone_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) nosecone_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(__even_in_range(TA0IV,TA0IV_TAIFG))
    {
    case TA0IV_TAIFG:
        //increment count
        nosecone_status.count+=1;
        switch(nosecone_status.mode)
        {
        //case NC_MODE_STATIC:
            //nothing to do here
            //break;
        case NC_MODE_FADE:
            if(nosecone_status.count>=nosecone_status.t2)
            {
                nosecone_status.count=0;
                if(nosecone_status.dir==NC_DIR_UP)
                {
                    //increment state
                    nosecone_status.state+=nosecone_status.t1;
                    //check if we have gone over val2
                    if(nosecone_status.state>=nosecone_status.val2)
                    {
                        nosecone_status.state=nosecone_status.val2;
                        nosecone_status.dir=NC_DIR_DOWN;
                    }
                }
                else
                {
                    //Decrement state
                    nosecone_status.state-=nosecone_status.t1;
                    //check if we have gone below val1
                    if(nosecone_status.state<=nosecone_status.val1)
                    {
                        nosecone_status.state=nosecone_status.val1;
                        nosecone_status.dir=NC_DIR_UP;
                    }
                }
                //set new value
                set_nosecone(nosecone_status.state);
            }
            break;
        case NC_MODE_FLASH:
            if(nosecone_status.state==0)
            {
                if(nosecone_status.count>=nosecone_status.t1)
                {
                    nosecone_status.count=0;
                    nosecone_status.state=1;
                    set_nosecone(nosecone_status.val2);
                }
            }
            else
            {
                if(nosecone_status.count>=nosecone_status.t2)
                {
                    nosecone_status.count=0;
                    nosecone_status.state=0;
                    set_nosecone(nosecone_status.val1);
                }
            }
            break;
        case NC_MODE_ONE_SHOT:
            if(nosecone_status.state==0)
            {
                //check if we have timed out
                if(nosecone_status.count>=nosecone_status.t1)
                {
                    //set new val
                    set_nosecone(nosecone_status.val2);
                    //disable further changes
                    nosecone_status.state=1;
                }
            }
            break;
        }
        break;
    }
}

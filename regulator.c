/*
 * regulator.c
 *
 *  Created on: Feb 26, 2020
 *      Author: jesse
 */

#include <msp430.h>
#include "regulator.h"
#include "flashPattern.h"

int reg_flags=0;

void init5Vreg(void)
{

    //setup regulator PG input
    P1DIR&=~BIT6;
    P1REN|= BIT6;
    P1OUT|= BIT6;

    //setup regulator enable pin
    P1OUT&=~BIT7;
    P1REN&=~BIT7;

    //setup regulator PG interrupt
    P1IES|= BIT6;
    P1IFG&=~BIT6;
    P1IE |= BIT6;

}


void reg5V_on(void)
{
    //set startup flag
    reg_flags|=REG_FLAGS_STARTUP;
    P1OUT|= BIT7;
    //stop timer
    TA3CTL=MC__STOP;
    //set interrupt interval ~1 ms
    TA3CCR0=33;
    //enable interrupt for CCR0
    TA3CCTL0=CCIE;
    //start timer
    TA3CTL=TASSEL__ACLK|MC__CONTINUOUS|TACLR;
}


// ============ TA3.0 ISR ============

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER3_A0_VECTOR
__interrupt void PG_inhibit (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER3_A0_VECTOR))) PG_inhibit (void)
#else
#error Compiler not found!
#endif
{
    //disable further interrupts
    TA3CCTL0=0;
    //clear startup flag
    reg_flags&=~REG_FLAGS_STARTUP;
    //check PG pin
    if(!(P1IN&BIT6))
    {
        //set error flag
        reg_flags|=REG_FLAGS_ERROR;
        panic(LED_PAT_POWER_PANIC);
    }
}

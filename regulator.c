/*
 * regulator.c
 *
 *  Created on: Feb 26, 2020
 *      Author: jesse
 */

#include <msp430.h>
#include "regulator.h"

void init5Vreg(void)
{

    //setup regulator PG input
    P1DIR&=~BIT6;
    P1REN|= BIT6;
    P1OUT|= BIT6;

    //setup regulator enable pin
    P1OUT&=~BIT7;
    P1REN&=~BIT7;
    P1DIR|= BIT7;

    //setup regulator PG interrupt
    P1IES|= BIT6;
    P1IFG&=~BIT6;
    P1IE |= BIT6;

}


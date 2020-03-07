/*
 * switches.c
 *
 *  Created on: Feb 26, 2020
 *      Author: jesse
 */

#include <msp430.h>
#include "switches.h"

#define DIP_PINS    (BIT2|BIT3|BIT4|BIT5)
#define DIP_SHIFT   (1)

void setupDIP(void)
{
    //setup DIP switch pins for input
    P1DIR&=~DIP_PINS;
    P1REN|= DIP_PINS;
    P1OUT|= DIP_PINS;
}

unsigned int readDIP(void)
{
    unsigned int val=P1IN&DIP_PINS;
    return val>>DIP_SHIFT;
}

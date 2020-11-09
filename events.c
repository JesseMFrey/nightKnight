/*
 * events.c
 *
 *  Created on: Jul 1, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include "events.h"

e_type e_flags;

e_type e_get_clear(void)
{
    e_type tmp;

    __disable_interrupt();
    //get flags
    tmp=e_flags;

    e_flags&=~tmp;
    __enable_interrupt();

    return tmp;
}

//go into LPM0 only if there are no events
void LPM0_check(void)
{
    __disable_interrupt();
    //check if there are events to process
    if(!e_flags)
    {
        // Enter LPM0
        __bis_SR_register(LPM0_bits + GIE);
        _NOP();
    }
}

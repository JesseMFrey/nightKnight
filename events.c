/*
 * events.c
 *
 *  Created on: Jul 1, 2019
 *      Author: jesse
 */

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

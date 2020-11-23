/*
 * settings.c
 *
 *  Created on: Nov 16, 2020
 *      Author: jesse
 */

#include "flashPattern.h"
#include "settings.h"

#include <msp430.h>


const SETTINGS defaults={.color={.brt=31,.r=244,.g=244,.b=244},.pattern=LED_PAT_LIST_PARTICLE,.value=7,.list=&RNBW_colors};

#pragma DATA_SECTION(fl_settings, ".infoD")

const FL_SETTINGS fl_settings;


void init_settings(void)
{
    //check magic to see if we have valid settings
    if(fl_settings.magic!=SETTINGS_MAGIC)
    {
        write_settings(&defaults);
    }
}

void write_settings(const SETTINGS *settings)
{
    unsigned short state;
    //get pointer to settings structure in flash
    //this will be used for write so, it is not const
    void *sect=(void*)&fl_settings;
    //src and dest for copy
    const char *src;
    char *dest;
    //loop counter
    int i;

    // Stop watchdog timer
    WDTCTL=WDTPW|WDTHOLD;

    //save interrupts so they can be restored
    state=__get_interrupt_state();
    //disable inturrupts for write
    __disable_interrupt();

    //erase flash memory info D segment

    //wait while flash is busy
    while(FCTL3&BUSY);
    //clear lock
    FCTL3=FWPW;
    //setup for the erase
    FCTL1=FWPW|ERASE;
    //dummy write to the section
    *(int*)sect=0;
    //wait till flash is not busy
    while(FCTL3&BUSY);
    //re-lock flash
    FCTL3=FWPW|LOCK;

    //write settings to flash
    //clear lock
    FCTL3=FWPW;
    //setup for the write
    FCTL1=FWPW|WRT;

    //write "magic" value
    ((FL_SETTINGS*)sect)->magic=SETTINGS_MAGIC;

    //setup source and destination pointers
    src=(const char*)settings;
    dest=(char*)&fl_settings.set;

    //write settings into flash
    for(i=0;i<sizeof(SETTINGS);i++)
    {
        //copy value
        *dest++=*src++;
    }


    //disable write
    FCTL1=FWPW;
    //re-lock flash
    FCTL3=FWPW|LOCK;

    //restore interrupts
    __set_interrupt_state(state);


}


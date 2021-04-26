/*
 * settings.c
 *
 *  Created on: Nov 16, 2020
 *      Author: jesse
 */

#include "flashPattern.h"
#include "settings.h"

#include <msp430.h>

#include <stdio.h>


const SETTINGS defaults={.color={.brt=31,.r=244,.g=244,.b=244},.pattern=LED_PAT_LIST_PARTICLE,.value=7,.list=&RNBW_colors,.alt=160};

#pragma DATA_SECTION(fl_settings, ".infoD")

const FL_SETTINGS fl_settings;

SETTINGS settings;

int settings_valid(void)
{
    return (fl_settings.magic==0xA5A3);
}

void init_settings(void)
{
    const SETTINGS *src;
    //check magic to see if we have valid settings
    if(fl_settings.magic!=SETTINGS_MAGIC)
    {
        printf("Invalid magic 0x%04X\r\n",fl_settings.magic);
        src=&defaults;
    }
    else
    {
        printf("Settings found!\r\n");
        src=&fl_settings.set;
    }
    memcpy(&settings,src,sizeof(SETTINGS));
}

void erase_settings(void)
{
    //get pointer to settings structure in flash
    //this will be used for write so, it is not const
    void *sect=(void*)&fl_settings;
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
}

void write_settings(void)
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
    erase_settings();

    //write settings to flash
    //clear lock
    FCTL3=FWPW;
    //setup for the write
    FCTL1=FWPW|WRT;

    //write "magic" value
    ((FL_SETTINGS*)sect)->magic=SETTINGS_MAGIC;

    //setup source and destination pointers
    src=(const char*)&settings;
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


    //check magic to see if we have valid settings
    if(fl_settings.magic!=SETTINGS_MAGIC)
    {
        //copy new settings
        memcpy(&settings,&fl_settings.set,sizeof(SETTINGS));
    }

}


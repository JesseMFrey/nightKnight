/*
 * comands.c
 * Terminal command functions
 *
 *  Created on: Aug 25, 2017
 *      Author: jmf6
 */

#include "terminal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hal.h"

#include "USBprintf.h"
#include "LEDs.h"


const struct{
    int port,pin;
}leds[2]={{GPIO_PORT_P1,GPIO_PIN0},{GPIO_PORT_P4,GPIO_PIN7}};

int LED_Cmd(int argc,char *argv[]){
    int i,end_len;
    char brightness=0x1F;

    //send start sequence
    for(i=0;i<4;i++){
        LEDSendByte(0);
    }

    for(i=0;i<NUM_LEDS;i++)
    {
        //send brightness
        LEDSendByte(0xE0|brightness);
        //send blue
        LEDSendByte(((i%3)==0)?0xFF:0);
        //send green
        LEDSendByte(((i%3)==1)?0xFF:0);
        //send red
        LEDSendByte(((i%3)==2)?0xFF:0);
    }


    //calculate number of end bytes
    end_len=((NUM_LEDS-1)+8)/16;

    //send end frame
    for(i=0;i<end_len;i++)
    {
        LEDSendByte(0);
    }

    return 0;
}


//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]\r\n\t""get a list of commands or help on a spesific command.",helpCmd},
                          {"LED","number state\r\n\t""set the LED status",LED_Cmd},
                          //end of list
                          {NULL,NULL,NULL}};

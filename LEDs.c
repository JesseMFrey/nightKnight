/*
 * LEDs.c
 *
 *  Created on: Dec 27, 2018
 *      Author: jesse
 */

#include <msp430.h>
#include "LEDs.h"


void initLEDs(void)
{

    int i,end_len;
    char brightness=0x1F;


    //setup SPI port
    UCB0CTL0=UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC;
    UCB0CTL1=UCSSEL_2|UCSWRST;
    //set clock rate to 1MHz
    //UCB0BRW=25;
    //set clock rate to 5MHz
    UCB0BRW=5;

    //setup pins
    P3SEL|=BIT2|BIT0;

    //take peripheral out of reset mode
    UCB0CTLW0&=~UCSWRST;

    //send start sequence
    for(i=0;i<4;i++){
        LEDSendByte(0);
    }

    for(i=0;i<NUM_LEDS;i++)
    {
        //send brightness
        LEDSendByte(0xE0|brightness);
        //send blue
        LEDSendByte(((i%4)==3)?0xFF:((i%4)==0)?0xFF:0);
        //send green
        LEDSendByte(((i%4)==3)?0xFF:((i%4)==1)?0xFF:0);
        //send red
        LEDSendByte(((i%4)==3)?0xFF:((i%4)==2)?0xFF:0);
    }


    //calculate number of end bytes
    end_len=((NUM_LEDS-1)+8)/16;

    //send end frame
    for(i=0;i<end_len;i++)
    {
        LEDSendByte(0);
    }

}

//Send one byte via SPI
unsigned char LEDSendByte(const unsigned char data)
{
    volatile int t;//temp var
    while(!(UCB0IFG&UCTXIFG));    // wait while not ready for TX
    //dummy read to clear flags
    t=UCB0RXBUF;
    //write data
    UCB0TXBUF=data;
    while(!(UCB0IFG&UCTXIFG));    // wait for RX buffer (full)
    return (UCB0RXBUF);
}

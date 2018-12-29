/*
 * LEDs.c
 *
 *  Created on: Dec 27, 2018
 *      Author: jesse
 */

#include <msp430.h>
#include <string.h>         //for memset
#include "LEDs.h"


void initLEDs(void)
{

    int i;
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


    //clear LED array
    memset(LED_stat,0,sizeof(LED_stat));



    for(i=0;i<NUM_LEDS;i++)
    {

        //set brightness
        LED_stat[0].colors[i].brt=(0xE0|(i+1));
        //set blue
        LED_stat[0].colors[i].b=0xFF;//8*i;
        //set green
        LED_stat[0].colors[i].g=0;
        //set red
        LED_stat[0].colors[i].r=0xFF;

        //set blue
        LED_stat[0].colors[i].b=(((i%4)==3)?0xFF:((i%4)==0)?0xFF:0);
        //set green
        LED_stat[0].colors[i].g=(((i%4)==3)?0xFF:((i%4)==1)?0xFF:0);
        //set red
        LED_stat[0].colors[i].r=(((i%4)==3)?0xFF:((i%4)==2)?0xFF:0);
    }

    LEDs_send(&LED_stat[0]);

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



void LEDs_send(LED_array *dat)
{

    int i;
    unsigned char *ptr=(unsigned char*)dat;

    for(i=0;i<sizeof(LED_array);i++)
    {
        LEDSendByte(ptr[i]);
    }

}

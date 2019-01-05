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
    unsigned char *ptr=(unsigned char*)dat;

    //disable DMA
    DMA0CTL&=~DMAEN;

    // DMA trigger is SPI send
    DMACTL0 = DMA0TSEL_19;

    // Source DMA address: the data buffer.
    __data20_write_long((unsigned long)&DMA0SA,(unsigned long)(ptr+1));

    // Destination DMA address: the SPI send register.
    __data20_write_long((unsigned long)&DMA0DA,(unsigned long)&UCB0TXBUF);

    // The size of the block to be transferred, the first byte is transfered manualy so don't count it
    DMA0SZ = sizeof(LED_array)-1;
    // Configure the DMA transfer. single byte transfer with destination increment
    DMA0CTL = DMADT_0|DMASBDB|DMAEN|DMASRCINCR_3;

    //kick off transfer
    UCB0TXBUF=*ptr;

}

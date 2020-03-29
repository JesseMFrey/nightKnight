/*
 * LEDs.c
 *
 *  Created on: Dec 27, 2018
 *      Author: jesse
 */

#include <msp430.h>
#include <string.h>         //for memset
#include "LEDs.h"
#include "flashPattern.h"
#include "buttons.h"

//                                AUTO_BRT,BRT_MODE_LOWEST,BRT_MODE_LOW,BRT_MODE_MED_LOW,BRT_MODE_MED,BRT_MODE_MED_HIGH,BRT_MODE_HIGH,BRT_MODE_HIGHEST};
const unsigned char brt_values[]={       0,              1,           3,               5,          15,               20,           25,              31};

void initLEDs(void)
{

    //setup SPI port
    UCB0CTL0=UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC;
    UCB0CTL1=UCSSEL_2|UCSWRST;
    //set clock rate to 1MHz
    //UCB0BRW=25;
    //set clock rate to 5MHz
    UCB0BRW=5;

    //setup pins
    //P3.0 for DAT
    //P3.2 for CLK
    P3SEL|=BIT2|BIT0;

    //take peripheral out of reset mode
    UCB0CTLW0&=~UCSWRST;


    //clear LED array
    memset(LED_stat,0,sizeof(LED_stat));

    //set end values
    //memset(LED_stat[0].end,0xFF,sizeof(LED_stat[0].end));
    //memset(LED_stat[1].end,0xFF,sizeof(LED_stat[1].end));

    set_LED_int(flashPatternChange(LED_PAT_SATURATION));

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

    //clear DMA trigger
    DMACTL0 &= ~DMA0TSEL_31;

    // DMA trigger is SPI send
    DMACTL0 |= DMA0TSEL__USCIB0TX;

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

const int LED_lut[150][2]=  {
                            {16,0},
                            {15,0},
                            {14,0},
                            {13,0},
                            {12,0},
                            {11,0},
                            {10,0},
                            {9,0},
                            {8,0},
                            {7,0},
                            {6,0},
                            {5,0},
                            {4,0},
                            {3,0},
                            {2,0},
                            {1,0},
                            {0,0},
                            {0,1},
                            {1,1},
                            {2,1},
                            {3,1},
                            {4,1},
                            {5,1},
                            {6,1},
                            {7,1},
                            {8,1},
                            {9,1},
                            {10,1},
                            {11,1},
                            {12,1},
                            {13,1},
                            {14,1},
                            {15,1},
                            {16,1},
                            {16,2},
                            {15,2},
                            {14,2},
                            {13,2},
                            {12,2},
                            {11,2},
                            {10,2},
                            {9,2},
                            {8,2},
                            {7,2},
                            {6,2},
                            {5,2},
                            {4,2},
                            {3,2},
                            {2,2},
                            {1,2},
                            {0,2},
                            {17,2},
                            {18,2},
                            {19,2},
                            {20,2},
                            {21,2},
                            {22,2},
                            {23,2},
                            {24,2},
                            {25,2},
                            {26,2},
                            {27,2},
                            {28,2},
                            {29,2},
                            {30,2},
                            {31,2},
                            {32,2},
                            {33,2},
                            {34,2},
                            {35,2},
                            {36,2},
                            {37,2},
                            {38,2},
                            {39,2},
                            {40,2},
                            {41,2},
                            {42,2},
                            {43,2},
                            {44,2},
                            {45,2},
                            {46,2},
                            {47,2},
                            {48,2},
                            {49,2},
                            {49,0},
                            {48,0},
                            {47,0},
                            {46,0},
                            {45,0},
                            {44,0},
                            {43,0},
                            {42,0},
                            {41,0},
                            {40,0},
                            {39,0},
                            {38,0},
                            {37,0},
                            {36,0},
                            {35,0},
                            {34,0},
                            {33,0},
                            {32,0},
                            {31,0},
                            {30,0},
                            {29,0},
                            {28,0},
                            {27,0},
                            {26,0},
                            {25,0},
                            {24,0},
                            {23,0},
                            {22,0},
                            {21,0},
                            {20,0},
                            {19,0},
                            {18,0},
                            {17,0},
                            {17,1},
                            {18,1},
                            {19,1},
                            {20,1},
                            {21,1},
                            {22,1},
                            {23,1},
                            {24,1},
                            {25,1},
                            {26,1},
                            {27,1},
                            {28,1},
                            {29,1},
                            {30,1},
                            {31,1},
                            {32,1},
                            {33,1},
                            {34,1},
                            {35,1},
                            {36,1},
                            {37,1},
                            {38,1},
                            {39,1},
                            {40,1},
                            {41,1},
                            {42,1},
                            {43,1},
                            {44,1},
                            {45,1},
                            {46,1},
                            {47,1},
                            {48,1},
                            {49,1}
                            };

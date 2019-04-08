/*
 * Companion.c
 *
 *  Created on: Apr 6, 2019
 *      Author: jesse
 */


#include <msp430.h>
#include "Companion.h"

//Board ID, just made this up
static const uint16_t companion_ID=0xA55A;

COMPANION_BUF cpRx,cpTx;

void init_Companion(void)
{
    //allow port mapping
    PMAPKEYID=PMAPKEY;

    P4MAP3 = PM_UCB1SCL;
    P4MAP0 = PM_UCB1STE;
    P4MAP2 = PM_UCB1SOMI;
    P4MAP1 = PM_UCB1SIMO;

    //allow reconfiguration
    PMAPCTL|=PMAPRECFG;
    //lock port maping with invalid key
    PMAPKEYID=0;

    //put UCB1 in reset mode
    UCB1CTL1=UCSWRST;

    //set up UCB1 for SPI
     UCB1CTL0=UCCKPH|UCMSB|UCMODE_2|UCSYNC;
     UCB1CTL1=UCSSEL_2|UCSWRST;
     //set clock rate to 1MHz
     UCB0BRW=25;
     //set clock rate to 5MHz
     //UCB0BRW=5;

     //setup pins
     P4SEL|=BIT0|BIT1|BIT2|BIT3;

     //take peripheral out of reset mode
     UCB1CTLW0&=~UCSWRST;

     //--setup companion message--

     //update every second
     cpTx.s.update_period=AO_SEC_TO_TICKS(1);

     //set ID
     cpTx.s.board_id        = companion_ID;
     cpTx.s.board_id_inverse=~companion_ID;

     //send back no data
     cpTx.s.channels=0;

     //--setup DMA 1 for SPI Rx--

     //disable DMA
     DMA1CTL&=~DMAEN;

     // DMA trigger is SPI Rx
     DMACTL0_H = DMA0TSEL__USCIB1RX;

     // Source DMA address: SPI RX register.
     __data20_write_long((unsigned long)&DMA0SA,(unsigned long)(&UCB1RXBUF));

     // Destination DMA address: buffer
     __data20_write_long((unsigned long)&DMA0DA,(unsigned long)(&cpRx));

     // Companion block size
     DMA1SZ = 8;
     // Configure the DMA transfer. single byte transfer with destination increment
     DMA1CTL = DMADT_0|DMASBDB|DMAEN|DMADSTINCR_3;

     //--setup DMA 2 for SPI Tx--

     //disable DMA
     DMA2CTL&=~DMAEN;

     // DMA trigger is SPI send
     DMACTL2_L = DMA2TSEL__USCIB1TX;

     // Source DMA address: buffer
     __data20_write_long((unsigned long)&DMA0SA,(unsigned long)(&cpTx));

     // Destination DMA address: SPI TX register
     __data20_write_long((unsigned long)&DMA0DA,(unsigned long)&UCB1TXBUF);

     // Companion block size
     DMA1SZ = 8;
     // Configure the DMA transfer. single byte transfer with destination increment
     DMA1CTL = DMADT_0|DMASBDB|DMAEN|DMADSTINCR_3;


}

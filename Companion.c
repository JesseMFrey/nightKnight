/*
 * Companion.c
 *
 *  Created on: Apr 6, 2019
 *      Author: jesse
 */


#include <msp430.h>
#include "Companion.h"
#include <string.h>

//Board ID, just made this up
#define BOARD_ID        1986

struct ao_companion_command cpCmd;
const struct ao_companion_setup cpSetup={BOARD_ID,~BOARD_ID,AO_SEC_TO_TICKS(1),TLM_ITEMS};

struct telemitry_dat cpTLM={};

//char to transmit if we have nothing to send
static const uint8_t dummy_Tx=0xA5;
//char to write to if we have nothing to receive
static uint8_t dummy_Rx;

static int cp_SPI_state;

//setup DMA for outbound transfer
static inline void cp_SPI_tx_setup(const void *buf,int size,int incr,int start)
{
    uint16_t incr_flags;
    unsigned long saddr=(unsigned long)(buf);

    if(incr)
    {
        incr_flags=DMASRCINCR_3;
    }
    else
    {
        incr_flags=0;
    }

    if(start)
    {
        size--;
        saddr++;
    }

    //disable DMA
    DMA2CTL&=~DMAEN;

    //clear DMA trigger
    DMACTL1 &= ~DMA2TSEL_31;

    // DMA trigger is SPI send
    DMACTL1 |= DMA2TSEL__USCIB1TX;

    // Source DMA address: buffer
    __data20_write_long((unsigned long)&DMA2SA,saddr);

    // Destination DMA address: SPI TX register
    __data20_write_long((unsigned long)&DMA2DA,(unsigned long)&UCB1TXBUF);

    // Companion block size
    DMA2SZ = size;
    // Configure the DMA transfer. single byte transfer with no increment
    DMA2CTL = DMADT_0|DMASBDB|DMAEN|DMAIE|incr_flags;

    if(start)
    {
        //kickoff transfer
        UCB1TXBUF=*((uint8_t*)buf);
    }

}

//setup DMA for inbound transfer
static inline void cp_SPI_rx_setup(void *buf,int size,int incr)
{
    uint16_t incr_flags;

    if(incr)
    {
        incr_flags=DMADSTINCR_3;
    }
    else
    {
        incr_flags=0;
    }

    //disable DMA
    DMA1CTL&=~DMAEN;

    //clear DMA trigger
    DMACTL0 &= ~DMA1TSEL_31;

    // DMA trigger is SPI Rx
    DMACTL0 |= DMA1TSEL__USCIB1RX;

    // Source DMA address: SPI RX register.
    __data20_write_long((unsigned long)&DMA1SA,(unsigned long)(&UCB1RXBUF));

    // Destination DMA address: buffer
    __data20_write_long((unsigned long)&DMA1DA,(unsigned long)(buf));

    // Companion block size
    DMA1SZ = size;
    // Configure the DMA transfer. single byte transfer with destination increment
    DMA1CTL = DMADT_0|DMASBDB|DMAEN|DMAIE|incr_flags;
}


void init_Companion(void)
{
    //setup interrupt for chip select
    /*P2DIR&=~BIT6;           //input
    P2IES&=~BIT6;           //rising edge triggered
    P2IFG&=~BIT6;           //clear flag
    P2IE |= BIT6;           //enable interrupt*/

    //allow port mapping
    PMAPKEYID=PMAPKEY;

    P4MAP3 = PM_UCB1CLK;
    P4MAP0 = PM_UCB1STE;
    P4MAP2 = PM_UCB1SOMI;
    P4MAP1 = PM_UCB1SIMO;

    //allow reconfiguration
    PMAPCTL|=PMAPRECFG;
    //lock port mapping with invalid key
    PMAPKEYID=0;

    //put UCB1 in reset mode
    UCB1CTL1=UCSWRST;

    //set up UCB1 for SPI
     UCB1CTL0=UCMSB|UCMODE_2|UCSYNC;
     UCB1CTL1=UCSSEL_2|UCSWRST;
     //set clock rate to 1MHz
     UCB0BRW=25;
     //set clock rate to 5MHz
     //UCB0BRW=5;

     //setup pins
     P4SEL|=BIT0|BIT1|BIT2|BIT3;

     //take peripheral out of reset mode
     UCB1CTLW0&=~UCSWRST;

     //set SPI data state
     cp_SPI_state=CP_COMMAND_RX;

     cp_SPI_rx_setup(&cpCmd,sizeof(cpCmd),DMA_INCR);
     cp_SPI_tx_setup(&dummy_Tx,sizeof(cpCmd),DMA_NO_INCR,DMA_START);

}

void companion_SPI_reset(void)
{
    //put peripheral in reset
    //UCB1CTLW0|= UCSWRST;
    //setup to receive command
    cp_SPI_rx_setup(&cpCmd,sizeof(cpCmd),DMA_INCR);
    cp_SPI_tx_setup(&dummy_Tx,sizeof(cpCmd),DMA_NO_INCR,DMA_START);
    //set next state
    cp_SPI_state=CP_COMMAND_RX;
    //take peripheral out of reset
    //UCB1CTLW0&=~UCSWRST;
}

int tx_bytes=0,rx_bytes=0;

/*
 * ======== DMA_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = DMA_VECTOR
__interrupt void DMA_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(DMA_VECTOR))) DMA_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(DMAIV)

    {
    //LED SPI DMA
    case DMAIV_DMA0IFG:
        break;
    //companion SPI RX DMA
    case DMAIV_DMA1IFG:
        switch(cp_SPI_state)
        {
            case CP_COMMAND_RX:
                //next state depends on command
                switch(cpCmd.command)
                {
                case AO_COMPANION_SETUP:
                    //send setup info
                    cp_SPI_state=CP_SETUP_TX;
                    break;
                case AO_COMPANION_FETCH:
                    //check if we have telemetry
                    if(sizeof(cpTLM)>0)
                    {
                        //send telemetry
                        cp_SPI_state=CP_TLM_TX;
                    }
                    else
                    {
                        cp_SPI_state=CP_COMMAND_RX;
                    }
                    break;
                case AO_COMPANION_NOTIFY:
                //no extra data to send
                cp_SPI_state=CP_COMMAND_RX;
                break;
                }
            case CP_SETUP_TX:
                //next SPI command Rx
                cp_SPI_state=CP_COMMAND_RX;
                break;
            case CP_TLM_TX:
                //next SPI command Rx
                cp_SPI_state=CP_COMMAND_RX;
                break;
        }
        //setup new DMA
        switch(cp_SPI_state)
        {
        case CP_COMMAND_RX:

            cp_SPI_rx_setup(&cpCmd,sizeof(cpCmd),DMA_INCR);

            break;
        case CP_SETUP_TX:

            cp_SPI_rx_setup(&dummy_Rx,sizeof(cpSetup),DMA_NO_INCR);

            break;
        case CP_TLM_TX:

            cp_SPI_rx_setup(&dummy_Rx,sizeof(cpTLM),DMA_NO_INCR);

            break;
        }
        break;
    //companion SPI TX DMA
    case DMAIV_DMA2IFG:
        //setup new DMA
        switch(cp_SPI_state)
        {
        case CP_COMMAND_RX:

            cp_SPI_tx_setup(&dummy_Tx,sizeof(cpCmd),DMA_NO_INCR,DMA_NO_START);

            break;
        case CP_SETUP_TX:

            cp_SPI_tx_setup(&cpSetup,sizeof(cpSetup),DMA_INCR,DMA_NO_START);

            break;
        case CP_TLM_TX:

            cp_SPI_tx_setup(&cpTLM,sizeof(cpTLM),DMA_INCR,DMA_NO_START);

            break;
        }
        break;
    }
}


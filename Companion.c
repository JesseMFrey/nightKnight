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
#define BOARD_ID        0xAFF5

struct ao_companion_command cpCmd;
const struct ao_companion_setup cpSetup={BOARD_ID,~BOARD_ID,AO_SEC_TO_TICKS(1),0};

struct telemitry_dat cpTLM;

//char to transmit if we have nothing to send
static const uint8_t dummy_Tx=0xA5;
//char to write to if we have nothing to receive
static uint8_t dummy_Rx;

static int cp_SPI_state;

static char* rx_ptr,*rx_end;
static const char* tx_ptr,*tx_end;

void SPI_rx_ptr_setup(void *rxptr,size_t size)
{

    //set tx pointer
    rx_ptr=rxptr;

    //setup end pointer
    rx_end=((char*)rxptr)+sizeof(cpCmd);
}

void SPI_tx_ptr_setup(const void *txptr,size_t size)
{

    //set tx pointer
    tx_ptr=txptr;

    //setup end pointer
    tx_end=((char*)txptr)+sizeof(cpCmd);
}

void SPI_ptr_setup(void *rxptr,const void *txptr,size_t size)
{
    SPI_tx_ptr_setup(txptr,size);
    SPI_rx_ptr_setup(rxptr,size);
}

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
    //lock port mapping with invalid key
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

     //setup SPI buffers
     SPI_ptr_setup(&cpCmd,NULL,sizeof(cpCmd));

     //enable interrupts
     UCB0IE|=UCTXIE|UCRXIE;


     //set SPI data state
     cp_SPI_state=CP_COMMAND_RX;

}


/*
 * ======== SPI_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void Companion_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) Companion_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(UCB0IV)

    {
    case USCI_UCRXIFG:
        if(rx_ptr==NULL){
            rx_end--;
        }else{
            *(rx_ptr++)=UCB0RXBUF;
        }
        //check if we have ended
        if(rx_ptr>=rx_end){
            //setup next transaction
            //state has already been set
            switch(cp_SPI_state)
            {
            case CP_COMMAND_RX:
                //setup to receive command
                SPI_rx_ptr_setup(&cpCmd,sizeof(cpCmd));
                break;
            case CP_SETUP_TX:
                //setup for setup Tx, send dummy bytes
                SPI_rx_ptr_setup(NULL,sizeof(cpSetup));
                break;
            case CP_TLM_TX:
                //setup for TLM tx, send dummy bytes
                SPI_rx_ptr_setup(NULL,sizeof(cpCmd));
                break;
            }
        }
        break;
    case USCI_UCTXIFG:
        if(tx_ptr==NULL){
            UCB0TXBUF=dummy_Tx;
            tx_end--;
        }else{
            UCB0TXBUF=*(tx_ptr++);
        }
        //check if we have ended
        if(tx_ptr>=tx_end){
            //setup next transaction
            switch(cp_SPI_state)
            {
            case CP_COMMAND_RX:
                //check what command we got
                switch(cpCmd.command)
                {
                    case AO_COMPANION_SETUP:
                        //send command
                        SPI_tx_ptr_setup(&cpSetup,sizeof(cpSetup));
                        //set next state
                        cp_SPI_state=CP_SETUP_TX;
                        break;
                    case AO_COMPANION_FETCH:
                        //send command
                        SPI_tx_ptr_setup(&cpTLM,sizeof(cpTLM));
                        //set next state
                        cp_SPI_state=CP_TLM_TX;
                        break;
                    case AO_COMPANION_NOTIFY:
                        //send command
                        SPI_tx_ptr_setup(NULL,sizeof(cpCmd));
                        //set next state
                        cp_SPI_state=CP_COMMAND_RX;
                        break;
                }
                break;
            case CP_SETUP_TX:
            case CP_TLM_TX:
                //send command
                SPI_tx_ptr_setup(NULL,sizeof(cpCmd));
                //set next state
                cp_SPI_state=CP_COMMAND_RX;
                break;
            }
        }
        break;
    }
}

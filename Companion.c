/*
 * Companion.c
 *
 *  Created on: Apr 6, 2019
 *      Author: jesse
 */


#include <msp430.h>
#include "Companion.h"
#include <string.h>
#include "events.h"

//Board ID, just made this up
#define BOARD_ID        1986

struct ao_companion_command cpCmd;
const struct ao_companion_setup cpSetup={BOARD_ID,~BOARD_ID,AO_SEC_TO_TICKS(1)/10,TLM_ITEMS};

struct telemitry_dat cpTLM={};

#define DUMMY_TX (0x05)
//char to write to if we have nothing to receive
static volatile uint8_t dummy_Rx;

static int cp_SPI_state;

static char* rx_ptr,*rx_end;
static const char* tx_ptr,*tx_end;

static void SPI_rx_ptr_setup(void *rxptr,size_t size)
{

    //set tx pointer
    rx_ptr=rxptr;

    //setup end pointer
    rx_end=((char*)rxptr)+size;
}

static void SPI_tx_ptr_setup(const void *txptr,size_t size)
{

    //set tx pointer
    tx_ptr=txptr;

    //setup end pointer
    tx_end=((char*)txptr)+size;
}

void SPI_ptr_setup(void *rxptr,const void *txptr,size_t size)
{
    SPI_tx_ptr_setup(txptr,size);
    SPI_rx_ptr_setup(rxptr,size);
}

void init_Companion(void)
{
    //pull down on chip select
    P2DIR&=~BIT5;           //input
    P2OUT&=~BIT5;
    P2REN|= BIT5;
    //setup interrupt for chip select
    P2DIR&=~BIT5;           //input
    P2IES&=~BIT5;           //rising edge triggered
    P2IFG&=~BIT5;           //clear flag
    P2IE |= BIT5;           //enable interrupt


    /*Cmpanion connector pinout

    Companion   KN pin      Function
    pin
    1           Gnd
    2           4.4         SCK
    3           4.5         MOSI
    4           4.7         MISO
    5           4.6,2.5     CS
    6           NC
    7           tm_vcc
    8           NC      */

    //put UCB1 in reset mode
    UCB1CTLW0|=UCSWRST;

    //set up UCB1 for SPI
     UCB1CTLW0=UCCKPH|UCMSB|UCMODE_2|UCSYNC|UCSSEL_2|UCSWRST;
     //set clock rate to 1MHz
     UCB1BRW=25;
     //set clock rate to 5MHz
     //UCB1BRW=5;

     //setup pins
     P4DIR &=~(BIT4|BIT5|BIT6|BIT7);
     P4SEL0|=  BIT4|BIT5|BIT6|BIT7;

     //setup port mapping


     //allow port mapping
     PMAPKEYID=PMAPKEY;

     P4MAP4 = PM_UCB1CLK;
     P4MAP5 = PM_UCB1SIMO;
     P4MAP6 = PM_UCB1STE;
     P4MAP7 = PM_UCB1SOMI;

     //allow reconfiguration
     PMAPCTL|=PMAPRECFG;
     //lock port mapping with invalid key
     PMAPKEYID=0;


     //take peripheral out of reset mode
     UCB1CTLW0&=~UCSWRST;

     //setup SPI buffers
     SPI_ptr_setup(&cpCmd,NULL,sizeof(cpCmd));

     //enable interrupts
     UCB1IE|=UCTXIE|UCRXIE;


     //set SPI data state
     cp_SPI_state=CP_COMMAND_RX;

}

void companion_SPI_reset(void)
{
    //disable pins
    P4SEL0|=~(BIT4|BIT5|BIT6|BIT7);
    //disable interrupts
    UCB1IE&=~(UCTXIE|UCRXIE);
    //put peripheral in reset
    UCB1CTLW0|= UCSWRST;
    //put peripheral in master mode
    UCB1CTLW0|= UCMST;
    //setup to receive command
    SPI_rx_ptr_setup(&cpCmd,sizeof(cpCmd));
    SPI_tx_ptr_setup(NULL,sizeof(cpCmd));
    //set next state
    cp_SPI_state=CP_COMMAND_RX;
    //put peripheral in slave mode
    UCB1CTLW0&=~UCMST;
    //enable pins
    P4SEL0|=  BIT4|BIT5|BIT6|BIT7;
    //take peripheral out of reset
    UCB1CTLW0&=~UCSWRST;
    //enable interrupts
    UCB1IE|=UCTXIE|UCRXIE;
}

/*
 * ======== SPI_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void Companion_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) Companion_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(UCB1IV)

    {
    case USCI_SPI_UCRXIFG:
        if(rx_ptr==NULL){
            rx_end--;
            dummy_Rx=UCB1RXBUF;
        }else{
            *(rx_ptr++)=UCB1RXBUF;
        }
        //check if we have ended
        if(rx_ptr>=rx_end){
            //setup next transaction
            //state has already been set
            switch(cp_SPI_state)
            {
            case CP_SETUP_TX:
                //setup for setup Tx, send dummy bytes
                SPI_rx_ptr_setup(NULL,sizeof(cpSetup));
                break;
            case CP_TLM_TX:
                //setup for TLM tx, send dummy bytes
                SPI_rx_ptr_setup(NULL,sizeof(cpTLM));
                break;
            default:
            case CP_COMMAND_RX:
                //setup to receive command
                SPI_rx_ptr_setup(&cpCmd,sizeof(cpCmd));
                break;
            }
            //recived a command, exit LPM
            e_flags|=COMP_RX_CMD;
            LPM0_EXIT;
        }
        break;
    case USCI_SPI_UCTXIFG:
        if(tx_ptr==NULL){
            UCB1TXBUF=DUMMY_TX;
            //write twice see USCI40
            UCB1TXBUF=DUMMY_TX;
            tx_end--;
        }else{
            unsigned char tx_val=*(tx_ptr++);
            UCB1TXBUF=tx_val;
            //write twice see USCI40
            UCB1TXBUF=tx_val;
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
                    default:
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



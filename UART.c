/*
 * UART.c
 *
 *  Created on: Mar 22, 2020
 *      Author: jesse
 */

#include "UART.h"
#include <msp430.h>
#include <stdio.h>

//make Tx buffer big
#define TX_BUF_SIZE     (512)
#define RX_BUF_SIZE     (64)

static char tx_buf[TX_BUF_SIZE];
int tx_in,tx_out;

static int tx_done;

static char rx_buf[RX_BUF_SIZE];
int rx_in,rx_out;


static void enable_ints(void){
  //enable interrupts
  UCA2IE|=UCTXIE|UCRXIE;
}

static void disable_ints(void){
  //disable interrupts
  UCA2IE&=~(UCTXIE|UCRXIE);
}

void initUART(void)
{
    //setup UART operation
    UCA2CTLW0 =UCSWRST;
    UCA2CTLW0|=UCSSEL__ACLK;

    //clear buffer vars
    tx_in=tx_out=0;
    rx_in=rx_out=0;
    //nothing in buffer, set done
    tx_done=1;

    //set baud rate to 9600
    UCA2BRW=3;
    UCA2MCTLW=0x9200;

    //setup pins
    //P3.6 for RX
    //P3.7 for TX
    P3SEL0|=BIT6|BIT7;


    //take UCA2 out of reset mode
    UCA2CTLW0&=~UCSWRST;
    //enable UART interrupts
    enable_ints();

}

//queue byte to get transmitted
int TxChar(unsigned char c){
    int tmp_idx;
    int res=c;
    //check if transmitting
    if(tx_done){
        //bypass queue for first byte if not transmitting
        UCA2TXBUF=c;
        //clear done flag
        tx_done=0;
        //queue byte
    }else{
        tmp_idx=tx_in+1;
        //check for wraparound
        if(tmp_idx>=TX_BUF_SIZE)
        {
            tmp_idx=0;
        }
        //check if there is space
        if(tmp_idx==tx_out)
        {
            return EOF;
        }
        tx_buf[tx_in]=c;
        tx_in=tmp_idx;
    }
    //return result
    return res;
}

//get byte from buffer
int UART_Getc (void){
  int c;
  //receive a byte from the queue
  //TODO: posibly add timeout and the posibility to return EOF
  do
  {
      c=UART_CheckKey();
      //check if we got a value
      if(c==EOF)
      {
          //no, go to low power mode
          __bis_SR_register(LPM0_bits + GIE);
          _NOP();
      }
  }while(c!=EOF);
  //return byte from queue
  return c;
}

//get a byte from the buffer but don't wait
int UART_CheckKey (void){
    unsigned char c;
    int tmp_idx;
    if(rx_in!=rx_out)
    {
        //get char
        c=rx_buf[rx_out];

        //calculate new index
        tmp_idx=rx_out+1;
        //check for wraparound
        if(tmp_idx>=RX_BUF_SIZE)
        {
            tmp_idx=0;
        }
        //set new idx
        rx_out=tmp_idx;
        //return value from buffer
        return c;
    }
    else
    {
        return EOF;
    }
}

/*
 * ======== UART_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_A2_VECTOR
__interrupt void UART_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(USCI_A2_VECTOR))) UART_ISR (void)
#else
#error Compiler not found!
#endif
{
    unsigned char c;
    int tmp_idx;
    switch(UCA2IV)

    {
        case USCI_UART_UCRXIFG:
            //read a byte from UART
            c=UCA2RXBUF;
            //set byte in buffer
            rx_buf[rx_in]=c;
            //increment index
            tmp_idx=rx_in+1;
            //check for wraparound
            if(tmp_idx>=RX_BUF_SIZE)
            {
                //back to the beginning
                tmp_idx=0;
            }
            //check if buffer is full
            if(tmp_idx!=rx_out)
            {
                rx_in=tmp_idx;
            }
            //char received, exit LPM
            LPM0_EXIT;

        break;
        case USCI_UART_UCTXIFG:
            if (tx_in==tx_out)
            {
                //buffer empty, set flag
                tx_done=1;
            }
            else
            {
                //send char to UART
                UCA2TXBUF=tx_buf[tx_out];
                //decriment tx_out
                tx_out+=1;
                //check for wraparound
                if(tx_out>=TX_BUF_SIZE)
                {
                    //back to the beginning
                    tx_out=0;
                }
                //char sent, exit LPM so it can be filled
                LPM0_EXIT;
            }
        break;
        case USCI_UART_UCSTTIFG:
            //start byte received
        break;
        case USCI_UART_UCTXCPTIFG:
            //char transmitted
        break;
    }
}


/*
 * buttons.c
 *
 *  Created on: Sep 7, 2017
 *      Author: jmf6
 */

#include "buttons.h"
#include "driverlib.h"
#include "hal.h"
#include "LEDs.h"

//======== Buttons Init function ========

void Buttons_init(void){

    //setup p1.1 pull up
    P1DIR&=~BIT1;
    P1OUT|= BIT1;
    P1REN|= BIT1;
    //initialize p1.1 interrupt
    P1DIR&=~BIT1;
    P1IES|= BIT1;
    P1IFG|= BIT1;
    //clear P1 interrupt flags
    P1IFG = 0;
    //enable P1.1 interrupt
    P1IE |= BIT1;

    //set input divider expansion to /4
    TA0EX0=TAIDEX_3;
    //setup TA0 to run in continuous mode for debounce
    //set input divider to /8 for a total of /32
    TA0CTL=TASSEL_1|ID_3|MC_2|TACLR;
}

// ======== P1 ISR ========

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void button1_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(PORT1_VECTOR))) button1_ISR (void)
#else
#error Compiler not found!
#endif
{
    int i;
    switch(__even_in_range(P1IV,P1IV_P1IFG7)){
        case P1IV_P1IFG1:
            //check IES
            if(P1IES&BIT1){
                //toggle LED for button
                P1OUT^=BIT0;
                //set LED brightness to zero
                /*for(i=0;i<NUM_LEDS;i++)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=(0xE0);
                }
                //send new info
                LEDs_send(&LED_stat[0]);*/


                //setup TA0CCR1 to capture timer value
                TA0CCTL2=CM_3|CCIS_2|SCS|CAP|CCIE;
                //capture current timer value
                TA0CCTL2^=CCIS0;

            }
            //disable P1.1 interrupts
            P1IE&=~BIT1;
            //Toggle IES
            P1IES^=BIT1;
            //setup TA0CCR1 to capture timer value
            TA0CCTL1=CM_3|CCIS_2|SCS|CAP|CCIE;
            //capture current timer value
            TA0CCTL1^=CCIS0;
        break;
    }
}

unsigned char brt=0;
int brt_dir=1;

// ============ TA0 ISR ============
// This is used for button debouncing
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER0_ISR (void)
#else
#error Compiler not found!
#endif
{
    int i;
    switch(__even_in_range(TA0IV,TA0IV_TAIFG)){
        case TA0IV_TACCR1:
            if(TA0CCTL1&CAP){
                //add 100ms to capture time
                TA0CCR1+=102;
                //set to compare mode and enable interrupts
                TA0CCTL1=CCIE;
            }else{
                //disable timer interrupt
                TA0CCTL1=0;
                //clear P1.1 flags
                P1IFG&=~BIT1;
                //enable P1.1 interrupt
                P1IE |= BIT1;
            }
        break;
        case TA0IV_TACCR2:
            //next int in 100ms
            TA0CCR2+=102;
            //check if we are in capture mode
            if(TA0CCTL2&CAP){
                //set to compare mode and enable interrupts
                TA0CCTL2=CCIE;
            }
            //calculate new brightness
            brt+=brt_dir;
            //check for overflow
            if(!(brt&0x1F))
            {
                    if(brt_dir==1)
                    {
                        //reset to max
                        brt=0x1F;
                        //flip dir
                        brt_dir=-1;
                    }
                    else
                    {
                        //reset to min
                        brt=1;
                        //flip dir
                        brt_dir=1;
                    }
            }
            for(i=0;i<NUM_LEDS;i++)
            {
                //increase brightness
                LED_stat[0].colors[i].brt=0xE0|brt;
            }
            //send new info
            LEDs_send(&LED_stat[0]);
        break;
    }
}

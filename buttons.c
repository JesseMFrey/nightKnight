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
    //clear P1 interrupt flags
    P1IFG = 0;
    //enable P1.1 interrupt
    P1IE |= BIT1;

    //setup p1.1 pull up
    P2DIR&=~BIT1;
    P2OUT|= BIT1;
    P2REN|= BIT1;
    //initialize p1.1 interrupt
    P2DIR&=~BIT1;
    P2IES|= BIT1;
    //clear P2 interrupt flags
    P2IFG = 0;
    //enable P2.1 interrupt
    P2IE |= BIT1;

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
    switch(__even_in_range(P1IV,P1IV_P1IFG7)){
        case P1IV_P1IFG1:
            //check IES
            if(P1IES&BIT1){
                //toggle LED for button
                P4OUT^=BIT7;
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


// ======== P2 ISR ========

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void button2_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(PORT2_VECTOR))) button2_ISR (void)
#else
#error Compiler not found!
#endif
{
    int i;
    switch(__even_in_range(P2IV,P2IV_P2IFG7)){
        case P2IV_P2IFG1:
            //check IES
            if(P2IES&BIT1){
                //toggle LED for button
                P1OUT^=BIT0;
                //set LED brightness to zero
                for(i=0;i<NUM_LEDS;i++)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=(0xE0);
                }
                //send new info
                LEDs_send(&LED_stat[0]);

                //stop flash interrupts
                TA0CCTL3=0;
            }
            //disable P2.1 interrupts
            P2IE&=~BIT1;
            //Toggle IES
            P2IES^=BIT1;
            //setup TA0CCR2 to capture timer value
            TA0CCTL2=CM_3|CCIS_2|SCS|CAP|CCIE;
            //capture current timer value
            TA0CCTL2^=CCIS0;
        break;
    }
}

int LED_idx=0;
int idx_dir=1;

int limit_idx(int i)
{
    if(i<0)
        return 0;
    if(i>=LED_LEN)
        return LED_LEN-1;
    return i;
}

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
    int red_idx,blue_idx,green_idx;
    unsigned int s_even;
    switch(__even_in_range(TA0IV,TA0IV_TAIFG)){
        case TA0IV_TACCR1:
            if(TA0CCTL1&CAP){

                //set TA0CCR3 interrupt time
                TA0CCR3=TA0CCR1+102*2;
                //setup TA0CCR3 for compare interrupt
                TA0CCTL3=CCIE;

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
            if(TA0CCTL2&CAP)
            {
                //add 100ms to capture time
                TA0CCR2+=102;
                //set to compare mode and enable interrupts
                TA0CCTL2=CCIE;
            }
            else
            {
                //disable timer interrupt
                TA0CCTL2=0;
                //clear P1.1 flags
                P2IFG&=~BIT1;
                //enable P2.1 interrupt
                P2IE |= BIT1;
            }
        break;
        case TA0IV_TACCR3:
            //next int in 200ms
            TA0CCR3+=102*2;

            //calculate new index
            LED_idx+=idx_dir;
            //check for overflow
            if(LED_idx>=(LED_LEN))
            {
                //reset index
                LED_idx=(LED_LEN-1);
                //flip direction
                idx_dir=-1;
            }
            //check for
            if(LED_idx<=-3)
            {
                //reset index
                LED_idx=-2;
                //flip direction
                idx_dir=1;
            }
            green_idx=limit_idx(LED_idx);
            blue_idx =limit_idx(LED_idx+1);
            red_idx  =limit_idx(LED_idx+2);

            for(i=0;i<NUM_LEDS;i++)
            {
                //set to full brightness
                LED_stat[0].colors[i].brt=0xFF;
                //check if strip # is even
                s_even=(i/LED_LEN)&0x01;
                //set red
                LED_stat[0].colors[i].r=((i%LED_LEN)==(s_even?(LED_LEN-red_idx-1):red_idx))?0xFF:0;
                //set blue
                LED_stat[0].colors[i].b=((i%LED_LEN)==(s_even?(LED_LEN-blue_idx-1):blue_idx))?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=((i%LED_LEN)==(s_even?(LED_LEN-green_idx-1):green_idx))?0xFF:0;
            }
            //send new info
            LEDs_send(&LED_stat[0]);
        break;
    }
}

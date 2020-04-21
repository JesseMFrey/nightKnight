/*
 * pinInt.c
 *
 *  Created on: Feb 26, 2020
 *      Author: jesse
 */

#include <msp430.h>
#include "Companion.h"
#include "flashPattern.h"

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
            //Telemetrum switch interrupt
        break;
        case P1IV_P1IFG2:
            //DIP switch0 interrupt
        break;
        case P1IV_P1IFG3:
            //DIP switch1 interrupt
        break;
        case P1IV_P1IFG4:
            //DIP switch2 interrupt
        break;
        case P1IV_P1IFG5:
            //DIP switch3 interrupt
        break;
        case P1IV_P1IFG6:
            //5V regulator power good interrupt
            flashPatternChange(LED_PAT_PANIC);
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
    switch(__even_in_range(P2IV,P2IV_P2IFG7)){
        case P2IV_P2IFG5:
            //companion CS
            companion_SPI_reset();
        break;
    }
}


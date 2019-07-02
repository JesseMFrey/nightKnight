/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#include <string.h>

#include "driverlib.h"
#include "buttons.h"
#include "Companion.h"
#include "events.h"


/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */
#include "hal.h"


/*  
 * ======== main ========
 */
void main (void)
{
    e_type wake_e;

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    //setup debug pins
    P6OUT&=~(BIT0|BIT1|BIT2|BIT3);
    P6SEL&=~(BIT0|BIT1|BIT2|BIT3);
    P6REN&=~(BIT0|BIT1|BIT2|BIT3);
    P6DIR|= (BIT0|BIT1|BIT2|BIT3);

    PMM_setVCore(PMM_CORE_LEVEL_3);
    //USBHAL_initPorts();           // Config GPIOS for low-power (output low)
    USBHAL_initClocks(25000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    Buttons_init();
    init_Companion();


    //setup onboard LEDs
    P4OUT&=~BIT7;
    P1OUT&~BIT0;
    P4DIR|=BIT7;
    P1DIR|=BIT0;

    __enable_interrupt();  // Enable interrupts globally
    
    while (1)
    {
        // Enter LPM0
        __bis_SR_register(LPM0_bits + GIE);
        _NOP();
        //toggle P6.3
        P6OUT^=BIT3;
        //read interrupts
        wake_e=e_get_clear();
        if(wake_e&COMP_RX_CMD)
        {
            P6OUT&=~(BIT0|BIT1|BIT2);
            P6OUT!=(BIT0|BIT1|BIT2)&cpCmd.command;
            switch(cpCmd.flight_state)
            {
            case ao_flight_idle:
                //set LED's
                P4OUT|= BIT7;
                P1OUT&=~BIT0;
                break;
            case ao_flight_pad:
                //set LED's
                P4OUT&=~BIT7;
                P1OUT|= BIT0;
                break;
            default:
                //turn off LED's
                P4OUT&=~BIT7;
                P1OUT&=~BIT0;
                break;
            }
        }

    }  // while(1)
} // main()

/*  
 * ======== UNMI_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCS_clearFaultFlag(UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_DCOFFG);
            SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.
            SYSBERRIV = 0; //clear bus error flag

    }
}

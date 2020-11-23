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
#include "terminal.h"
#include "Nosecone.h"
#include "Companion.h"
#include "events.h"
#include "flashPattern.h"
#include "regulator.h"
#include "switches.h"
#include "ADC.h"
#include "UART.h"
#include "terminal.h"
#include <stdio.h>
#include "flightPattern.h"
#include "reset.h"
#include "settings.h"

/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */
#include "hal.h"

#include "LEDs.h"

int fputc(int _c, register FILE *_fp)
{
  return TxChar(_c);
}

int putchar(int _c)
{
  return TxChar(_c);
}

int fputs(const char *_ptr, register FILE *_fp)
{
    int len=0;
    while(*_ptr)
    {
        TxChar(*_ptr++);
        len+=1;
    }
    return len;
}


static TERM_DAT term;

/*  
 * ======== main ========
 */
void main (void)
{
    e_type wake_e;
    uint8_t lastState=ao_flight_invalid;
    int c;
    int expected_reset;
    //true if flash pattern has been updated
    int fp_done=0;

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    setupDIP();
    init5Vreg();
    expected_reset=init_reset();

    PMM_setVCore(PMM_CORE_LEVEL_3);
    USBHAL_initClocks(25000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    initLEDs();
    initUART();
    initADC();
    init_Nosecone();
    init_Companion();

    init_FlashPattern();

    init_settings();

    //check if this was unusual
    if(!expected_reset)
    {
        panic(LED_PAT_RESET_PANIC);
    }

    __enable_interrupt();  // Enable interrupts globally
    

    printf("NightKnight Ready!\r\n>");

    //initialize command vars
    terminal_init(&term);


    while (1)
    {
        //read interrupts
        wake_e=e_get_clear();
        if(wake_e&COMP_RX_CMD)
        {
            lastState=proc_flightP(&cpCmd,&patterns[0],lastState);
        }
        //advance flash pattern
        fp_done=flashPatternStep();

        c=UART_CheckKey();
        if(c!=EOF)
        {
            terminal_proc_char(c,&term);
        }
        else if(fp_done)
        {
            //go into LPM0 if flash pattern is updated and there are no event flags
            LPM0_check();
        }

    }  // while(1)
} // main()


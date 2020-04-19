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
    unsigned int maxSpeed=0;
    uint8_t lastState=ao_flight_invalid;
    int c;

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    setupDIP();
    init5Vreg();

    PMM_setVCore(PMM_CORE_LEVEL_3);
    USBHAL_initClocks(25000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    initLEDs();
    initUART();
    initADC();
    init_Nosecone();
    init_Companion();

    init_FlashPattern();


    __enable_interrupt();  // Enable interrupts globally
    

    printf("NightKnight Command mode\r\n>");

    //initialize command vars
    terminal_init(&term);

    //interactive loop for commandline
    while(cpCmd.flight_state<ao_flight_pad || cpCmd.flight_state>ao_flight_landed)
    {
        c=UART_CheckKey();
        if(c==EOF)
        {
            //no char from user, sleep
            // Enter LPM0
            __bis_SR_register(LPM0_bits + GIE);
            _NOP();
        }
        else
        {
            terminal_proc_char(c,&term);
        }
    }

    printf("Flight detected command mode exited\n");

    while (1)
    {
        // Enter LPM0
        __bis_SR_register(LPM0_bits + GIE);
        _NOP();
        //read interrupts
        wake_e=e_get_clear();
        if(wake_e&COMP_RX_CMD)
        {
            switch(cpCmd.flight_state)
            {
            case ao_flight_idle:
                //set LED's
                P4OUT&=~BIT7;
                P1OUT|= BIT0;
                if(lastState!=cpCmd.flight_state)
                {
                    //turn everything off
                    flashPatternChange(LED_PAT_OFF);
                    nosecone_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                break;
            case ao_flight_pad:
                if(lastState!=cpCmd.flight_state)
                {
                    flashPatternChange(LED_PAT_PAD);
                    nosecone_mode(NC_MODE_STATIC,400,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                break;
            case ao_flight_boost:
                if(lastState!=cpCmd.flight_state)
                {
                    flashPatternVC(LED_PAT_BOOST,0,LED_COLOR_BLUE);
                    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                break;
            case ao_flight_fast:
            case ao_flight_coast:
                if(lastState!=cpCmd.flight_state && lastState!=ao_flight_fast)
                {
                    maxSpeed=cpCmd.speed;
                    flashPatternVC(LED_PAT_GRAPH,0,LED_COLOR_RED);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                else
                {
                    flashPattern_setValue(50-(cpCmd.speed*50)/maxSpeed);
                }
                //set nosecone based on speed
                nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM*(cpCmd.speed/(float)maxSpeed),NC_NA,NC_NA,NC_NA);
                break;
            case ao_flight_drogue:
                if(lastState!=cpCmd.flight_state)
                {
                    flashPatternChange(LED_PAT_USA);
                    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                break;
            case ao_flight_main:
                if(lastState!=cpCmd.flight_state)
                {
                    flashPatternChange(LED_PAT_ST_USA);
                    nosecone_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,NC_MAX_PWM,NC_NA,NC_NA,NC_NA);
                }
                //turn off chute under 5m
                if(cpCmd.height<=10){
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM,NC_NA,NC_NA,NC_NA);
                }
                break;
            case ao_flight_landed:
                if(lastState!=cpCmd.flight_state)
                {
                    flashPatternChange(LED_PAT_USA);
                    nosecone_mode(NC_MODE_STATIC,300,NC_NA,NC_NA,NC_NA);
                    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                }
                break;
            default:
                //turn off chute
                chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
                break;
            }
            //set last state
            lastState=cpCmd.flight_state;
        }

    }  // while(1)
} // main()


/*
 * lightSensor.c
 *
 *  Created on: Aug 31, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include <string.h>
#include "lightSensor.h"
#include "filter.h"

unsigned int adc_vals[NUM_ADC_VALS];

int adc_idx;
int filt_dty=1;
unsigned short filt_val;

void init_light_sensor(void)
{

    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //10 k Rs so 3.46 μs sample time
    ADC12CTL0=ADC12SHT1_13|ADC12SHT0_13;
    //convert CH0 repeated single channel trigger from TB0.1
    ADC12CTL1=ADC12CSTARTADD_0|ADC12SHS_3|ADC12SHP|ADC12SSEL_0|ADC12CONSEQ_2;
    ADC12CTL2=ADC12PDIV|ADC12TCOFF|ADC12RES_2|ADC12SR|ADC12REFBURST;
    //setup MCTL0 for A0 with Vcc ref
    ADC12MCTL0=ADC12EOS|ADC12SREF_0|ADC12INCH_0;

    //enable interrupts for ADC
    ADC12IE=BIT0;

    //clear flags
    ADC12IFG=0;

    //setup P6.0 for ADC input
    P6SEL|=BIT0;

    //initialize ADC values
    memset(adc_vals,0,NUM_ADC_VALS);

    //zero adc index
    adc_idx=0;

    //setup TB0 for ADC timing at 4 sps
    TB0CTL=TBSSEL_1|ID_0|MC_0|TBCLR;
    TB0CCR0=8192;
    //on for 10 timer clocks
    TB0CCR1=10;
    //setup output for TB0.1
    TB0CCTL1=OUTMOD_7;
}

void light_sensor_start(void)
{
    //enable ADC
    ADC12CTL0|=ADC12ON|ADC12ENC;
    //start timer running
    TB0CTL|=MC_1|TBCLR;
}


void light_sensor_stop(void)
{
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //turn off ADC
    ADC12CTL0&=~ADC12ON;
    //stop timer
    TB0CTL&=~MC_3;
}

unsigned short light_sensor_get(void)
{
    if(filt_dty)
    {
        int s_idx=adc_idx,i,idx;
        //setup MAC
        MPY32CTL0=MPYDLY32|MPYDLYWRTEN|MPYM_1|MPYSAT;
        //initialize MAC by running first calc
        MPYS=adc_vals[s_idx];
        OP2=filter_coefs[0];

        for(i=1,idx=s_idx+1;idx!=s_idx;i++,idx++)
        {
            //wrap idx
            if(idx>=NUM_ADC_VALS){
                idx=0;
                //check for end condition
                if(idx==s_idx)
                {
                    break;
                }
            }
            //use MAC
            MACS=adc_vals[idx];
            OP2=filter_coefs[i];
        }
        filt_val=RESHI;
        filt_dty=0;
    }

    //return value from MAC
    return filt_val;
}


// ============ ADC12 ISR ============
// This is used for button debouncing
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(ADC12IV)
    {
    case ADC12IV_ADC12IFG0:
        //store value
        adc_vals[adc_idx]=ADC12MEM0;
        //move to next index
        adc_idx+=1;
        //wrap around
        if(adc_idx>=NUM_ADC_VALS)
        {
            adc_idx=0;
        }
        //set filter dirty
        filt_dty=1;
        break;
    }
}

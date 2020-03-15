/*
 * ADC.c
 *
 *  Created on: Mar 10, 2020
 *      Author: jesse
 */

#include <msp430.h>
#include <limits.h>
#include "Companion.h"

//#define SD24_read(ch)     {unsigned long val;val =(unsigned long)SD24BMEML##ch;val|=((unsigned long)SD24BMEMH##ch)<<16;val}
#define SD24_read(ch)       (SD24BMEMH##ch)


#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=SD24B_VECTOR
__interrupt void SD24_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(SD24B_VECTOR))) SD24_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(SD24BIV){
        case SD24BIV_SD24OVIFG:
            //clear interrupts
            SD24BIFG&=~(SD24OVIFG0|SD24OVIFG1|SD24OVIFG2|SD24OVIFG3|SD24OVIFG4|SD24OVIFG5|SD24OVIFG6);
            //set flag in telemitry structure
            cpTLM.flags|=TLM_ADC_OV;
        return;
        case SD24BIV_SD24TRGIFG:
        break;
        case SD24BIV_SD24IFG0:
            //Battery voltage conversion complete
            //read and store value
            cpTLM.Batt_V=SD24_read(0);
        return;
        case SD24BIV_SD24IFG1:
            //Battery current conversion complete
            //read and store value
            cpTLM.Batt_I=SD24_read(1);
        return;
        case SD24BIV_SD24IFG2:
            //LED voltage monitor conversion complete
            //read and store value
            cpTLM.LED_V=SD24_read(2);
        return;
        case SD24BIV_SD24IFG3:
            //3.3V voltage monitor conversion complete
            //read and store value
            cpTLM.MSP_V=SD24_read(3);
        return;
        case SD24BIV_SD24IFG4:
            //Power supply temperature conversion complete
            //read and store value
            cpTLM.Temp=SD24_read(4);
        return;
        case SD24BIV_SD24IFG5:
            //LED current sensor conversion complete
            //read and store value
            cpTLM.LED_I=SD24_read(5);
        return;
        case SD24BIV_SD24IFG6:
            //external sensor conversion complete
            //read value
            SD24_read(6);
            //TODO : data processing on sensor value
        return;
            //unknown interrupt
        default:
        break;
    }
}

#define SD24BCCTL_common        (SD24DF_1|SD24ALGN|SD24SCS__GROUP0)

void initADC(void)
{

  //setup reference
  REFCTL0  = REFMSTR|REFVSEL_3|REFON;
  //set ADC settings
  SD24BCTL0= SD24PDIV_1|SD24DIV1|SD24DIV2|SD24SSEL__SMCLK|SD24REFS;
  SD24BCTL1= 0;
  //setup ADCs

  //battery voltage ADC setup
  SD24BCCTL0 = SD24BCCTL_common;
  SD24BINCTL0= 0;
  SD24BOSR0  = 0xFF;
  SD24BPRE0  = 0;          //no preload

  //battery current ADC setup
  SD24BCCTL1 = SD24BCCTL_common;
  SD24BINCTL1= 0;
  SD24BOSR1  = 0xFF;
  SD24BPRE1  = 0;          //no preload

  //LED voltage monitor ADC setup
  SD24BCCTL2 = SD24BCCTL_common;
  SD24BINCTL2= 0;
  SD24BOSR2  = 0xFF;
  SD24BPRE2  = 0;          //no preload

  //3.3V voltage monitor ADC setup
  SD24BCCTL3 = SD24BCCTL_common;
  SD24BINCTL3= 0;
  SD24BOSR3  = 0xFF;
  SD24BPRE3  = 0;          //no preload

  //power supply temperature ADC setup
  SD24BCCTL4 = SD24BCCTL_common;
  SD24BINCTL4= 0;
  SD24BOSR4  = 0xFF;
  SD24BPRE4  = 0;          //no preload

  //LED current ADC setup
  SD24BCCTL5 = SD24BCCTL_common;
  SD24BINCTL5= 0;
  SD24BOSR5  = 0xFF;
  SD24BPRE5  = 0;          //no preload

  //External sensor ADC setup
  //SD24BCCTL6 = SD24BCCTL_common;
  SD24BCCTL6 = 0;   //not used, yet...
  SD24BINCTL6= 0;
  SD24BOSR6  = 0xFF;
  SD24BPRE6  = 0;          //no preload

  //clear interrupt flags
  SD24BIFG=0;
  //enable interrupts
  SD24BIE=SD24OVIE0|SD24IE0|
          SD24OVIE1|SD24IE1|
          SD24OVIE2|SD24IE2|
          SD24OVIE3|SD24IE3|
          SD24OVIE4|SD24IE4|
          SD24OVIE5|SD24IE5;

  //start conversions
  SD24BCTL1|=SD24GRP0SC;

}



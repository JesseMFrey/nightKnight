/*
 * flashPattern.c
 *
 *  Created on: Jan 3, 2019
 *      Author: jesse
 */

#include "LEDs.h"
#include "flashPattern.h"
#include "regulator.h"
#include <msp430.h>

#define C_RIGHT ((LED_LEN  )/2)
#define C_LEFT  ((LED_LEN-1)/2)

static int LED_idx=0;
static int LED_pattern;
static int idx_dir=0;

static unsigned short LED_int=102*2;

static LED_color pat_color;
unsigned int pat_val;

static int limit_idx(int i)
{
    if(i<0)
        return 0;
    if(i>=LED_LEN)
        return LED_LEN-1;
    return i;
}

void init_FlashPattern(void)
{

    //flash pattern timer interrupt
    TA1CCTL0=CCIE;

    //set flash pattern and interrupt period
    flashPatternChange(LED_PAT_SATURATION);

    //set input divider expansion to /4
    TA1EX0=TAIDEX_3;
    //setup TA0 to run in continuous mode
    //set input divider to /8 for a total of /32
    TA1CTL=TASSEL_1|ID_3|MC_2|TACLR;

}

void flashPattern_setColor(LED_color color)
{
    pat_color=color;
    //refresh pattern
    flashPatternAdvance();
}

void flashPattern_setValue(unsigned int val)
{
    //set value
    pat_val=val;
    //refresh pattern
    flashPatternAdvance();
}

void flashPatternVC(int pattern,unsigned int val,LED_color color)
{
    pat_color=color;
    //set value
    pat_val=val;

    flashPatternChange(pattern);
}

void flashPatternAdvance(void)
{
    int i;
    int red_idx,blue_idx,green_idx;
    int lin_idx,strp_idx;

    //advance index if needed
    switch(LED_pattern){
        case LED_PAT_COLORTRAIN:
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
        break;
        case LED_PAT_PAD:
            //calculate new index
            LED_idx+=(idx_dir&0x01)?-1:1;
            //check for reversal
            if(LED_idx>=(0xFF+50) || LED_idx<=0)
            {
                //increment direction
                idx_dir+=1;
                //check for overflow
                if(idx_dir>=6){
                    idx_dir=0;
                }
            }
        break;
        case LED_PAT_HUE:
            //calculate new index
            LED_idx+=1;
            //limit 0-255
            LED_idx&=0xFF;
        break;
        case LED_PAT_SATURATION:
            //calculate new index
            LED_idx+=(idx_dir&0x01)?-1:1;
            //check for reversal
            if(LED_idx>=(0xFF+50) || LED_idx<=0)
            {
                //increment direction
                idx_dir+=1;
                //check for overflow
                if(idx_dir>=6){
                    idx_dir=0;
                }
            }
        break;
        case LED_PAT_USA:
            LED_idx+=1;
            if(LED_idx>=300)
            {
                LED_idx=0;
            }
        break;
        case LED_PAT_BOOST:
            LED_idx+=1;
            if(LED_idx>50){
                LED_idx=0;
            }
        break;
        case LED_PAT_BURST:
            //calculate new index
            LED_idx+=1;
            //limit 0 to LED_LEN
            if(LED_idx>=(LED_LEN*5))
            {
                LED_idx=0;
            }
        break;
    }


    for(i=0;i<NUM_LEDS;i++)
    {
        //calculate linear index for LED
        lin_idx  =LED_lut[i][0];
        //get strip index for LED
        strp_idx =LED_lut[i][1];
        switch(LED_pattern){
            case LED_PAT_ST_COLORS:
                //set brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_NORM;
                //set blue
                LED_stat[0].colors[i].b=(strp_idx==0)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(strp_idx==1)?0xFF:0;
                //set red
                LED_stat[0].colors[i].r=(strp_idx==2)?0xFF:0;
            break;
            case LED_PAT_ST_USA:
                //set brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_NORM;
                //set blue
                LED_stat[0].colors[i].b=(strp_idx==0||strp_idx==1)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(strp_idx==1)?0xFF:0;
                //set red
                LED_stat[0].colors[i].r=(strp_idx==2||strp_idx==1)?0xFF:0;
            break;
            case LED_PAT_USA:
                if(i==0){
                    _NOP();
                }
                if(LED_idx<=75){
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_HIGH;
                    //red color
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<100)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                    //LEDs off
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<=175)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_HIGH;
                    //white color
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0xFF;
                    LED_stat[0].colors[i].b=0xFF;
                }
                else if(LED_idx<200)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                    //LEDs off
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<=275)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_HIGH;
                    //blue color
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0xFF;
                }
                else if(LED_idx<300)
                {
                    //set brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                    //LEDs off
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
            break;
            case LED_PAT_BOOST:
                //set color from pattern color
                LED_stat[0].colors[i]=pat_color;
                if(LED_idx>0)
                {
                    //overide brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_NORM;
                }
                else
                {
                    //overide brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_EXHIGH;
                }
            break;
            case LED_PAT_OFF:
                //set color to zero
                LED_stat[0].colors[i].r=LED_stat[0].colors[i].b=LED_stat[0].colors[i].g;
                //set brightness to zero
                LED_stat[0].colors[i].brt=LED_ST_BITS;
            break;
            case LED_PAT_COLORTRAIN:
                //set brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_DIM;
                //set red
                LED_stat[0].colors[i].r=(lin_idx==red_idx)?0xFF:0;
                //set blue
                LED_stat[0].colors[i].b=(lin_idx==blue_idx)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(lin_idx==green_idx)?0xFF:0;
            break;
            case LED_PAT_PAD:
                //calculate color in RGB
                HsvToLED(&LED_stat[0].colors[i],LED_BRT_LOW,strp_idx*85+(idx_dir>>1)*85,(LED_idx>0xFF)?0xFF:LED_idx,0xFF);
            break;
            case LED_PAT_HUE:
                //is this the first loop
                if(i==0)
                {
                    //calculate color in RGB
                    HsvToLED(&LED_stat[0].colors[0],LED_BRT_NORM,LED_idx,0xFF,0xFF);
                }else
                {
                    //copy from first LED
                    LED_stat[0].colors[i].brt=LED_stat[0].colors[0].brt;
                    LED_stat[0].colors[i].r  =LED_stat[0].colors[0].r;
                    LED_stat[0].colors[i].g  =LED_stat[0].colors[0].g;
                    LED_stat[0].colors[i].b  =LED_stat[0].colors[0].b;
                }
            break;
            case LED_PAT_BURST:
                //set to full brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|LED_BRT_NORM;
                //LEDs are red or whit, red always max
                LED_stat[0].colors[i].r  =0xFF;
                if(lin_idx>=(C_LEFT-LED_idx) && lin_idx<=(C_RIGHT+LED_idx))
                {
                    if(LED_idx>=3 && lin_idx>=(C_LEFT-(LED_idx-3)) && lin_idx<=(C_RIGHT+(LED_idx-3)))
                    {
                        //LED is black, clear red
                        LED_stat[0].colors[i].r  =0x00;
                    }
                    //LED is red
                    LED_stat[0].colors[i].g  =0x00;
                    LED_stat[0].colors[i].b  =0x00;
                }
                else
                {
                    //LED is white
                    LED_stat[0].colors[i].g  =0xFF;
                    LED_stat[0].colors[i].b  =0xFF;
                }
                if(LED_idx>=6)
                {
                    //HsvToLED(&LED_stat[0].colors[i],0,0,0xFF/(25-6)*(LED_idx-5));
                    //LED is white
                    LED_stat[0].colors[i].r  =0xFF;
                    LED_stat[0].colors[i].g  =0xFF;
                    LED_stat[0].colors[i].b  =0xFF;
                    //ramp up brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|((MAX_BRT/(26-6)*(LED_idx-5)));

                }
            break;
            case LED_PAT_SATURATION:
                //calculate color in RGB
                HsvToLED(&LED_stat[0].colors[i],LED_BRT_NORM,strp_idx*85+(idx_dir>>1)*85,(LED_idx>0xFF)?0xFF:LED_idx,0xFF);
            break;
            case LED_PAT_GRAPH:
                if(lin_idx<pat_val)
                {
                    LED_stat[0].colors[i]=pat_color;
                }
                else
                {
                    LED_stat[0].colors[i].brt=pat_color.brt;
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0xFF;
                    LED_stat[0].colors[i].b=0xFF;
                }
        }
    }
    //send new info
    LEDs_send(&LED_stat[0]);
}

void flashPatternNext(void)
{
    int new_pattern=LED_pattern+1;
    if(new_pattern>LED_PAT_MAX)
    {
        //set to next pattern
        new_pattern=LED_PAT_MIN;
    }
    flashPatternChange(new_pattern);
}


int flashPatternGet(void)
{
    return LED_pattern;
}

void flashPatternChange(int pattern)
{
    //check if LED's will be on
    if(pattern!=LED_PAT_OFF)
    {
        //turn on regulator
        reg5V_on();
    }
    //default is ~1 s
    unsigned short flash_per=1020;
    //limit pattern to valid values
    if(pattern<LED_PAT_MIN)
    {
        pattern=LED_PAT_MIN;
    }
    if(pattern>LED_PAT_MAX)
    {
        pattern=LED_PAT_MAX;
    }
    //set new pattern
    LED_pattern=pattern;

    //init indexes
    switch(LED_pattern)
    {
        case LED_PAT_COLORTRAIN:
            idx_dir=1;
            LED_idx=-2;
            //set interrupt interval
            flash_per=102*2;
        break;
        case LED_PAT_USA:
            LED_idx=0;
            //set interrupt interval
            flash_per=20;
        break;
        case LED_PAT_BOOST:
            LED_idx=0;
            //set interrupt interval
            flash_per=600;
        break;
        case LED_PAT_HUE:
            LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_PAD:
            idx_dir=0;
            LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_SATURATION:
            idx_dir=0;
            LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_BURST:
            LED_idx=0;
            //set interrupt interval
            flash_per=102;
        break;
        case LED_PAT_OFF:
            //don't update
            flash_per=0;
        break;
    }

    //write LED's
    flashPatternAdvance();
    //set interrupt period
    if(flash_per==0)
    {
        //no interrupts
        TA1CCTL0=CCIE;
    }
    else
    {
        LED_int=flash_per;
        //setup TA0CCR1 to capture timer value
        TA1CCTL0=CM_3|CCIS_2|SCS|CAP|CCIE;
        //capture current timer value
        TA1CCTL0^=CCIS0;
    }
    //check if LED's are off
    if(LED_pattern==LED_PAT_OFF)
    {
        //turn off regulator
        reg5V_off();
        //TODO: do more low power stuff??
    }
}


void HsvToLED(LED_color *dest,unsigned char brt,unsigned char hue,unsigned char saturation,unsigned char value)
{
    unsigned short region, remainder, p, q, t;

    if(brt>MAX_BRT)
    {
        //set to max
        brt=MAX_BRT;
    }

    //set brightness
    dest->brt=LED_ST_BITS|brt;

    if (saturation == 0)
    {
        dest->r = value;
        dest->g = value;
        dest->b = value;
        return;
    }

    region = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (value * (255 - saturation)) >> 8;
    q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
    t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            dest->r = value;
            dest->g = t;
            dest->b = p;
            break;
        case 1:
            dest->r = q;
            dest->g = value;
            dest->b = p;
            break;
        case 2:
            dest->r = p;
            dest->g = value;
            dest->b = t;
            break;
        case 3:
            dest->r = p;
            dest->g = q;
            dest->b = value;
            break;
        case 4:
            dest->r = t;
            dest->g = p;
            dest->b = value;
            break;
        default:
            dest->r = value;
            dest->g = p;
            dest->b = q;
            break;
    }
}


// ============ TA0 ISR ============
// This is used for button debouncing
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt void flash_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) flash_ISR (void)
#else
#error Compiler not found!
#endif
{
    if(TA1CCTL0&CAP)
    {
        //set next interrupt time
        TA1CCR0+=LED_int;
        //switch to compare mode
        TA1CCTL0=CCIE;
    }
    else
    {
        //set next interrupt time
        TA1CCR0+=LED_int;

        //set next flash pattern
        flashPatternAdvance();
    }
}

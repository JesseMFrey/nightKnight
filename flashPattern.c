/*
 * flashPattern.c
 *
 *  Created on: Jan 3, 2019
 *      Author: jesse
 */

#include "LEDs.h"
#include "flashPattern.h"


#define C_RIGHT ((LED_LEN  )/2)
#define C_LEFT  ((LED_LEN-1)/2)

static int LED_idx=0;
static int LED_pattern=LED_PAT_ST_COLORS;
static int idx_dir=0;

static int limit_idx(int i)
{
    if(i<0)
        return 0;
    if(i>=LED_LEN)
        return LED_LEN-1;
    return i;
}

void flashPatternAdvance(void)
{
    int i;
    int red_idx,blue_idx,green_idx;
    int lin_idx;

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
        case LED_PAT_HUE:
            //calculate new index
            LED_idx+=1;
            //limit 0-255
            LED_idx&=0xFF;
        break;
        case LED_PAT_SATURATION:
            //calculate new index
            LED_idx+=idx_dir;
            //check for reversal
            if(LED_idx>=0xFF)
            {
                //set direction to down
                idx_dir=-1;
            }
            else if(LED_idx<=0)
            {
                //set direction to up
                idx_dir=1;
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
        lin_idx=(i%LED_LEN);
        //reverse odd numbered strips
        if((i/LED_LEN)&0x01)
        {
            lin_idx=LED_LEN-lin_idx-1;
        }
        switch(LED_pattern){
            case LED_PAT_ST_COLORS:
                //set to full brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|MAX_BRT;
                //set blue
                LED_stat[0].colors[i].b=(((i%4)==3)?0xFF:((i%4)==0)?0xFF:0);
                //set green
                LED_stat[0].colors[i].g=(((i%4)==3)?0xFF:((i%4)==1)?0xFF:0);
                //set red
                LED_stat[0].colors[i].r=(((i%4)==3)?0xFF:((i%4)==2)?0xFF:0);
            break;
            case LED_PAT_OFF:
                //set color to zero
                LED_stat[0].colors[i].r=LED_stat[0].colors[i].b=LED_stat[0].colors[i].g;
                //set brightness to zero
                LED_stat[0].colors[i].brt=LED_ST_BITS;
            break;
            case LED_PAT_COLORTRAIN:
                //set to full brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|MAX_BRT;
                //set red
                LED_stat[0].colors[i].r=(lin_idx==red_idx)?0xFF:0;
                //set blue
                LED_stat[0].colors[i].b=(lin_idx==blue_idx)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(lin_idx==green_idx)?0xFF:0;
            break;
            case LED_PAT_HUE:
                //is this the first loop
                if(i==0)
                {
                    //calculate color in RGB
                    HsvToLED(&LED_stat[0].colors[0],LED_idx,0xFF,0xFF);
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
                LED_stat[0].colors[i].brt=LED_ST_BITS|MAX_BRT;
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
                //is this the first loop
                if(i==0)
                {
                    //calculate color in RGB
                    HsvToLED(&LED_stat[0].colors[0],0,LED_idx,0xFF);
                }else
                {
                    //copy from first LED
                    LED_stat[0].colors[i].brt=LED_stat[0].colors[0].brt;
                    LED_stat[0].colors[i].r  =LED_stat[0].colors[0].r;
                    LED_stat[0].colors[i].g  =LED_stat[0].colors[0].g;
                    LED_stat[0].colors[i].b  =LED_stat[0].colors[0].b;
                }
            break;
        }
    }
    //send new info
    LEDs_send(&LED_stat[0]);
}

unsigned short flashPatternNext(void)
{
    int new_pattern=LED_pattern+1;
    if(new_pattern>LED_PAT_MAX)
    {
        //set to next pattern
        new_pattern=LED_PAT_MIN;
    }
    return flashPatternChange(new_pattern);
}

unsigned short flashPatternChange(int pattern)
{
    unsigned short flash_per=0;
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
        case LED_PAT_HUE:
            LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_SATURATION:
            idx_dir=1;
            LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_BURST:
            LED_idx=0;
            //set interrupt interval
            flash_per=102;
        break;
    }

    //write LED's
    flashPatternAdvance();

    //return interval
    return flash_per;
}


void HsvToLED(LED_color *dest,unsigned char hue,unsigned char saturation,unsigned char value)
{
    unsigned short region, remainder, p, q, t;

    //set brightness to maximum
    dest->brt=LED_ST_BITS|MAX_BRT;

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

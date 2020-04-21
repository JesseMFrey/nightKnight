/*
 * flashPattern.c
 *
 *  Created on: Jan 3, 2019
 *      Author: jesse
 */

#include "LEDs.h"
#include "flashPattern.h"
#include "regulator.h"
#include "switches.h"
#include <msp430.h>
#include <math.h>
#include <stdlib.h>
#include "Nosecone.h"

#define C_RIGHT ((LED_LEN  )/2)
#define C_LEFT  ((LED_LEN-1)/2)

static int LED_idx=0;
static int LED_pattern;
static int idx_dir=0;

#define NUM_PARTICLES (LED_STR*7)

//particle stuff for particle pattern
static PARTICLE particles[NUM_PARTICLES];

static unsigned short LED_int=102*2;

static LED_color pat_color={LED_BRT_NORM,0xFF,0xFF,0xFF};
unsigned int pat_val;

LED_color RNBW_colors[6]={//start bits|brightness  , b  , g  , r
                           //{LED_ST_BITS|LED_BRT_NORM,0x03,0x03,0xE4},   //Red
                           {LED_ST_BITS|LED_BRT_NORM,0x03,0x03,0xF9},   //Red
                           //{LED_ST_BITS|LED_BRT_NORM,0x00,0xC0,0xFF},   //Orange
                           {LED_ST_BITS|LED_BRT_NORM,0x00,0x30,0xFF},   //Orange
                           //{LED_ST_BITS|LED_BRT_NORM,0x00,0xeD,0xFF},   //Yellow
                           {LED_ST_BITS|LED_BRT_NORM,0x00,0xB1,0xFF},   //Yellow
                           //{LED_ST_BITS|LED_BRT_NORM,0x26,0x80,0x00},   //Green
                           {LED_ST_BITS|LED_BRT_NORM,0x00,0xFF,0x00},   //Green
                           {LED_ST_BITS|LED_BRT_NORM,0xFF,0x4D,0x00},   //Blue
                           {LED_ST_BITS|LED_BRT_NORM,0x78,0x07,0x75}    //Purple
                          };

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
    char sw_val;

    //flash pattern timer interrupt
    TA1CCTL0=CCIE;

    //read DIP switches
    sw_val=readDIP();

    switch(sw_val){
    case 0:
        //set flash pattern
        flashPatternChange(LED_PAT_SATURATION);
        break;
    case 1:
        //set flash pattern
        flashPatternChange(LED_PAT_USA);
        break;
    case 2:
        //set flash pattern
        flashPatternChange(LED_PAT_ST_USA);
        break;
    case 3:
        //set flash pattern
        flashPatternChange(LED_PAT_COLORTRAIN);
        break;
    case 4:
        //set flash pattern
        flashPatternChange(LED_PAT_HUE);
        break;
    case 5:
        flashPatternChange(LED_PAT_RNBW_FLOW);
        break;
    case 6:
        flashPatternChange(LED_PAT_RNBW_ST);
        break;
    case 7:
        flashPatternChange(LED_PAT_RNBW_FLASH);
        break;
    default:
        //set LED's off
        flashPatternChange(LED_PAT_OFF);
        break;
    }

    //set input divider expansion to /4
    TA1EX0=TAIDEX_3;
    //setup TA1 to run in continuous mode
    //set input divider to /8 for a total of /32
    TA1CTL=TASSEL_1|ID_3|MC_2|TACLR;

}

void flashPattern_setColor(LED_color color)
{
    pat_color=color;
    //clear status bits in color
    pat_color.brt&=~LED_ST_BITS;
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

static void new_particle(PARTICLE *n)
{
    //create new particle
    n->v=0.3*(rand()/(float)RAND_MAX)+0.1;
    n->x=LED_LEN+(LED_LEN/2)*(rand()/(float)RAND_MAX);
}

static inline char brt_offset(unsigned char brt,int offset)
{
    offset+=brt;
    //check lower limit
    if(offset<1)
    {
        offset=1;
    }
    if(offset>MAX_BRT)
    {
        offset=MAX_BRT;
    }
    return offset;
}

void flashPatternAdvance(void)
{
    int i,j;
    int tmp1,tmp2;
    int red_idx,blue_idx,green_idx;
    int lin_idx,strp_idx;
    static int particle_pos[NUM_PARTICLES];

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
            if(LED_idx>100){
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
        case LED_PAT_RNBW_FLASH:
            //calculate new index
            LED_idx+=1;
            //limit 0 to 6
            if(LED_idx>=6)
            {
                LED_idx=0;
            }
        break;
        case LED_PAT_RNBW_FLOW:
            //calculate new index
            LED_idx+=1;
            //limit
            if(LED_idx>=(LED_LEN-FIN_LED-2))
            {
                LED_idx=0;
            }
        break;
        case LED_PAT_USA_FLOW:
            //calculate new index
            LED_idx+=1;
            //limit
            if(LED_idx>=6)
            {
                LED_idx=0;
            }
        break;
        case LED_PAT_PARTICLE:
            //shift all particles by their velocity
            for(j=0;j<NUM_PARTICLES;j++)
            {
                particles[j].x-=particles[j].v;
                particle_pos[j]=round(particles[j].x);
                //check if we have gone off the end
                if(particle_pos[j]<0)
                {
                    //create new particle
                    new_particle(&particles[j]);
                }
                //check if particle is about to start
                else if(particle_pos[j]==LED_LEN)
                {
                    //blip the nosecone
                    nosecone_mode(NC_MODE_ONE_SHOT,700,0,5,NC_NA);
                }
            }
        break;
        case LED_PAT_EYES_H:
            LED_idx-=1;
            if(LED_idx<-20)
            {
                //reset
                LED_idx=MAX_BRT;
                //use dir for eye location
                idx_dir=(rand()%(BOOST_LED+UPR_LED-3)) + FIN_LED;
            }
        break;
        case LED_PAT_WAVE_BIG_U:
        case LED_PAT_WAVE_SM_U:
        case LED_PAT_WAVE_SAT_U:
            LED_idx-=1;
            if(LED_idx<0)
            {
                LED_idx=2*pat_val;
            }
            if(LED_PAT_WAVE_SAT_U==LED_pattern)
            {
                idx_dir=(2*0xFF)/pat_val;
            }
        break;
        case LED_PAT_WAVE_HUE_U:
            LED_idx-=1;
            if(LED_idx<0)
            {
                LED_idx=pat_val;
            }
            idx_dir=(0xFF)/pat_val;
        break;
        case LED_PAT_WAVE_BIG_D:
        case LED_PAT_WAVE_SM_D:
        case LED_PAT_WAVE_SAT_D:
            LED_idx+=1;
            if(LED_idx>=2*pat_val)
            {
                LED_idx=0;
            }
            if(LED_PAT_WAVE_SAT_D==LED_pattern)
            {
                idx_dir=(2*0xFF)/pat_val;
            }
        case LED_PAT_WAVE_HUE_D:
            LED_idx+=1;
            if(LED_idx>=pat_val)
            {
                LED_idx=0;
            }
            idx_dir=(0xFF)/pat_val;
        break;
        case LED_PAT_PANIC:
            LED_idx+=1;
            if(LED_idx>=6)
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
                LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                //set blue
                LED_stat[0].colors[i].b=(strp_idx==0)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(strp_idx==1)?0xFF:0;
                //set red
                LED_stat[0].colors[i].r=(strp_idx==2)?0xFF:0;
            break;
            case LED_PAT_ST_USA:
                //set brightness
                LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                //set blue
                LED_stat[0].colors[i].b=(strp_idx==0||strp_idx==1)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(strp_idx==1)?0xFF:0;
                //set red
                LED_stat[0].colors[i].r=(strp_idx==2||strp_idx==1)?0xFF:0;
            break;
            case LED_PAT_USA:
                if(LED_idx<=75){
                    //set brightness from color
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                    //red color
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<100)
                {
                    //set brightness to zero
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                    //LEDs off
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<=175)
                {
                    //set brightness from color
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                    //white color
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0xFF;
                    LED_stat[0].colors[i].b=0xFF;
                }
                else if(LED_idx<200)
                {
                    //set brightness to zero
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                    //LEDs off
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0;
                }
                else if(LED_idx<=275)
                {
                    //set brightness from color
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                    //blue color
                    LED_stat[0].colors[i].r=0;
                    LED_stat[0].colors[i].g=0;
                    LED_stat[0].colors[i].b=0xFF;
                }
                else if(LED_idx<300)
                {
                    //set brightness to zero
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
                if(LED_idx==0)
                {
                    //make brighter
                    LED_stat[0].colors[i].brt=brt_offset(pat_color.brt,14);
                }
                //add brightness bits
                LED_stat[0].colors[i].brt|=LED_ST_BITS;
            break;
            case LED_PAT_OFF:
                //set color to zero
                LED_stat[0].colors[i].r=LED_stat[0].colors[i].b=LED_stat[0].colors[i].g;
                //set brightness to zero
                LED_stat[0].colors[i].brt=LED_ST_BITS;
            break;
            case LED_PAT_COLORTRAIN:
                //set brightness from color
                LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                //set red
                LED_stat[0].colors[i].r=(lin_idx==red_idx)?0xFF:0;
                //set blue
                LED_stat[0].colors[i].b=(lin_idx==blue_idx)?0xFF:0;
                //set green
                LED_stat[0].colors[i].g=(lin_idx==green_idx)?0xFF:0;
            break;
            case LED_PAT_PAD:
                //calculate color in RGB
                HsvToLED(&LED_stat[0].colors[i],brt_offset(pat_color.brt,-9),strp_idx*85+(idx_dir>>1)*85,(LED_idx>0xFF)?0xFF:LED_idx,0xFF);
            break;
            case LED_PAT_HUE:
                //is this the first loop
                if(i==0)
                {
                    //calculate color in RGB
                    HsvToLED(&LED_stat[0].colors[0],pat_color.brt,LED_idx,0xFF,0xFF);
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
                LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
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
                HsvToLED(&LED_stat[0].colors[i],pat_color.brt,strp_idx*85+(idx_dir>>1)*85,(LED_idx>0xFF)?0xFF:LED_idx,0xFF);
            break;
            case LED_PAT_GRAPH:
                if(lin_idx<pat_val)
                {
                    LED_stat[0].colors[i]=pat_color;
                }
                else
                {
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                    LED_stat[0].colors[i].r=0xFF;
                    LED_stat[0].colors[i].g=0xFF;
                    LED_stat[0].colors[i].b=0xFF;
                }
            break;
            case LED_PAT_RNBW_ST:
                if(lin_idx<FIN_LED)
                {
                    //fins bright white
                    LED_stat[0].colors[i].r  =0xFF;
                    LED_stat[0].colors[i].g  =0xFF;
                    LED_stat[0].colors[i].b  =0xFF;
                    //high brightness for fins
                    LED_stat[0].colors[i].brt=LED_ST_BITS|brt_offset(pat_color.brt,9);

                }
                else
                {
                    //set color from array
                    LED_stat[0].colors[i]=RNBW_colors[(((lin_idx-11)/6)%6)];
                    //set color from pattern color
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                }
            break;
            case LED_PAT_RNBW_FLASH:
                LED_stat[0].colors[i]=RNBW_colors[LED_idx];
            break;
            case LED_PAT_RNBW_FLOW:
                if(lin_idx<FIN_LED)
                {
                    //fins bright white
                    LED_stat[0].colors[i].r  =255;
                    LED_stat[0].colors[i].g  =200;
                    LED_stat[0].colors[i].b  =150;
                    //high brightness for fins
                    LED_stat[0].colors[i].brt=LED_ST_BITS|brt_offset(pat_color.brt,9);

                }
                else
                {
                    //set color from array
                    LED_stat[0].colors[i]=RNBW_colors[( ((lin_idx+LED_idx)/6)%6 )];
                    //set color from pattern color
                    LED_stat[0].colors[i].brt=LED_ST_BITS|pat_color.brt;
                }
            break;
            case LED_PAT_USA_FLOW:
                if(lin_idx<FIN_LED)
                {
                    //fins bright Blue
                    LED_stat[0].colors[i].r  =0;
                    LED_stat[0].colors[i].g  =0;
                    LED_stat[0].colors[i].b  =0xFF;
                    //high brightness for fins
                    LED_stat[0].colors[i].brt=LED_ST_BITS|brt_offset(pat_color.brt,9);

                }
                else if( ((lin_idx+LED_idx)/3)%2 )
                {
                    //set color to red
                    LED_stat[0].colors[i]=LED_COLOR_RED;
                }
                else
                {
                    //set color to white
                    //LED_stat[0].colors[i]=LED_COLOR_WHITE;

                    //fins bright white
                    LED_stat[0].colors[i].r  =255;
                    LED_stat[0].colors[i].g  =200;
                    LED_stat[0].colors[i].b  =150;
                    //high brightness for fins
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_ST_BITS|pat_color.brt;
                }
            break;
            case LED_PAT_PARTICLE:

                //set to off
                LED_stat[0].colors[i].r=LED_stat[0].colors[i].g=LED_stat[0].colors[i].b=0;
                //set brightness to zero
                LED_stat[0].colors[i].brt=LED_ST_BITS|0;

                for(j=strp_idx;j<NUM_PARTICLES;j+=LED_STR)
                {
                    tmp1=lin_idx-particle_pos[j];
                    if(tmp1>=0 && tmp1<=5)
                    {
                        //set color
                        LED_stat[0].colors[i].r  =255;
                        LED_stat[0].colors[i].g  =150;
                        LED_stat[0].colors[i].b  =10;
                        if(tmp1==0)
                        {
                            //set brightness
                            LED_stat[0].colors[i].brt=LED_ST_BITS|MAX_BRT;
                        }
                        else
                        {
                            //get brightness, mask out start bits
                            int tbrt=LED_stat[0].colors[i].brt&(~LED_ST_BITS);
                            //add brightness from particle
                            tbrt+=(6-tmp1);
                            //saturate brightness
                            if(tbrt>MAX_BRT)
                            {
                                tbrt=MAX_BRT;
                            }
                            LED_stat[0].colors[i].brt=LED_ST_BITS|(tbrt);
                        }
                    }
                }
            break;
            case LED_PAT_EYES_H:
                if((lin_idx!=idx_dir && lin_idx!=idx_dir+2) || LED_idx<=0)
                {
                    //turn of LED
                    LED_stat[0].colors[i].r  =0;
                    LED_stat[0].colors[i].g  =0;
                    LED_stat[0].colors[i].b  =0;
                    LED_stat[0].colors[i].brt=LED_ST_BITS;
                }
                else
                {
                    //fade out LED
                    LED_stat[0].colors[i].r  =0xFF;
                    LED_stat[0].colors[i].g  =0;
                    LED_stat[0].colors[i].b  =0;
                    LED_stat[0].colors[i].brt=LED_ST_BITS|LED_idx;
                }
            break;
            case LED_PAT_WAVE_BIG_U:
            case LED_PAT_WAVE_BIG_D:
            case LED_PAT_WAVE_SM_U:
            case LED_PAT_WAVE_SM_D:
                //set color
                LED_stat[0].colors[i].r  = pat_color.r;
                LED_stat[0].colors[i].g  = pat_color.g;
                LED_stat[0].colors[i].b  = pat_color.b;

                //calculate index in pattern
                tmp1=lin_idx+LED_idx;
                tmp1=tmp1%pat_val;

                //calculate midpoint
                tmp2=(pat_val+1)/2;
                //check if we are past the midpoint
                if(tmp1>tmp2)
                {
                    //shift so we get a V
                    tmp1=(pat_val+1)-tmp1;
                }

                //for the small patterns subtract more
                if(LED_pattern==LED_PAT_WAVE_SM_U || LED_pattern==LED_PAT_WAVE_SM_D)
                {
                    //multiply value by four
                    tmp1*=4;
                }

                tmp1=(pat_color.brt)-tmp1;
                if(tmp1<0)
                {
                    tmp1=0;
                }
                LED_stat[0].colors[i].brt=LED_ST_BITS|tmp1;
            break;
            case LED_PAT_WAVE_HUE_D:
            case LED_PAT_WAVE_HUE_U:
                //calculate index in pattern
                tmp1=lin_idx+LED_idx;
                tmp1=tmp1%pat_val;

                //scale value to get to full scale
                tmp1*=idx_dir;

                //calculate color in RGB. Use the green and blue values from the pattern color as saturation and value
                HsvToLED(&LED_stat[0].colors[i],pat_color.brt,tmp1,pat_color.g,pat_color.b);
            break;
            case LED_PAT_WAVE_SAT_D:
            case LED_PAT_WAVE_SAT_U:

                //calculate index in pattern
                tmp1=lin_idx+LED_idx;
                tmp1=tmp1%pat_val;

                //calculate midpoint
                tmp2=(pat_val+1)/2;
                //check if we are past the midpoint
                if(tmp1>tmp2)
                {
                    //shift so we get a V
                    tmp1=(pat_val+1)-tmp1;
                }
                //scale value to get to full scale
                tmp1*=idx_dir;

                //calculate color in RGB. Use the red and blue values from the pattern color as hue and value
                HsvToLED(&LED_stat[0].colors[i],pat_color.brt,pat_color.r,tmp1,pat_color.b);
            break;
            case LED_PAT_PANIC:
                if(LED_idx==1 ||LED_idx==3)
                {
                    //set color to red
                    LED_stat[0].colors[i].r  =0xFF;
                    LED_stat[0].colors[i].g  =0;
                    LED_stat[0].colors[i].b  =0;
                    //set fixed, medium brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|8;
                }
                else
                {
                    //set LED's off
                    LED_stat[0].colors[i].r  =0;
                    LED_stat[0].colors[i].g  =0;
                    LED_stat[0].colors[i].b  =0;
                    //set fixed, medium brightness
                    LED_stat[0].colors[i].brt=LED_ST_BITS|0;
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
    int string;
    int i;
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
            LED_idx=-1;
            //set interrupt interval
            flash_per=2000;
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
        case LED_PAT_RNBW_FLASH:
            LED_idx=0;
            //set interrupt interval
            flash_per=2048;
        break;
        case LED_PAT_RNBW_FLOW:
            LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_USA_FLOW:
            LED_idx=0;
            //set interrupt interval
            flash_per=200;
        break;
        case LED_PAT_PARTICLE:
            LED_idx=LED_LEN;
            //set interrupt interval
            flash_per=10;
            string=0;
            for(i=0;i<NUM_PARTICLES;i++)
            {
                new_particle(&particles[i]);
                string+=1;
                if(string>=LED_STR)
                {
                    string=0;
                }

            }
        break;
        case LED_PAT_EYES_H:
            LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_WAVE_BIG_U:
        case LED_PAT_WAVE_BIG_D:
        case LED_PAT_WAVE_SM_U:
        case LED_PAT_WAVE_SM_D:
        case LED_PAT_WAVE_HUE_D:
        case LED_PAT_WAVE_HUE_U:
        case LED_PAT_WAVE_SAT_D:
        case LED_PAT_WAVE_SAT_U:
            LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_PANIC:
            LED_idx=0;
            //set interrupt interval
            flash_per=200;
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
        //setup TA1CCR1 to capture timer value
        TA1CCTL0=CM_3|CCIS_2|SCS|CAP|CCIE;
        //capture current timer value
        TA1CCTL0^=CCIS0;
    }
    //check if LED's are off
    if(LED_pattern==LED_PAT_OFF)
    {
        //turn off regulator
        reg5V_off();
        //turn off nosecone
        nosecone_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
        //turn chute off
        chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
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


// ============ TA1.0 ISR ============

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

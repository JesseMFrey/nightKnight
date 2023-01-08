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
#include "events.h"
#include "settings.h"

#define C_RIGHT ((LED_LEN  )/2)
#define C_LEFT  ((LED_LEN-1)/2)

#define GRAPH_WHITE_BRT  (180)

#define USA_WHITE_BRT   (90)

static int LED_pattern;

#define NUM_PARTICLES (LED_STR*30)

static int buffer_idx=0;

int only_fins = 0;

static union{
    struct{
        int idx_dir;
        int LED_idx;
        //indicies for colortrain
        int red_idx,blue_idx,green_idx;
    }basic;
    struct{
        //particle stuff for particle pattern
        PARTICLE particles[NUM_PARTICLES];
        int particle_pos[NUM_PARTICLES];
        //number of particles
        int num;
    }ptc;
} pat_d;

static int pat_i;

static unsigned short LED_int=102*2;

//interval for simulations
unsigned int sim_int=1000;


const COLOR_LIST RNBW_colors={ .num_colors=6,.alt_color={
                       {.alt_frac=0  ,.color={.brt=LED_BRT_NORM,.r=0xF9,.g=0x03,.b=0x03}},   //Red
                       {.alt_frac=42 ,.color={.brt=LED_BRT_NORM,.r=0xE7,.g=0x18,.b=0x00}},   //Orange
                       {.alt_frac=84,.color={.brt=LED_BRT_NORM,.r=0xA6,.g=0x58,.b=0x00}},   //Yellow
                       {.alt_frac=126,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0xFF,.b=0x00}},   //Green
                       {.alt_frac=168,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0x26,.b=0xD9}},   //Blue
                       {.alt_frac=210,.color={.brt=LED_BRT_NORM,.r=0x78,.g=0x0A,.b=0x7B}}    //Purple
                    }
                  };

const COLOR_LIST USA_colors={ .num_colors=3,.alt_color={
                       {.alt_frac=0  ,.color={.brt=LED_BRT_NORM,.r=0xFF,.g=0x00,.b=0x00}},
                       {.alt_frac=85 ,.color={.brt=LED_BRT_NORM,.r=USA_WHITE_BRT,.g=USA_WHITE_BRT,.b=USA_WHITE_BRT}},
                       {.alt_frac=170,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0x00,.b=0xFF}}
                    }
                  };

const COLOR_LIST USA_spk_colors={ .num_colors=3,.alt_color={
                       {.alt_frac=0   ,.color={.brt=LED_BRT_NORM,.r=USA_WHITE_BRT,.g=USA_WHITE_BRT,.b=USA_WHITE_BRT}},
                       {.alt_frac=85  ,.color={.brt=LED_BRT_NORM,.r=0xFF,.g=0x00,.b=0x00}},
                       {.alt_frac=170,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0x00,.b=0xFF}}
                    }
                  };

const COLOR_LIST RGB_colors={ .num_colors=3,.alt_color={
                       {.alt_frac=0  ,.color={.brt=LED_BRT_NORM,.r=0xFF,.g=0x00,.b=0x00}},
                       {.alt_frac=85 ,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0xFF,.b=0x00}},
                       {.alt_frac=170,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0x00,.b=0xFF}}
                    }
                  };

const COLOR_LIST USA_RW_colors={
                    .num_colors=2,.alt_color={
                       {.alt_frac=0  ,.color={.brt=LED_BRT_NORM,.r=0xFF,.g=0x00,.b=0x00}},
                       {.alt_frac=128 ,.color={.brt=LED_BRT_NORM,.r=USA_WHITE_BRT,.g=USA_WHITE_BRT,.b=USA_WHITE_BRT}},
                    }
                  };

const COLOR_LIST Xmas_colors={
                    .num_colors=3,.alt_color={
                          {.alt_frac=0  ,.color={.brt=LED_BRT_NORM,.r=0xFF,.g=0x00,.b=0x00}},
                          {.alt_frac=85 ,.color={.brt=LED_BRT_NORM,.r=90,.g=90,.b=80}},
                          {.alt_frac=170,.color={.brt=LED_BRT_NORM,.r=0x00,.g=0xFF,.b=0x00}},
                    }
                  };

static int limit_idx(int i)
{
    if(i<0)
        return 0;
    if(i>=LED_LEN)
        return LED_LEN-1;
    return i;
}

int isPanic(int type)
{
    switch(type)
    {
        case LED_PAT_POWER_PANIC:
		case LED_PAT_MODE_PANIC:
		case LED_PAT_RESET_PANIC:
		case LED_PAT_PATTERN_PANIC:
		    return 1;
		default:
		    return 0;
    }
}

void panic(int type)
{
    //check if we are in panic
    if(!isPanic(LED_pattern))
    {
        //check if we actually have a panic pattern
        if(isPanic(type))
        {
            //PANIC!!
            flashPatternChange(type);
        }
        else
        {
            //use pattern panic for incorrect panic
            flashPatternChange(LED_PAT_PATTERN_PANIC);
        }
    }
}

void init_FlashPattern(void)
{
    char sw_val;

    //flash pattern timer interrupt
    TA1CCTL0=CCIE;

    //read DIP switches
    sw_val=readDIP();
    //check if we should be off on startup
    if((sw_val&BIT0))
    {
        //set pattern from settings
        flashPatternChange(settings.pattern);
    }
    else
    {
        //set LED's off
        flashPatternChange(LED_PAT_OFF);
    }
    //setup TA1CCR1 for sim timer
    TA1CCR1=sim_int;
    TA1CCTL1=CCIE;
    //set input divider expansion to /4
    TA1EX0=TAIDEX_3;
    //setup TA1 to run in continuous mode
    //set input divider to /8 for a total of /32
    TA1CTL=TASSEL_1|ID_3|MC_2|TACLR;

}

void flashPattern_setList(const COLOR_LIST *list)
{
    settings.list=list;
    //refresh pattern
    flashPatternAdvance();
}

void flashPattern_setColor(LED_color color)
{
    settings.color=color;
    //clear status bits in color
    settings.color.brt&=~LED_ST_BITS;
    //refresh pattern
    flashPatternAdvance();
}

void flashPattern_setValue(unsigned int val)
{
    //set value
    settings.value=val;
    //refresh pattern
    flashPatternAdvance();
}

void flashPatternVC(int pattern,unsigned int val,LED_color color)
{
    settings.color=color;
    //set value
    settings.value=val;

    flashPatternChange(pattern);
}

void flashPatternVCL(int pattern,unsigned int val,LED_color color,const COLOR_LIST *list)
{
    settings.color=color;
    //set value
    settings.value=val;
    //set list
    settings.list=list;
    flashPatternChange(pattern);
}

static void new_particle(PARTICLE *n,int type)
{
    int tmp;
    //create new particle
    n->v=0.3*(rand()/(float)RAND_MAX)+0.1;
    n->x=only_fins?FIN_LED:LED_LEN+(LED_LEN/2)*(rand()/(float)RAND_MAX);

    switch(type)
    {

    case LED_PAT_PARTICLE:
    case LED_PAT_UNIFORM_PARTICLE:
        //set color from pattern
        n->color=settings.color;
    break;
    case LED_PAT_COLOR_PARTICLE:
    case LED_PAT_COLOR_UNIFORM_PARTICLE:
        tmp=rand();
        //set color with random hue and saturation
        HsvToLED(&n->color,settings.color.brt,tmp,(tmp>>8)|0x80,0xFF);
    break;
    case LED_PAT_LIST_UNIFORM_PARTICLE:
    case LED_PAT_LIST_PARTICLE:
        n->color=settings.list->alt_color[rand()%settings.list->num_colors].color;
        //set brightness from color
        n->color.brt=LED_ST_BITS|settings.color.brt;
    break;
    }
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
    //check if pattern has finished
    if(pat_i>NUM_LEDS)
    {
        //reset index
        pat_i=0;
    }
    //set event for flash pattern
    e_flags|=FP_ADVANCE;
}

int flashPatternStep(void)
{
    int j;
    int tmp1,tmp2;
    int lin_idx,strp_idx;

    if(pat_i==0){

        //increment buffer index
        buffer_idx+=1;
        //limit to 1 bit
        buffer_idx&=0x01;
        //advance index if needed
        switch(LED_pattern){
            case LED_PAT_COLORTRAIN:
                //calculate new index
                pat_d.basic.LED_idx+=pat_d.basic.idx_dir;
                //check for overflow
                if(pat_d.basic.LED_idx>=(LED_LEN))
                {
                    //reset index
                    pat_d.basic.LED_idx=(LED_LEN-1);
                    //flip direction
                    pat_d.basic.idx_dir=-1;
                }
                //check for
                if(pat_d.basic.LED_idx<=-3)
                {
                    //reset index
                    pat_d.basic.LED_idx=-2;
                    //flip direction
                    pat_d.basic.idx_dir=1;
                }
                pat_d.basic.green_idx=limit_idx(pat_d.basic.LED_idx);
                pat_d.basic.blue_idx =limit_idx(pat_d.basic.LED_idx+1);
                pat_d.basic.red_idx  =limit_idx(pat_d.basic.LED_idx+2);
            break;
            case LED_PAT_HUE:
                //calculate new index
                pat_d.basic.LED_idx+=1;
                //limit 0-255
                pat_d.basic.LED_idx&=0xFF;
            break;
            case LED_PAT_SATURATION:
                //calculate new index
                pat_d.basic.LED_idx+=(pat_d.basic.idx_dir&0x01)?-1:1;
                //check for reversal
                if(pat_d.basic.LED_idx>=(0xFF+50) || pat_d.basic.LED_idx<=0)
                {
                    //increment direction
                    pat_d.basic.idx_dir+=1;
                    //check for overflow
                    if(pat_d.basic.idx_dir>=6){
                        pat_d.basic.idx_dir=0;
                    }
                }
            break;
            case LED_PAT_FLASH_GAP:
                if(pat_d.basic.LED_idx&1)
                {
                    //odd, off time
                    pat_d.basic.idx_dir+=1;
                    if(pat_d.basic.idx_dir>=25)
                    {
                        pat_d.basic.idx_dir=0;
                        pat_d.basic.LED_idx+=1;
                    }
                }
                else
                {
                    //even, on time
                    pat_d.basic.idx_dir+=1;
                    if(pat_d.basic.idx_dir>=75)
                    {
                        pat_d.basic.idx_dir=0;
                        pat_d.basic.LED_idx+=1;
                    }
                }
                if(pat_d.basic.LED_idx>=(settings.list->num_colors*2))
                {
                    //wrap around
                    pat_d.basic.LED_idx=0;
                }
            break;
            case LED_PAT_BOOST:
                pat_d.basic.LED_idx+=1;
                if(pat_d.basic.LED_idx>100){
                    pat_d.basic.LED_idx=0;
                }
                nosecone_pattern_mode(NC_MODE_STATIC,pat_d.basic.LED_idx==0?0xFFF:0x1F,NC_NA,NC_NA,NC_NA);
            break;
            case LED_PAT_BURST:
                //calculate new index
                pat_d.basic.LED_idx+=1;
                //limit 0 to LED_LEN
                if(pat_d.basic.LED_idx>=(LED_LEN*5))
                {
                    pat_d.basic.LED_idx=0;
                }
            break;
            case LED_PAT_FLASH_NOGAP:
                //calculate new index
                pat_d.basic.LED_idx+=1;
                //limit 0 to 6
                if(pat_d.basic.LED_idx>=settings.list->num_colors)
                {
                    pat_d.basic.LED_idx=0;
                }
            break;
            case LED_PAT_FLOW_LIST:
                //calculate new index
                pat_d.basic.LED_idx+=1;
                //limit
                if(pat_d.basic.LED_idx>=settings.value*settings.list->num_colors)
                {
                    pat_d.basic.LED_idx=0;
                }
            break;
            case LED_PAT_PARTICLE:
            case LED_PAT_COLOR_PARTICLE:
            case LED_PAT_UNIFORM_PARTICLE:
            case LED_PAT_COLOR_UNIFORM_PARTICLE:
            case LED_PAT_LIST_PARTICLE:
            case LED_PAT_LIST_UNIFORM_PARTICLE:
                //calculate number of particles from pattern value
                pat_d.ptc.num=settings.value;
                //check if we have independent strings
                if(LED_pattern==LED_PAT_COLOR_PARTICLE || LED_pattern==LED_PAT_PARTICLE || LED_pattern==LED_PAT_LIST_PARTICLE)
                {
                    if(pat_d.ptc.num>(NUM_PARTICLES/LED_STR))
                    {
                        //limit to number of particles
                        pat_d.ptc.num=(NUM_PARTICLES/LED_STR);
                    }
                }
                else
                {
                    if(pat_d.ptc.num>NUM_PARTICLES)
                    {
                        //limit to number of particles
                        pat_d.ptc.num=NUM_PARTICLES;
                    }
                }
                //shift all particles by their velocity
                for(j=0;j<pat_d.ptc.num;j++)
                {
                    pat_d.ptc.particles[j].x-=pat_d.ptc.particles[j].v;
                    pat_d.ptc.particle_pos[j]=round(pat_d.ptc.particles[j].x);
                    //check if we have gone off the end
                    if(pat_d.ptc.particle_pos[j]<-4)
                    {
                        //create new particle
                        new_particle(&pat_d.ptc.particles[j],LED_pattern);
                    }
                    //check if particle is about to start
                    else if(pat_d.ptc.particle_pos[j]==(LED_LEN-1))
                    {
                        //blip the nosecone
                        nosecone_pattern_mode(NC_MODE_ONE_SHOT,700,0,5,NC_NA);
                    }
                }
            break;
            case LED_PAT_EYES_H:
                pat_d.basic.LED_idx-=1;
                if(pat_d.basic.LED_idx<-20)
                {
                    //reset
                    pat_d.basic.LED_idx=MAX_BRT;
                    //use dir for eye location
                    pat_d.basic.idx_dir=(rand()%(BOOST_LED+UPR_LED-3)) + FIN_LED;
                }
            break;
            case LED_PAT_WAVE_BIG_U:
            case LED_PAT_WAVE_SM_U:
            case LED_PAT_WAVE_SAT_U:
                pat_d.basic.LED_idx-=1;
                if(pat_d.basic.LED_idx<0)
                {
                    pat_d.basic.LED_idx=2*settings.value;
                }
                if(LED_PAT_WAVE_SAT_U==LED_pattern)
                {
                    pat_d.basic.idx_dir=(2*0xFF)/settings.value;
                }
            break;
            case LED_PAT_WAVE_HUE_U:
                pat_d.basic.LED_idx-=1;
                if(pat_d.basic.LED_idx<0)
                {
                    pat_d.basic.LED_idx=settings.value;
                }
                pat_d.basic.idx_dir=(0xFF)/settings.value;
            break;
            case LED_PAT_WAVE_BIG_D:
            case LED_PAT_WAVE_SM_D:
            case LED_PAT_WAVE_SAT_D:
                pat_d.basic.LED_idx+=1;
                if(pat_d.basic.LED_idx>=2*settings.value)
                {
                    pat_d.basic.LED_idx=0;
                }
                if(LED_PAT_WAVE_SAT_D==LED_pattern)
                {
                    pat_d.basic.idx_dir=(2*0xFF)/settings.value;
                }
            case LED_PAT_WAVE_HUE_D:
                pat_d.basic.LED_idx+=1;
                if(pat_d.basic.LED_idx>=settings.value)
                {
                    pat_d.basic.LED_idx=0;
                }
                pat_d.basic.idx_dir=(0xFF)/settings.value;
            break;
            case LED_PAT_POWER_PANIC:
            case LED_PAT_MODE_PANIC:
            case LED_PAT_RESET_PANIC:
            case LED_PAT_PATTERN_PANIC:
                pat_d.basic.LED_idx+=1;
                if(pat_d.basic.LED_idx>=6)
                {
                    pat_d.basic.LED_idx=0;
                }
            break;
        }
    }
    else if(pat_i<=NUM_LEDS)
    {
        //calculate linear index for LED
        lin_idx  =LED_lut[pat_i-1][0];
        //check if we should skip
        if(only_fins && lin_idx >= FIN_LED)
        {
            //set color to zero
            LED_stat[buffer_idx].colors[pat_i-1].r=LED_stat[buffer_idx].colors[pat_i-1].b=LED_stat[buffer_idx].colors[pat_i-1].g;
            //set brightness to zero
            LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS;
        }
        else
        {
            //get strip index for LED
            strp_idx =LED_lut[pat_i-1][1];
            switch(LED_pattern){
                case LED_PAT_STR_ST:
                    //set color
                    LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[(strp_idx%(settings.list->num_colors))].color;
                    //set brightness
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                break;
                case LED_PAT_FLASH_GAP:
                    if(pat_d.basic.LED_idx&1)
                    {
                        //set brightness to zero
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS;
                        //LEDs off
                        LED_stat[buffer_idx].colors[pat_i-1].r=0;
                        LED_stat[buffer_idx].colors[pat_i-1].g=0;
                        LED_stat[buffer_idx].colors[pat_i-1].b=0;
                    }
                    else
                    {
                        //even,on
                        //set brightness from color
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                        //color index
                        tmp1=pat_d.basic.LED_idx/2;
                        //set color from list
                        LED_stat[buffer_idx].colors[pat_i-1].r=settings.list->alt_color[tmp1].color.r;
                        LED_stat[buffer_idx].colors[pat_i-1].g=settings.list->alt_color[tmp1].color.g;
                        LED_stat[buffer_idx].colors[pat_i-1].b=settings.list->alt_color[tmp1].color.b;
                    }
                break;
                case LED_PAT_BOOST:
                    //set color from pattern color
                    LED_stat[buffer_idx].colors[pat_i-1]=settings.color;
                    if(pat_d.basic.LED_idx==0)
                    {
                        //make brighter
                        LED_stat[buffer_idx].colors[pat_i-1].brt=brt_offset(settings.color.brt,14);
                    }
                    //add brightness bits
                    LED_stat[buffer_idx].colors[pat_i-1].brt|=LED_ST_BITS;
                break;
                case LED_PAT_OFF:
                    //set color to zero
                    LED_stat[buffer_idx].colors[pat_i-1].r=LED_stat[buffer_idx].colors[pat_i-1].b=LED_stat[buffer_idx].colors[pat_i-1].g;
                    //set brightness to zero
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS;
                break;
                case LED_PAT_COLORTRAIN:
                    //set brightness from color
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                    //set red
                    LED_stat[buffer_idx].colors[pat_i-1].r=(lin_idx==pat_d.basic.red_idx)?0xFF:0;
                    //set blue
                    LED_stat[buffer_idx].colors[pat_i-1].b=(lin_idx==pat_d.basic.blue_idx)?0xFF:0;
                    //set green
                    LED_stat[buffer_idx].colors[pat_i-1].g=(lin_idx==pat_d.basic.green_idx)?0xFF:0;
                break;
                case LED_PAT_HUE:
                    //calculate color in RGB
                    HsvToLED(&LED_stat[buffer_idx].colors[pat_i-1],settings.color.brt,pat_d.basic.LED_idx,0xFF,0xFF);
                break;
                case LED_PAT_BURST:
                    //set to full brightness
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                    //LEDs are red or whit, red always max
                    LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                    if(lin_idx>=(C_LEFT-pat_d.basic.LED_idx) && lin_idx<=(C_RIGHT+pat_d.basic.LED_idx))
                    {
                        if(pat_d.basic.LED_idx>=3 && lin_idx>=(C_LEFT-(pat_d.basic.LED_idx-3)) && lin_idx<=(C_RIGHT+(pat_d.basic.LED_idx-3)))
                        {
                            //LED is black, clear red
                            LED_stat[buffer_idx].colors[pat_i-1].r  =0x00;
                        }
                        //LED is red
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0x00;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0x00;
                    }
                    else
                    {
                        //LED is white
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0xFF;
                    }
                    if(pat_d.basic.LED_idx>=6)
                    {
                        //HsvToLED(&LED_stat[buffer_idx].colors[pat_i-1],0,0,0xFF/(25-6)*(pat_d.basic.LED_idx-5));
                        //LED is white
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0xFF;
                        //ramp up brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|((MAX_BRT/(26-6)*(pat_d.basic.LED_idx-5)));

                    }
                break;
                case LED_PAT_SATURATION:
                    //calculate color in RGB
                    HsvToLED(&LED_stat[buffer_idx].colors[pat_i-1],settings.color.brt,strp_idx*85+(pat_d.basic.idx_dir>>1)*85,(pat_d.basic.LED_idx>0xFF)?0xFF:pat_d.basic.LED_idx,0xFF);
                break;
                case LED_PAT_GRAPH:
                    if(lin_idx<settings.value)
                    {
                        LED_stat[buffer_idx].colors[pat_i-1]=settings.color;
                        //add status bits
                        LED_stat[buffer_idx].colors[pat_i-1].brt|=LED_ST_BITS;
                    }
                    else
                    {
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                        LED_stat[buffer_idx].colors[pat_i-1].r=GRAPH_WHITE_BRT;
                        LED_stat[buffer_idx].colors[pat_i-1].g=GRAPH_WHITE_BRT;
                        LED_stat[buffer_idx].colors[pat_i-1].b=GRAPH_WHITE_BRT;
                    }
                break;
                case LED_SOLID_ST:
                    //set all LEDs from color
                    LED_stat[buffer_idx].colors[pat_i-1].r  =settings.color.r;
                    LED_stat[buffer_idx].colors[pat_i-1].g  =settings.color.g;
                    LED_stat[buffer_idx].colors[pat_i-1].b  =settings.color.b;
                    //set brightness
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                break;
                case LED_PAT_DAY:
                    if(lin_idx<FIN_LED)
                    {
                        //set fins off
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS;
                    }
                    else
                    {
                        //set all non-fin LEDs from color
                        LED_stat[buffer_idx].colors[pat_i-1].r  =settings.color.r;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =settings.color.g;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =settings.color.b;
                        //set brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                    }
                break;
                case LED_PAT_ST_LIST:
                    if(only_fins){
                        //set color from array
                        LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[((lin_idx/settings.value)%settings.list->num_colors)].color;
                        //set brightens from pattern color
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                    }
                    else
                    {
                        if(lin_idx<FIN_LED)
                        {
                            //fins bright white
                            LED_stat[buffer_idx].colors[pat_i-1].r  =settings.color.r;
                            LED_stat[buffer_idx].colors[pat_i-1].g  =settings.color.g;
                            LED_stat[buffer_idx].colors[pat_i-1].b  =settings.color.b;
                            //high brightness for fins
                            LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|brt_offset(settings.color.brt,9);

                        }
                        else
                        {
                            //set color from array
                            LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[(((lin_idx-FIN_LED)/settings.value)%settings.list->num_colors)].color;
                            //set brightens from pattern color
                            LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                        }
                    }
                break;
                case LED_PAT_FLASH_NOGAP:
                    //set color from list
                    LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[pat_d.basic.LED_idx].color;
                    //set brightens from pattern color
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                break;
                case LED_PAT_FLOW_LIST:
                    if(only_fins)
                    {
                        //set color from array
                        LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[( (((lin_idx+pat_d.basic.LED_idx))/settings.value)%settings.list->num_colors)].color;
                        //set color from pattern color
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                    }
                    else
                    {
                        if(lin_idx<FIN_LED)
                        {
                            //fins bright white
                            LED_stat[buffer_idx].colors[pat_i-1].r  =settings.color.r;
                            LED_stat[buffer_idx].colors[pat_i-1].g  =settings.color.g;
                            LED_stat[buffer_idx].colors[pat_i-1].b  =settings.color.b;
                            //high brightness for fins
                            LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|brt_offset(settings.color.brt,9);

                        }
                        else
                        {
                            //set color from array
                            LED_stat[buffer_idx].colors[pat_i-1]=settings.list->alt_color[( ((lin_idx+pat_d.basic.LED_idx)/settings.value)%settings.list->num_colors)].color;
                            //set color from pattern color
                            LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|settings.color.brt;
                        }
                    }
                break;
                case LED_PAT_COLOR_PARTICLE:
                case LED_PAT_PARTICLE:
                case LED_PAT_LIST_PARTICLE:

                    //set to off
                    LED_stat[buffer_idx].colors[pat_i-1].r=LED_stat[buffer_idx].colors[pat_i-1].g=LED_stat[buffer_idx].colors[pat_i-1].b=0;
                    //set brightness to zero
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;

                    for(j=strp_idx;j<pat_d.ptc.num;j+=LED_STR)
                    {
                        tmp1=lin_idx-pat_d.ptc.particle_pos[j];
                        if(tmp1>=0 && tmp1<=5)
                        {
                            //set color
                            LED_stat[buffer_idx].colors[pat_i-1].r  =pat_d.ptc.particles[j].color.r;
                            LED_stat[buffer_idx].colors[pat_i-1].g  =pat_d.ptc.particles[j].color.g;
                            LED_stat[buffer_idx].colors[pat_i-1].b  =pat_d.ptc.particles[j].color.b;
                            if(tmp1==0)
                            {
                                //set brightness
                                LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|pat_d.ptc.particles[j].color.brt;
                            }
                            else
                            {
                                //get brightness, mask out start bits
                                int tbrt=LED_stat[buffer_idx].colors[pat_i-1].brt&(~LED_ST_BITS);
                                //add brightness from particle
                                tbrt+=(6-tmp1);
                                //saturate brightness
                                if(tbrt>MAX_BRT)
                                {
                                    tbrt=MAX_BRT;
                                }
                                LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|(tbrt);
                            }
                        }
                    }
                break;
                case LED_PAT_UNIFORM_PARTICLE:
                case LED_PAT_COLOR_UNIFORM_PARTICLE:
                case LED_PAT_LIST_UNIFORM_PARTICLE:
                    //set to off
                    LED_stat[buffer_idx].colors[pat_i-1].r=LED_stat[buffer_idx].colors[pat_i-1].g=LED_stat[buffer_idx].colors[pat_i-1].b=0;
                    //set brightness to zero
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;

                    for(j=0;j<pat_d.ptc.num;j+=1)
                    {
                        tmp1=lin_idx-pat_d.ptc.particle_pos[j];
                        if(tmp1>=0 && tmp1<=5)
                        {
                            //set color
                            LED_stat[buffer_idx].colors[pat_i-1].r  =pat_d.ptc.particles[j].color.r;
                            LED_stat[buffer_idx].colors[pat_i-1].g  =pat_d.ptc.particles[j].color.g;
                            LED_stat[buffer_idx].colors[pat_i-1].b  =pat_d.ptc.particles[j].color.b;
                            if(tmp1==0)
                            {
                                //set brightness
                                LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|pat_d.ptc.particles[j].color.brt;
                            }
                            else
                            {
                                //get brightness, mask out start bits
                                int tbrt=LED_stat[buffer_idx].colors[pat_i-1].brt&(~LED_ST_BITS);
                                //add brightness from particle
                                tbrt+=(6-tmp1);
                                //saturate brightness
                                if(tbrt>MAX_BRT)
                                {
                                    tbrt=MAX_BRT;
                                }
                                LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|(tbrt);
                            }
                        }
                    }
                break;
                case LED_PAT_EYES_H:
                    if((lin_idx!=pat_d.basic.idx_dir && lin_idx!=pat_d.basic.idx_dir+2) || pat_d.basic.LED_idx<=0)
                    {
                        //turn of LED
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS;
                    }
                    else
                    {
                        //fade out LED
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|pat_d.basic.LED_idx;
                    }
                break;
                case LED_PAT_WAVE_BIG_U:
                case LED_PAT_WAVE_BIG_D:
                case LED_PAT_WAVE_SM_U:
                case LED_PAT_WAVE_SM_D:
                    //set color
                    LED_stat[buffer_idx].colors[pat_i-1].r  = settings.color.r;
                    LED_stat[buffer_idx].colors[pat_i-1].g  = settings.color.g;
                    LED_stat[buffer_idx].colors[pat_i-1].b  = settings.color.b;

                    //calculate index in pattern
                    tmp1=lin_idx+pat_d.basic.LED_idx;
                    tmp1=tmp1%settings.value;

                    //calculate midpoint
                    tmp2=(settings.value+1)/2;
                    //check if we are past the midpoint
                    if(tmp1>tmp2)
                    {
                        //shift so we get a V
                        tmp1=(settings.value+1)-tmp1;
                    }

                    //for the small patterns subtract more
                    if(LED_pattern==LED_PAT_WAVE_SM_U || LED_pattern==LED_PAT_WAVE_SM_D)
                    {
                        //multiply value by four
                        tmp1*=4;
                    }

                    tmp1=(settings.color.brt)-tmp1;
                    if(tmp1<0)
                    {
                        tmp1=0;
                    }
                    LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|tmp1;

                    if(lin_idx==(LED_LEN-1) && strp_idx==0)
                    {
                        nosecone_pattern_mode(NC_MODE_STATIC,tmp1*128+127,NC_NA,NC_NA,NC_NA);
                    }
                break;
                case LED_PAT_WAVE_HUE_D:
                case LED_PAT_WAVE_HUE_U:
                    //calculate index in pattern
                    tmp1=lin_idx+pat_d.basic.LED_idx;
                    tmp1=tmp1%settings.value;

                    //scale value to get to full scale
                    tmp1*=pat_d.basic.idx_dir;

                    //calculate color in RGB. Use the green and blue values from the pattern color as saturation and value
                    HsvToLED(&LED_stat[buffer_idx].colors[pat_i-1],settings.color.brt,tmp1,settings.color.g,settings.color.b);
                break;
                case LED_PAT_WAVE_SAT_D:
                case LED_PAT_WAVE_SAT_U:

                    //calculate index in pattern
                    tmp1=lin_idx+pat_d.basic.LED_idx;
                    tmp1=tmp1%settings.value;

                    //calculate midpoint
                    tmp2=(settings.value+1)/2;
                    //check if we are past the midpoint
                    if(tmp1>tmp2)
                    {
                        //shift so we get a V
                        tmp1=(settings.value+1)-tmp1;
                    }
                    //scale value to get to full scale
                    tmp1*=pat_d.basic.idx_dir;

                    //calculate color in RGB. Use the red and blue values from the pattern color as hue and value
                    HsvToLED(&LED_stat[buffer_idx].colors[pat_i-1],settings.color.brt,settings.color.r,tmp1,settings.color.b);
                    //set nosecone brightness based on saturation
                    if(lin_idx==(LED_LEN-1) && strp_idx==0)
                    {
                        nosecone_pattern_mode(NC_MODE_STATIC,tmp1*16+15,NC_NA,NC_NA,NC_NA);
                    }
                break;
                case LED_PAT_POWER_PANIC:
                    if((pat_d.basic.LED_idx==1 || pat_d.basic.LED_idx==3) && lin_idx&1)
                    {
                        //set color to red
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|8;
                    }
                    else
                    {
                        //set LED's off
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;
                    }
                break;
                case LED_PAT_MODE_PANIC:
                    if(pat_d.basic.LED_idx==1 ||pat_d.basic.LED_idx==3)
                    {
                        //set color to red
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|8;
                    }
                    else
                    {
                        //set LED's off
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;
                    }
                break;
                case LED_PAT_RESET_PANIC:
                    if(pat_d.basic.LED_idx==1 ||pat_d.basic.LED_idx==3)
                    {
                        //set color to red yellow
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|8;
                    }
                    else
                    {
                        //set LED's off
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;
                    }
                break;
                case LED_PAT_PATTERN_PANIC:
                    if(pat_d.basic.LED_idx==1)
                    {
                        //set color to red
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|8;
                    }
                    else if(pat_d.basic.LED_idx==3)
                    {
                        //set color to yellow
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0xFF;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|8;
                    }
                    else
                    {
                        //set LED's off
                        LED_stat[buffer_idx].colors[pat_i-1].r  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].g  =0;
                        LED_stat[buffer_idx].colors[pat_i-1].b  =0;
                        //set fixed, medium brightness
                        LED_stat[buffer_idx].colors[pat_i-1].brt=LED_ST_BITS|0;
                    }
                break;
            }
        }
    }
    else if(pat_i==NUM_LEDS+1)
    {
        //send new info
        LEDs_send(&LED_stat[buffer_idx]);
        //done!
        return 1;
    }
    else
    {
        //nothing to do
        return 2;
    }
    //advance index
    pat_i++;
    //not done
    return 0;
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

    //set nosecone pattern mode off by default
    nosecone_pattern_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);

    //init indexes
    switch(LED_pattern)
    {
        case LED_PAT_COLORTRAIN:
            pat_d.basic.idx_dir=1;
            pat_d.basic.LED_idx=-2;
            //set interrupt interval
            flash_per=102*2;
        break;
        case LED_PAT_FLASH_GAP:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=20;
        break;
        case LED_PAT_BOOST:
            pat_d.basic.LED_idx=-1;
            //set interrupt interval
            flash_per=2000;
        break;
        case LED_PAT_HUE:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_SATURATION:
        pat_d.basic.idx_dir=0;
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=51;
        break;
        case LED_PAT_BURST:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=102;
        break;
        case LED_PAT_FLASH_NOGAP:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=2048;
        break;
        case LED_PAT_STR_ST:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_FLOW_LIST:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_COLOR_PARTICLE:
        case LED_PAT_PARTICLE:
        case LED_PAT_UNIFORM_PARTICLE:
        case LED_PAT_COLOR_UNIFORM_PARTICLE:
        case LED_PAT_LIST_UNIFORM_PARTICLE:
        case LED_PAT_LIST_PARTICLE:
            pat_d.basic.LED_idx=LED_LEN;
            //set interrupt interval
            flash_per=10;
            for(i=0;i<NUM_PARTICLES;i++)
            {
                new_particle(&pat_d.ptc.particles[i],LED_pattern);
            }
        break;
        case LED_PAT_EYES_H:
            pat_d.basic.LED_idx=0;
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
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=70;
        break;
        case LED_PAT_POWER_PANIC:
        case LED_PAT_MODE_PANIC:
        case LED_PAT_RESET_PANIC:
        case LED_PAT_PATTERN_PANIC:
            pat_d.basic.LED_idx=0;
            //set interrupt interval
            flash_per=200;
            // mode unconditionally
            nosecone_pattern_mode(NC_MODE_FLASH,0xFFF,0,200,100);
            nosecone_mode(NC_MODE_PATTERN,NC_NA,NC_NA,NC_NA,NC_NA);
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
        //don't use nosecone_pattern_mode here so it is shut off always
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
        //we need to exit LPM to update flash pattern
        LPM0_EXIT;
    }
}

// ============ TA1.1 ISR ============

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt void sim_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) sim_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(__even_in_range(TA1IV,TA1IV_TAIFG))
    {
    case TA1IV_TACCR1:
        //set next interrupt time
        TA1CCR1+=sim_int;
        //set sim advance flag
        e_flags|=FM_SIM_ADVANCE;
        //exit low power mode
        LPM0_EXIT;
    break;
    }

}

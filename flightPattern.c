/*
 * flightPattern.c
 *
 *  Created on: Apr 26, 2020
 *      Author: jesse
 */

#include "flightPattern.h"
#include "flashPattern.h"
#include "Nosecone.h"
#include <stddef.h>


const FLIGHT_PATTERN patterns[]={
                                 {"test",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_WAVE_BIG_D,.value=12,.color={.brt=8,.r=0xFF,.g=0x00,.b=0x00}}}},
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_BOOST,.value=0,.color={.brt=17,.r=0xFF,.g=0,.b=0}}}},
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=17,.r=0xFF,.g=0,.b=0}}}},
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=17,.r=0xFF,.g=0,.b=0}}}},
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightGraph,.data=&(LED_color){.brt=17,.r=0xFF,.g=0,.b=0}}}},
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=0x00,.b=0x00}}}},
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_SATURATION,.color={.brt=2,.r=0xFF,.g=0xFF,.b=0xFF}}}},
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PANIC}}},
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}}},
                                         }
                                 },
                                 {"USA",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLOW_LIST,.value=3,.color={.brt=8,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_RW_colors}}},
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}}},
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}}},
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}}},
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLASH_GAP,.color={.brt=31,.r=0xFF,.g=0xFF,.b=0xFF},.list=&USA_RW_colors}}},
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightListParticle,.data=(void*)&USA_colors}}},
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLOW_LIST,.value=2,.color={.brt=4,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_RW_colors}}},
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PANIC}}},
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}}},
                                         }
                                 },
                                 {"rnbw",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}}},
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLOW_LIST,.value=6,.color={.brt=8,.r=255,.g=200,.b=150},.list=&RNBW_colors}}},
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}}},
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}}},
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}}},
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLASH_GAP,.color={.brt=31,.r=0xFF,.g=0xFF,.b=0xFF},.list=&RNBW_colors}}},
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightListParticle,.data=(void*)&RNBW_colors}}},
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,{.basic={.pattern=LED_PAT_FLOW_LIST,.value=9,.color={.brt=4,.r=255,.g=200,.b=150},.list=&RNBW_colors}}},
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PANIC}}},
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}}},
                                         }
                                 },
                                 {NULL,{}}
};


int proc_flightP(const struct ao_companion_command *new_dat,const FLIGHT_PATTERN *pattern,int last)
{
    int fm_new=last!=new_dat->flight_state;
    const FLIGHT_MODE_HANDLER *handler;

    if(new_dat->command>=AO_FLIGHT_NUM)
    {
        //set panic mode
        panicPattern();
        //invalid mode, nothing left to do
        return new_dat->flight_state;
    }
    //get the handler for this mode
    handler=&pattern->handlers[new_dat->flight_state];
    //check what type of command we are processing
    switch(handler->type)
    {
        case FLIGHT_TYPE_NO_CHANGE:
        break;
        case FLIGHT_TYPE_PATTERN:
            if(fm_new)
            {
                flashPatternChange(handler->basic.pattern);
            }
        break;
        case FLIGHT_TYPE_PATTERN_VAL_COLOR:
            if(fm_new)
            {
                flashPatternVC(handler->basic.pattern,handler->basic.value,handler->basic.color);
            }
        break;
        case FLIGHT_TYPE_COLOR:
            if(fm_new)
            {
                flashPattern_setColor(handler->basic.color);
            }
        case FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST:
            if(fm_new)
            {
                flashPatternVCL(handler->basic.pattern,handler->basic.value,handler->basic.color,handler->basic.list);
            }
        break;
        case FLIGHT_TYPE_PATTERN_COMPLEX:
            handler->cb.callback(new_dat,fm_new,handler->cb.data);
        break;
        default:
            //not a valid pattern, Panic!!
            panicPattern();
        break;
    }
    //return current mode
    return new_dat->flight_state;
}

void fp_speedGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxSpeed=0;
    if(fm_new)
    {
        if(cmd_dat->speed>maxSpeed)
        {
            maxSpeed=cmd_dat->speed;
        }
        flashPatternVC(LED_PAT_GRAPH,0,*((LED_color*)color));
        chute_mode(NC_MODE_STATIC,NC_MAX_PWM,NC_NA,NC_NA,NC_NA);
    }
    else
    {
        flashPattern_setValue(LED_LEN-(cmd_dat->speed*LED_LEN)/maxSpeed);
    }
    //set nosecone based on speed16-(cmd_dat->height*12)/maxHeight
    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM*(cmd_dat->speed/(float)maxSpeed),NC_NA,NC_NA,NC_NA);
}

void fp_heightGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVC(LED_PAT_GRAPH,0,*((LED_color*)color));
        chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
    }
    else
    {
        flashPattern_setValue((cmd_dat->height*LED_LEN)/maxHeight);
    }
    //set nosecone based on height
    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM*(cmd_dat->height/(float)maxHeight),NC_NA,NC_NA,NC_NA);
}

void fp_heightParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVC(LED_PAT_UNIFORM_PARTICLE,4,*((LED_color*)color));
        chute_mode(NC_MODE_FLASH,NC_MAX_PWM,0,100,500);
    }
    else
    {
        flashPattern_setValue(16-(cmd_dat->height*12)/maxHeight);
    }
}
void fp_heightListParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *list)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVCL(LED_PAT_LIST_UNIFORM_PARTICLE,4,(LED_color){.brt=31,.r=0xFF,.g=0xFF,.b=0xFF},(COLOR_LIST*)list);
        chute_mode(NC_MODE_FLASH,NC_MAX_PWM,0,100,500);
    }
    else
    {
        flashPattern_setValue(16-(cmd_dat->height*12)/maxHeight);
    }
}

void fp_colorHeight(const struct ao_companion_command *cmd_dat,int fm_new, void *_list)
{
    COLOR_LIST *list=(COLOR_LIST*)_list;
    LED_color color=color=list->alt_color[0].color;
    int i;
    for(i=0;i<list->num_colors;i++)
    {
        if(cmd_dat->height>list->alt_color[i].alt)
        {
            color=list->alt_color[i].color;
        }
        if(cmd_dat->height<list->alt_color[i].alt)
        {
            break;
        }
    }
    //set maximum brightness
    color.brt=MAX_BRT;

    if(fm_new)
    {
        flashPatternVC(LED_SOLID_ST,0,color);
    }
    else
    {
        flashPattern_setColor(color);
    }
}


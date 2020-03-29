/*
 * commands.c
 *
 *  Created on: Mar 23, 2020
 *      Author: jesse
 */

#include "terminal.h"
#include "Companion.h"
#include "LEDs.h"
#include "flashPattern.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int brt_Cmd(int argc,char **argv)
{
    long int temp;
    char *eptr;
    int LED_brt;
    int i;
    //check number of arguments
    if(argc!=1)
    {
        printf("Error : %s requires exactly one argument\r\n",argv[0]);
        return 1;
    }

    //parse brightness
    temp=strtol(argv[1],&eptr,10);

    //check if the whole string was parsed
    if(*eptr)
    {
        //end of string not found
        printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1],eptr);
        return 2;
    }

    //check limits
    if(temp>MAX_BRT)
    {
        printf("Error : LED brightness must be less than %i. got %li\r\n",MAX_BRT,temp);
        return 4;
    }
    if(temp<0)
    {
        printf("Error : LED brightness can not be less than zero. got %li\r\n",temp);
        return 5;
    }
    LED_brt=temp;

    //add start bits
    LED_brt|=LED_ST_BITS;

    //set manual mode
    flashPatternChange(LED_PAT_MAN);


    for(i=0;i<NUM_LEDS;i++)
    {
        LED_stat[0].colors[i].brt=LED_brt;
    }


    //send new info
    LEDs_send(&LED_stat[0]);

    return 0;
}

int LED_Cmd(int argc,char **argv)
{
    enum{LED_SPEC_SINGLE=0,LED_SPEC_ALL,LED_SPEC_FINS};
    int LED_spec,LED_num,LED_brt;
    char *eptr;
    int i;
    long int temp;
    int HSV;
    int c[3];
    LED_color color;
    //arg1  arg2    arg3    arg4 arg5 arg6
    //num   brt     HSV|RGB h|r  s|g  v|b

    if(argc!=6)
    {
        printf("Expected 6 arguments but got %i\r\n",argc);
        return 1;
    }

    //parse LED number
    if(!strcmp("all",argv[1]))
    {
        LED_spec=LED_SPEC_ALL;
    }
    else
    {
        //attempt to parse number
        temp=strtol(argv[1],&eptr,10);

        //check if the whole string was parsed
        if(*eptr)
        {
            //end of string not found
            printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1],eptr);
            return 2;
        }

        if(temp<0)
        {
            printf("Error : LED num can not be less than zero. got %li\r\n",temp);
            return 3;
        }
        if(temp>=NUM_LEDS)
        {
            printf("Error : LED num must be less than %i. got %li\r\n",NUM_LEDS,temp);
            return 3;
        }
        //set number
        LED_num=temp;
        LED_spec=LED_SPEC_SINGLE;
    }

    //parse brightness
    temp=strtol(argv[2],&eptr,10);

    //check if the whole string was parsed
    if(*eptr)
    {
        //end of string not found
        printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[2],eptr);
        return 2;
    }

    //check limits
    if(temp>MAX_BRT)
    {
        printf("Error : LED brightness must be less than %i. got %li\r\n",MAX_BRT,temp);
        return 4;
    }
    if(temp<0)
    {
        printf("Error : LED brightness can not be less than zero. got %li\r\n",temp);
        return 5;
    }
    LED_brt=temp;


    //parse color type
    if(!strcmp("HSV",argv[3]))
    {
        HSV=1;
    }
    else if(!strcmp("RGB",argv[3]))
    {
        HSV=0;
    }
    else
    {
        printf("Error while parsing \"%s\" expecting HSV or RGB\r\n",argv[3]);
        return 3;
    }

    //parse color arguments
    for(i=0;i<3;i++)
    {
        //parse value
        temp=strtol(argv[4+i],&eptr,10);

        //check if the whole string was parsed
        if(*eptr)
        {
            //end of string not found
            printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[4+i],eptr);
            return 2;
        }

        if(temp>0xFF)
        {
            printf("Error : color values must be less than 255. got %li",temp);
            return 4;
        }
        if(temp<0)
        {
            printf("Error : color values must be greater than zero. got %li",temp);
            return 5;
        }
        c[i]=temp;
    }

    //set manual mode
    flashPatternChange(LED_PAT_MAN);

    if(HSV)
    {
        //convert to RGB
        HsvToLED(&color,LED_brt,c[0],c[1],c[2]);
    }
    else
    {
        //set values
        color.r=c[0];
        color.g=c[1];
        color.b=c[2];
        color.brt=LED_ST_BITS|LED_brt;
    }

    switch(LED_spec)
    {
    case LED_SPEC_SINGLE:
        LED_stat[0].colors[LED_num]=color;
        break;
    case LED_SPEC_ALL:
        for(i=0;i<NUM_LEDS;i++)
        {
            LED_stat[0].colors[i]=color;
        }
        break;
    //case LED_SPEC_FINS:
    //    break;
    default:
        printf("Error : internal error\r\n");
        return -1;
    }


    //send new info
    LEDs_send(&LED_stat[0]);

    return 0;
}

const struct {
    const char *name;
    int val;
} pattern_names[]={
                 {"off",LED_PAT_OFF},
                 {"static",LED_PAT_ST_COLORS},
                 {"colortrain",LED_PAT_COLORTRAIN},
                 {"hue",LED_PAT_HUE},
                 {"burst",LED_PAT_BURST},
                 {"sat",LED_PAT_SATURATION},
                 {"USAst",LED_PAT_ST_USA},
                 {"USA",LED_PAT_USA},
                 {"pad",LED_PAT_PAD},
                 {"boost",LED_PAT_BOOST},
                 {"graph",LED_PAT_GRAPH},
                 {"rnbw_st",LED_PAT_RNBW_ST},
                 {"rnbw_fs",LED_PAT_RNBW_FLASH},
                 {"rnbw_fl",LED_PAT_RNBW_FLOW},
                 {"USA_fl",LED_PAT_USA_FLOW},
                 {"particle",LED_PAT_PARTICLE},
                 {NULL,0}
};


int patternCmd(int argc,char **argv)
{
    int i,val;
    const char *name;
    if(argc==0)
    {
        //get current pattern
        val=flashPatternGet();
        //print message
        printf("Possible pattern names :\r\n");
        //loop through patterns
        for(i=0;pattern_names[i].name!=NULL;i++)
        {
            printf("\t%c%s\r\n",pattern_names[i].val==val?'*':' ',pattern_names[i].name);
        }
    }
    else
    {
        val=-1;
        for(i=0;pattern_names[i].name!=NULL;i++)
        {
            if(!strcmp(pattern_names[i].name,argv[1]))
            {
                val=pattern_names[i].val;
                name=pattern_names[i].name;
                break;
            }
        }
        if(val==-1)
        {
            printf("Error : Unknown pattern \"%s\"\r\n",argv[1]);
            return 1;
        }
        else
        {
            flashPatternChange(val);
            printf("LED pattern set to \'%s\"\r\n",name);
        }
    }
    return  0;
}

const struct {
    const char *name,*unit;
    float scale,offset;
} ADC_chans[]={
               {"Battery Voltage","V",8.5,0},
               {"Battery Current","A",2,0},
               {"LED Voltage","V",1.5625,0},
               {"LED Current","A",2,0},
               {"MSP Voltage","V",2,0},
               {"Regulator Temp","C",51.282051,-20.641026}
};


int ADC_Cmd(int argc,char **argv)
{
    int i;
    //get values from telemitry structure
    int *vals=((int*)&cpTLM)+1;
    printf("ADC values\r\n");
    for(i=0;i<6;i++)
    {
        printf("%-15s : %f %s\r\n",ADC_chans[i].name,
               vals[i]*ADC_chans[i].scale*(1.2)/((float)((int)0x7FFF))+ADC_chans[i].offset,
               ADC_chans[i].unit);
    }
    return 0;
}

int pnext_Cmd(int argc,char **argv)
{
    flashPatternNext();
    return 0;
}

const CMD_SPEC cmd_tbl[]={
                          {"help","get help on commands",helpCmd},
                          {"LED","Change LED stuff",LED_Cmd},
                          {"brt","Change the LED brightness",brt_Cmd},
                          {"pat","Change/query LED pattern",patternCmd},
                          {"ADC","get values from the ADC",ADC_Cmd},
                          {"pnext","switch to the next LED flash pattern",pnext_Cmd},
                          {NULL,NULL,NULL}
};

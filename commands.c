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
#include <limits.h>
#include <msp430.h>
#include "Nosecone.h"
#include "flightPattern.h"
#include "Companion.h"
#include "events.h"
#include "reset.h"
#include "settings.h"


int brt_Cmd(int argc,char **argv)
{
    long int temp;
    char *eptr;
    int LED_brt;
    //check number of arguments

    if(argc==1)
    {
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

        settings.color.brt=LED_brt;
    }
    else if(argc!=0)
    {

        printf("Error : %s requires zero or one arguments\r\n",argv[0]);
        return 1;
    }
    printf("Brightness %u\r\n",settings.color.brt);
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
            printf("Error : color values must be less than 255. got %li\r\n",temp);
            return 4;
        }
        if(temp<0)
        {
            printf("Error : color values must be greater than zero. got %li\r\n",temp);
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


int color_Cmd(int argc,char **argv)
{

    int LED_brt;
    char *eptr;
    int i;
    long int temp;
    int c[3];
    LED_color color;


    if(argc==4)
    {
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

        //parse color values
        for(i=0;i<3;i++)
        {
            //parse value
            temp=strtol(argv[2+i],&eptr,0);

            //check if the whole string was parsed
            if(*eptr)
            {
                //end of string not found
                printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[2+i],eptr);
                return 2;
            }

            if(temp>0xFF)
            {
                printf("Error : color values must be less than 255. got %li\r\n",temp);
                return 4;
            }
            if(temp<0)
            {
                printf("Error : color values must be greater than zero. got %li\r\n",temp);
                return 5;
            }
            c[i]=temp;
        }


        //set values
        color.r=c[0];
        color.g=c[1];
        color.b=c[2];
        color.brt=LED_ST_BITS|LED_brt;

        flashPattern_setColor(color);
    }
    else if(argc!=0)
    {
        printf("Expected 4 or zero arguments but got %i\r\n",argc);
        return 1;
    }
    //print color
    printf("color : 0x%02X 0x%02X 0x%02X 0x%02X\r\n",settings.color.brt,settings.color.r,settings.color.g,settings.color.b);
    return 0;
}

int value_Cmd(int argc,char **argv)
{
    char *eptr;
    long int temp;

    if(argc==1)
    {
        //parse value
        temp=strtol(argv[1],&eptr,0);

        //check if the whole string was parsed
        if(*eptr)
        {
            //end of string not found
            printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1],eptr);
            return 2;
        }

        if(temp>UINT_MAX)
        {
            printf("Error : color values must be less than %u. got %li\r\n",UINT_MAX,temp);
            return 4;
        }
        if(temp<0)
        {
            printf("Error : value must be greater than or equal to zero. got %li\r\n",temp);
            return 5;
        }
        flashPattern_setValue(temp);
    }
    else if(argc!=0)
    {
        printf("Expected 1 or zero arguments but got %i\r\n",argc);
        return 1;
    }
    printf("Value : %i\r\n",settings.value);
    return 0;
}

const struct {
    const char *name;
    int val;
} pattern_names[]={
                 {"off",LED_PAT_OFF},
                 {"colortrain",LED_PAT_COLORTRAIN},
                 {"hue",LED_PAT_HUE},
                 {"burst",LED_PAT_BURST},
                 {"sat",LED_PAT_SATURATION},
                 {"str_st",LED_PAT_STR_ST},
                 {"fs_gap",LED_PAT_FLASH_GAP},
                 {"boost",LED_PAT_BOOST},
                 {"graph",LED_PAT_GRAPH},
                 {"day",LED_PAT_DAY},
                 {"st_list",LED_PAT_ST_LIST},
                 {"fs_nogap",LED_PAT_FLASH_NOGAP},
                 {"fl_list",LED_PAT_FLOW_LIST},
                 {"particle",LED_PAT_PARTICLE},
                 {"eyes_h",LED_PAT_EYES_H},
                 {"wave_bu",LED_PAT_WAVE_BIG_U},
                 {"wave_bd",LED_PAT_WAVE_BIG_D},
                 {"wave_su",LED_PAT_WAVE_SM_U},
                 {"wave_sd",LED_PAT_WAVE_SM_D},
                 {"hwave_d",LED_PAT_WAVE_HUE_D},
                 {"hwave_u",LED_PAT_WAVE_HUE_U},
                 {"swave_d",LED_PAT_WAVE_SAT_D},
                 {"swave_u",LED_PAT_WAVE_SAT_U},
                 {"ppanic",LED_PAT_POWER_PANIC},
                 {"mpanic",LED_PAT_MODE_PANIC},
                 {"rpanic",LED_PAT_RESET_PANIC},
                 {"ptpanic",LED_PAT_PATTERN_PANIC},
                 {"cparticle",LED_PAT_COLOR_PARTICLE},
                 {"uparticle",LED_PAT_UNIFORM_PARTICLE},
                 {"cuparticle",LED_PAT_COLOR_UNIFORM_PARTICLE},
                 {"luparticle",LED_PAT_LIST_UNIFORM_PARTICLE},
                 {"lparticle",LED_PAT_LIST_PARTICLE},
                 {NULL,0}
};


int patternCmd(int argc,char **argv)
{
    int i,val;
    const char *name;
    char *eptr;
    unsigned long int temp;

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
            //parse value
            temp=strtoul(argv[1],&eptr,0);
            if(*eptr)
            {
                printf("Error : Unknown pattern \"%s\"\r\n",argv[1]);
                return 1;
            }
            if(temp>=LED_NUM_PAT)
            {
                printf("Error : valid patterns are 0 to %u\r\n",LED_NUM_PAT-1);
                return 2;
            }
            val=temp;
            printf("LED pattern set to #%u\r\n",val);
        }
        else
        {
            printf("LED pattern set to \'%s\'\r\n",name);
        }
        flashPatternChange(val);
    }
    return  0;
}

const struct {
    const char *name,*unit;
    float scale,offset;
} ADC_chans[]={
               {"Battery Voltage","V",6.3667,0},
               {"Battery Current","A",2,0},
               {"LED Voltage","V",4.8802,0},
               {"LED Current","A",2,0},
               {"MSP Voltage","V",2.75,0},
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

const struct{
    const char *name;
    int value;
} NC_modes[]={
           {"static",NC_MODE_STATIC},
           {"fade",NC_MODE_FADE},
           {"flash",NC_MODE_FLASH},
           {"blip",NC_MODE_ONE_SHOT},
           {"pattern",NC_MODE_PATTERN}
};

int NC_Cmd(int argc,char **argv)
{
    int NC_args[5];
    int i,j;
    int found;
    char *eptr;
    long int temp;


    if(argc==0)
    {
        unsigned short ncVal;
        ncVal=getNoseconeLED();
        printf("Nosecone PWM : 0X%03X = %.2f %%\r\n",ncVal,100.0*ncVal/(float)NC_MAX_PWM);
        NC_debug();
    }
    else
    {
        for(i=0;i<5;i++)
        {
            //check if we have arguments to parse
            if(i<argc)
            {
                found=0;
                if(i==0)
                {
                    //on first argument check for mode names
                    for(j=0;j<NC_MODE_NUM;j++)
                    {
                        if(!strcmp(argv[1+i],NC_modes[j].name))
                        {
                            NC_args[i]=NC_modes[j].value;
                            //set flag
                            found=1;
                            //skip rest of check
                            break;
                        }
                    }
                }
                //if we didn't find a matching mode, parse as number
                if(!found)
                {
                    //parse value
                    temp=strtol(argv[1+i],&eptr,0);

                    //check if the whole string was parsed
                    if(*eptr)
                    {
                        //end of string not found
                        printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1+i],eptr);
                        return 2;
                    }

                    if(temp>INT_MAX)
                    {
                        printf("Error : values must be less than %i. got %li\r\n",INT_MAX,temp);
                        return 4;
                    }
                    if(temp<INT_MIN)
                    {
                        printf("Error : values must be greater than %i. got %li\r\n",INT_MIN,temp);
                        return 5;
                    }
                    NC_args[i]=temp;
                }
            }
            else
            {
                //no value given, pass don't care value
                NC_args[i]=NC_NA;
            }
        }
        nosecone_mode(NC_args[0],NC_args[1],NC_args[2],NC_args[3],NC_args[4]);
    }
    return 0;
}
int chute_Cmd(int argc,char **argv)
{
    int chute_args[5];
    int i,j;
    int found;
    char *eptr;
    long int temp;


    if(argc==0)
    {
        unsigned short chuteVal;
        chuteVal=getChuteLED();
        printf("Chute PWM    : 0X%03X = %.2f %%\r\n",chuteVal,100.0*chuteVal/(float)NC_MAX_PWM);
        chute_debug();
    }
    else
    {
        //parse chute mode
        for(i=0;i<5;i++)
        {
            //check if we have arguments to parse
            if(i<argc)
            {
                found=0;
                if(i==0)
                {
                    //on first argument check for mode names
                    for(j=0;j<NC_MODE_NUM;j++)
                    {
                        if(!strcmp(argv[1+i],NC_modes[j].name))
                        {
                            chute_args[i]=NC_modes[j].value;
                            //set flag
                            found=1;
                            //skip rest of check
                            break;
                        }
                    }
                }
                //if we didn't find a matching mode, parse as number
                if(!found)
                {
                    //parse value
                    temp=strtol(argv[1+i],&eptr,0);

                    //check if the whole string was parsed
                    if(*eptr)
                    {
                        //end of string not found
                        printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1+i],eptr);
                        return 2;
                    }

                    if(temp>INT_MAX)
                    {
                        printf("Error : color values must be less than %i. got %li\r\n",INT_MAX,temp);
                        return 4;
                    }
                    if(temp<INT_MIN)
                    {
                        printf("Error : color values must be greater than %i. got %li\r\n",INT_MIN,temp);
                        return 5;
                    }
                    chute_args[i]=temp;
                }
            }
            else
            {
                //no value given, pass don't care value
                chute_args[i]=NC_NA;
            }
        }
        chute_mode(chute_args[0],chute_args[1],chute_args[2],chute_args[3],chute_args[4]);
    }
    return 0;
}

//flight pattern command
int fpat_Cmd(int argc,char **argv)
{
    char current;
    int pat_idx;

    if(argc==0)
    {
        for(pat_idx=0;flight_patterns[pat_idx].name[0]!='\0';pat_idx++)
        {
            //check if this is the current flight pattern
            if(!strncmp(flight_patterns[pat_idx].name,settings.flightp,FP_NAME_LEN))
            {
                //marker for current
                current='>';
            }
            else
            {
                //not current
                current=' ';
            }
            //print name
            printf("\t%c%s\r\n",current,flight_patterns[pat_idx].name);
        }
        return 0;
    }
    else
    {
        pat_idx=find_flightP(argv[1]);
        //check if pattern was found
        if(pat_idx<0)
        {
            printf("Error : could not find pattern matching \"%s\"\r\n",argv[1]);
            return 1;
        }

        strncpy(settings.flightp,flight_patterns[pat_idx].name,FP_NAME_LEN);
        //ensure there is a null byte
        settings.flightp[FP_NAME_LEN-1]='\0';

        return 0;
    }
}

int sim_Cmd(int argc,char **argv)
{
    const struct ao_companion_command *dat_ptr=flight_dat;
    int fp_idx;
    unsigned int mul=1;
    int i;
    uint8_t last=ao_flight_invalid;
    e_type wake_e;
    char *eptr;
    unsigned long int temp;
    //true if flash pattern has been updated
    int fp_done=0;

    //find pattern from settings
    fp_idx=find_flightP(settings.flightp);
    //check if pattern was found
    if(fp_idx<0)
    {
        printf("Error : could not find pattern matching \"%s\"\r\n",settings.flightp);
        return 1;
    }

    if(argc>0)
    {
        //parse value
        temp=strtoul(argv[1],&eptr,0);

        //check if the whole string was parsed
        if(*eptr)
        {
            //end of string not found
            printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[1],eptr);
            return 2;
        }

        if(temp>UINT_MAX)
        {
            printf("Error : multiplier must be less than %u. got %li\r\n",UINT_MAX,temp);
            return 4;
        }
        if(temp<=0)
        {
            printf("Error : multiplier must be greater than zero\r\n");
            return 5;
        }
        mul=temp;
    }

    //print pattern name
    printf("Starting simulation of \"%s\"\r\n",flight_patterns[fp_idx].name);
    //set interval to the time separation in the first two packets
    sim_int=(dat_ptr[1].tick-dat_ptr[0].tick)*10*mul;

    for(i=0;dat_ptr[i].command!=0;i++)
    {
        printf("Time = %4.1f  Alt = %6i Speed = %6.2f\r\n",AO_TICKS_TO_SEC((float)dat_ptr[i].tick),dat_ptr[i].height,dat_ptr[i].speed/((float)16));
        last=proc_flightP(&dat_ptr[i],&flight_patterns[fp_idx],last);
        //wait for the interval to elapse
        do
        {
            //advance flash pattern
            fp_done=flashPatternStep();
            if(fp_done)
            {
                //go into LPM0 if flash pattern is updated and there are no event flags
                LPM0_check();
            }
            //read interrupts
            wake_e=e_get_clear();
        }
        while(!(wake_e&FM_SIM_ADVANCE));
    }
    printf("Flight complete!\r\n");

    return 0;

}


#define CUSTOM_LIST_COLORS      (10)

static char custom_clist[sizeof(COLOR_LIST)+CUSTOM_LIST_COLORS*(sizeof(LED_color)+sizeof(int))];

const struct{
    const char * name;
    const COLOR_LIST *list;
} clists[]={
          {"rnbw",&RNBW_colors},
          {"USA",&USA_colors},
          {"RGB",&RGB_colors},
          {"USA_RW",&USA_RW_colors},
          {"custom",(COLOR_LIST*)custom_clist},
          {NULL,NULL}
};



int clist_Cmd(int argc,char **argv)
{
    const COLOR_LIST *list=NULL;
    COLOR_LIST *cust_list=(COLOR_LIST*)custom_clist;
    int LED_brt;
    char *eptr;
    int i,j;
    long int temp;
    int height;
    int c[3];

    if(argc==0)
    {
        printf("Color Lists:\r\n");
        for(i=0;clists[i].name!=NULL;i++)
        {
            printf("\t%c%s\r\n",(settings.list==clists[i].list)?'>':' ',clists[i].name);
        }
    }
    else
    {
        if(!strcmp("print",argv[1]))
        {
            list=(COLOR_LIST*)custom_clist;
            printf("idx\t""alt\t""Brt\t""Red\t""Green\t""Blue\r\n");
            for(i=0;i<list->num_colors;i++)
            {
                printf("%i\t%i\t%u\t%u\t%u\t%u\r\n",i,list->alt_color[i].alt,list->alt_color[i].color.brt,list->alt_color[i].color.r,list->alt_color[i].color.g,list->alt_color[i].color.b);
            }
            return 0;
        }
        else if(!strcmp("add",argv[1]))
        {
            if(cust_list->num_colors>=CUSTOM_LIST_COLORS)
            {
                printf("Error : list can not have more than %u entries\r\n",CUSTOM_LIST_COLORS);
                return 1;
            }
            if(argc<6)
            {
                printf("Error : add requires 5 extra arguments\r\n");
                return 2;
            }
            //parse height
            temp=strtol(argv[2],&eptr,0);

            //check if the whole string was parsed
            if(*eptr)
            {
              //end of string not found
              printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[2],eptr);
              return 2;
            }

            if(temp>INT_MAX)
            {
              printf("Error : altitude must be less than %i. got %li\r\n",UINT_MAX,temp);
              return 4;
            }
            if(temp<INT_MIN)
            {
              printf("Error : altitude must be greater than %i but got %li\r\n",INT_MIN,temp);
              return 5;
            }
            //set height
            height=temp;

            //parse brightness
            temp=strtol(argv[3],&eptr,10);

            //check if the whole string was parsed
            if(*eptr)
            {
               //end of string not found
               printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[3],eptr);
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

            //parse color values
            for(i=0;i<3;i++)
            {
                //parse value
                temp=strtol(argv[4+i],&eptr,0);

                //check if the whole string was parsed
                if(*eptr)
                {
                    //end of string not found
                    printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[4+i],eptr);
                    return 2;
                }

                if(temp>0xFF)
                {
                    printf("Error : color values must be less than 255. got %li\r\n",temp);
                    return 4;
                }
                if(temp<0)
                {
                    printf("Error : color values must be greater than zero. got %li\r\n",temp);
                    return 5;
                }
                c[i]=temp;
            }

            for(i=0;i<cust_list->num_colors;i++)
            {
                if(cust_list->alt_color[i].alt>height)
                {
                    break;
                }
            }
            for(j=cust_list->num_colors;j>i;j--)
            {
                //shift up colors after new color
                cust_list->alt_color[j]=cust_list->alt_color[j-1];
            }

            //set values
            cust_list->alt_color[i].color.r=c[0];
            cust_list->alt_color[i].color.g=c[1];
            cust_list->alt_color[i].color.b=c[2];
            cust_list->alt_color[i].color.brt=LED_brt;
            cust_list->alt_color[i].alt=height;
            //increment count
            cust_list->num_colors+=1;
            return 0;
        }
        else if(!strcmp("remove",argv[1]))
        {
            if(cust_list->num_colors<=0)
            {
                printf("Error : cant remove from an empty list\r\n");
                return 7;
            }
            if(argc<2)
            {
                printf("Error : \"remove\" requires one extra argument\r\n");
                return 6;
            }
            //parse value
            temp=strtol(argv[2],&eptr,0);

            //check if the whole string was parsed
            if(*eptr)
            {
                //end of string not found
                printf("Error while parsing \"%s\" unknown suffix \"%s\"\r\n",argv[2],eptr);
                return 2;
            }

            if(cust_list->num_colors>=0xFF)
            {
                printf("Error : removal index must be less than %i for current list. got %li\r\n",cust_list->num_colors,temp);
                return 4;
            }
            if(temp<0)
            {
                printf("Error : removal index must be greater than zero. got %li\r\n",temp);
                return 5;
            }

            //shrink list size
            cust_list->num_colors-=1;
            //shift entries down
            for(i=temp;i<cust_list->num_colors;i++)
            {
                cust_list->alt_color[i]=cust_list->alt_color[i+1];
            }

        }
        else
        {
            for(i=0;clists[i].name!=NULL;i++)
            {
                if(!strcmp(clists[i].name,argv[1]))
                {
                    list=clists[i].list;
                    break;
                }
            }
            if(list==NULL)
            {
                printf("Error : \"%s\" is not a valid color list\r\n",argv[1]);
                return 1;
            }
            flashPattern_setList(list);
        }
    }
    return 0;
}


int resets_Cmd(int argc,char **argv)
{
    printf("Number of resets = %u\r\n",nr_resets);
    printf("Reset reason : %s\r\n",reset_to_string(reset_reason));
    return 0;
}

void print_settings(const SETTINGS *set)
{
    int i;
    const char *pat_name;
    //print out color
    printf("color : 0x%02X 0x%02X 0x%02X 0x%02X\r\n",set->color.brt,set->color.r,set->color.g,set->color.b);

    //set pattern to NULL
    pat_name=NULL;
    //look for a matching pattern name
    for(i=0;pattern_names[i].name!=NULL;i++)
    {
        if(pattern_names[i].val==set->pattern)
        {
            //found, set name and exit loop
            pat_name=pattern_names[i].name;
            break;
        }
    }
    //check if we found a pattern name
    if(pat_name==NULL)
    {
        //no, print address
        printf("pattern : %i\r\n",set->pattern);
    }
    else
    {
        //yes, print name
        printf("pattern : %s\r\n",pat_name);
    }
    //print value
    printf("value : %u\r\n",set->value);

    pat_name=NULL;
    //look for a matching list name
    for(i=0;clists[i].name!=NULL;i++)
    {
        if(clists[i].list==set->list)
        {
            //found, set name and exit loop
            pat_name=clists[i].name;
            break;
        }
    }
    //check if we found a list name
    if(pat_name==NULL)
    {
        //no, print address
        printf("list : 0x%p\r\n");
    }
    else
    {
        //yes, print name
        printf("list  : %s\r\n",pat_name);
    }
    //print flight pattern
    printf("flight pattern : %s\r\n",set->flightp);
}

int settings_Cmd(int argc,char **argv)
{
    const SETTINGS *set=&settings;

    //check if we got arguments
    if(argc!=0)
    {
        //check what argument was given
        if(!strcmp(argv[1],"save"))
        {
            //write settings to flash
            write_settings();
            //reset flashpattern to reset timers and things
            flashPatternChange(settings.pattern);

        }
        else if(!strcmp(argv[1],"clear"))
        {
            //write settings to flash
            erase_settings();
        }
        else if(!strcmp(argv[1],"flash"))
        {
            set=&fl_settings.set;
            printf("Showing settings from flash\r\n");
        }
        else
        {
            //unknown argument
            printf("Error : unknown argument \'%s\'\r\n",argv[1]);
            return 1;
        }
    }

    //check if settings are valid
    if(!settings_valid())
    {
        //settings are not valid
        printf("Flash settings are invalid!!\r\n");
    }

    //print out settings
    print_settings(set);

    return 0;
}


int rst_Cmd(int argc,char **argv)
{
    //check number of arguments
    if(argc==1)
    {
        //check what sort of reset
        if(!strcmp("por",argv[1]))
        {
            //software power on reset
            PMMCTL0=PMMPW|PMMSWPOR;
            //code should never get here
        }
        else if(!strcmp("bor",argv[1]))
        {
            //software brown out reset
            PMMCTL0=PMMPW|PMMSWBOR;
            //code should never get here
        }
        else
        {
            printf("Error : unknown argument \'%s\'\r\n",argv[1]);
        }
    }
    else
    {
        printf("Error : exactly one argument is required\r\n");
    }
    return 1;
}

const CMD_SPEC cmd_tbl[]={
                          {"help","get help on commands",helpCmd},
                          {"LED","Change LED stuff",LED_Cmd},
                          {"brt","Change the LED brightness",brt_Cmd},
                          {"pat","Change/query LED pattern",patternCmd},
                          {"ADC","get values from the ADC",ADC_Cmd},
                          {"pnext","switch to the next LED flash pattern",pnext_Cmd},
                          {"value","Set pattern value",value_Cmd},
                          {"color","Set pattern color",color_Cmd},
                          {"NC","change nosecone LED mode",NC_Cmd},
                          {"chute","change chute LED",chute_Cmd},
                          {"sim","simulate a flight",sim_Cmd},
                          {"clist","set color list",clist_Cmd},
                          {"resets","Print number of resets",resets_Cmd},
                          {"settings","Print out settings",settings_Cmd},
                          {"rst","Reset LED microcontroller",rst_Cmd},
                          {"fpat","set flight pattern",fpat_Cmd},
                          {NULL,NULL,NULL}
};

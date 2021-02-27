/*
 * Nosecone.c
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include "Nosecone.h"

typedef struct {
    int mode;
    int val1,val2;
    int t1,t2;
    int count;
    int state;
    int dir;
} LED_STATE;

static LED_PARAMS pat_NC_stat;

static LED_STATE nosecone_status,chute_status;

enum{NC_DIR_UP=0,NC_DIR_DOWN};

static uint16_t NC_buf,chute_buf;

const char* const mode_names[]={"static","fade","flash","blip","pattern"};

void NC_print(LED_STATE *state)
{
    if(state->mode<NC_MODE_NUM)
    {
        printf("\t""Mode : %s\r\n",mode_names[state->mode]);
        if(state->mode==NC_MODE_PATTERN && pat_NC_stat.mode<NC_MODE_PATTERN)
        {
            printf("\t""Pattern Mode : %s\r\n",mode_names[pat_NC_stat.mode]);
        }
    }
    else
    {
        printf("\t""Mode : %i\r\n",state->mode);
    }
    printf("\t""val1 : %i\r\n",state->val1);
    printf("\t""val2 : %i\r\n",state->val2);
    printf("\t""t1 : %i\r\n",state->t1);
    printf("\t""t2 : %i\r\n",state->t2);
    printf("\t""count : %i\r\n",state->count);
    printf("\t""state : %i\r\n",state->state);
    printf("\t""dir : %i\r\n",state->dir);
}

void NC_debug(void)
{
    printf("Nosecone:\r\n");
    NC_print(&nosecone_status);
}

void chute_debug(void)
{
    printf("Chute:\r\n");
    NC_print(&chute_status);
}

void set_chute(uint16_t chute)
{
    if(chute==0)
    {
        //set output to out bit and set low
        TA0CCTL1=OUTMOD_0;
    }
    else
    {

        if(chute<NC_MAX_PWM/2)
        {
            //set chute period
            TA0CCR1=chute;
            //set output reset/set mode
            TA0CCTL1=OUTMOD_7;
        }
        else
        {
            //set nosecone buffer
            chute_buf=chute;
            //set interrupt,
            TA0CCTL1|=CCIE;
        }
    }
}


static inline void set_nosecone(uint16_t cone)
{
    if(cone==0)
    {
        //set output to out bit and set low
        TA0CCTL2=OUTMOD_0;
    }
    else
    {
        if(cone<NC_MAX_PWM/2)
        {
            //set chute period
            TA0CCR2=cone;
            //set output reset/set mode
            TA0CCTL2=OUTMOD_7;
        }
        else
        {

            //set nosecone buffer
            NC_buf=cone;
            //set interrupt,
            TA0CCTL2|=CCIE;
        }

    }
}


void init_Nosecone(void)
{

    //TA0.2 and TA0.1 output pins
    P2DIR |=BIT1|BIT2;
    P2SEL0|=BIT1|BIT2;


    //set timer period
    TA0CCR0=NC_MAX_PWM;

    //nosecone_mode(NC_MODE_FADE,0,200,10,60);
    nosecone_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
    //turn off the chute light
    chute_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);
    //init NC pattern mode
    nosecone_pattern_mode(NC_MODE_STATIC,0,NC_NA,NC_NA,NC_NA);

    //setup TA0 to run in up mode for PWM
    TA0CTL=TASSEL_2|ID_3|MC_1|TACLR|TAIE;

    //enable interrupts for CCR0
    //TA0CCTL0|=CCIE;
}

int nosecone_mode(int mode,int val1,int val2,int t1,int t2)
{
    int new_mode=mode;
    //put in static mode while mode is being set
    nosecone_status.mode=NC_MODE_STATIC;

    if(mode==NC_MODE_PATTERN)
    {
        //set values from pattern structure
        val1=pat_NC_stat.val1;
        val2=pat_NC_stat.val2;
        t1=pat_NC_stat.t1;
        t2=pat_NC_stat.t2;
        new_mode=pat_NC_stat.mode;
    }
    //mode based init
    switch(new_mode)
    {
    case NC_MODE_STATIC:
        //nothing to do here
        break;
    case NC_MODE_FADE:
        nosecone_status.dir=NC_DIR_UP;
        nosecone_status.count=0;
        break;
    case NC_MODE_FLASH:
        nosecone_status.state=-1;
        nosecone_status.count=0;
        break;
    case NC_MODE_ONE_SHOT:
        nosecone_status.state=0;
        nosecone_status.count=0;
        break;
    default:
        //unknown mode
        return 1;
    }
    //write values to structure
    nosecone_status.val1=val1;
    nosecone_status.val2=val2;
    nosecone_status.t1=t1;
    nosecone_status.t2=t2;
    //set mode
    nosecone_status.mode=mode;
    return 0;
}


int nosecone_pattern_mode(int mode,int val1,int val2,int t1,int t2)
{
    if(mode>NC_MODE_PATTERN)
    {
        //invalid mode
        return 1;
    }
    pat_NC_stat.val1=val1;
    pat_NC_stat.val2=val2;
    pat_NC_stat.t1=t1;
    pat_NC_stat.t2=t2;
    pat_NC_stat.mode=mode;
    if(nosecone_status.mode==NC_MODE_PATTERN)
    {
        //update mode
        nosecone_mode(nosecone_status.mode,NC_NA,NC_NA,NC_NA,NC_NA);
    }
    return 0;
}

int chute_mode(int mode,int val1,int val2,int t1,int t2)
{
    //put in static mode while mode is being set
    chute_status.mode=NC_MODE_STATIC;
    //mode based init
    switch(mode)
    {
    case NC_MODE_STATIC:
        //nothing to do here
        break;
    case NC_MODE_FADE:
        chute_status.dir=NC_DIR_UP;
        chute_status.count=0;
        break;
    case NC_MODE_FLASH:
        chute_status.state=-1;
        chute_status.count=0;
        break;
    case NC_MODE_ONE_SHOT:
        chute_status.state=0;
        chute_status.count=0;
        break;
    default:
        //unknown mode
        return 1;
    }
    //write values to structure
    chute_status.val1=val1;
    chute_status.val2=val2;
    chute_status.t1=t1;
    chute_status.t2=t2;
    //set mode
    chute_status.mode=mode;
    return 0;
}

static inline void update(LED_STATE *state, void (*update_fcn)(uint16_t) )
{
    int mode=state->mode;
    if(mode==NC_MODE_PATTERN)
    {
        //set mode from pattern mode
        //this won't happen for chute
        mode=pat_NC_stat.mode;
    }
    //increment count
    state->count+=1;
    switch(mode)
    {
    case NC_MODE_STATIC:
        update_fcn(state->val1);
        break;
    case NC_MODE_FADE:
        if(state->count>=state->t2)
        {
            state->count=0;
            if(state->dir==NC_DIR_UP)
            {
                //increment state
                state->state+=state->t1;
                //check if we have gone over val2
                if(state->state>=state->val2)
                {
                    state->state=state->val2;
                    state->dir=NC_DIR_DOWN;
                }
            }
            else
            {
                //Decrement state
                state->state-=state->t1;
                //check if we have gone below val1
                if(state->state<=state->val1)
                {
                    state->state=state->val1;
                    state->dir=NC_DIR_UP;
                }
            }
            //set new value
            update_fcn(state->state);
        }
        break;
    case NC_MODE_FLASH:
        if(state->state==0)
        {
            if(state->count>=state->t1)
            {
                state->count=0;
                state->state=1;
                update_fcn(state->val2);
            }
        }
        else if(state->state==-1)
        {
            //set initial value
            update_fcn(state->val1);
            state->state=0;
        }
        else
        {
            if(state->count>=state->t2)
            {
                state->count=0;
                state->state=0;
                update_fcn(state->val1);
            }
        }
        break;
    case NC_MODE_ONE_SHOT:
        if(state->count==1)
        {
            //set initial value
            update_fcn(state->val1);
        }
        if(state->state==0)
        {
            //check if we have timed out
            if(state->count>=state->t1)
            {
                //set new val
                update_fcn(state->val2);
                //disable further changes
                state->state=1;
            }
        }
        break;
    }
}

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void nosecone_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) nosecone_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(__even_in_range(TA0IV,TA0IV_TAIFG))
    {
    case TA0IV_TACCR1:
        //set chute period
        TA0CCR1=chute_buf;
        //set output reset/set mode
        //clear IE
        TA0CCTL1=OUTMOD_7;
        break;
    case TA0IV_TACCR2:
        //set nosecone period
        TA0CCR2=NC_buf;
        //set output reset/set mode
        //clear IE
        TA0CCTL2=OUTMOD_7;
        break;
    case TA0IV_TAIFG:
        //update chute light
        update(&chute_status,&set_chute);
        //update nosecone light
        update(&nosecone_status,&set_nosecone);
        break;
    }
}

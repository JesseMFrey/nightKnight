/*
 * flashPattern.h
 *
 *  Created on: Jan 6, 2019
 *      Author: jesse
 */

#ifndef FLASHPATTERN_H_
#define FLASHPATTERN_H_

#include "LEDs.h"


//maximum brightness 31
#define LED_BRT_DIM      (0x1)
#define LED_BRT_LOW      (4)
#define LED_BRT_MED      (8)
#define LED_BRT_NORM     (15)
#define LED_BRT_HIGH     (18)
#define LED_BRT_EXHIGH   (24)

//define standard colors                    brightness               | b  | g  | r  |
#define LED_COLOR_RED           ((LED_color){LED_ST_BITS|LED_BRT_NORM,0x00,0x00,0xFF})
#define LED_COLOR_GREEN         ((LED_color){LED_ST_BITS|LED_BRT_NORM,0x00,0xFF,0x00})
#define LED_COLOR_BLUE          ((LED_color){LED_ST_BITS|LED_BRT_NORM,0xFF,0x00,0x00})
#define LED_COLOR_YELLOW        ((LED_color){LED_ST_BITS|LED_BRT_NORM,0x00,0xFF,0xFF})
#define LED_COLOR_ORANGE        ((LED_color){LED_ST_BITS|LED_BRT_NORM,0x00,0xA5,0xFF})
#define LED_COLOR_PURPLE        ((LED_color){LED_ST_BITS|LED_BRT_NORM,0x80,0x00,0x80})
#define LED_COLOR_PINK          ((LED_color){LED_ST_BITS|LED_BRT_NORM,0xCB,0xC0,0xFF})


#define LED_COLOR_WHITE         ((LED_color){LED_ST_BITS|LED_BRT_NORM,0xFF,0xFF,0xFF})

typedef struct{
    float v,x;
    LED_color color;
}PARTICLE;

typedef struct
{
    int num_colors;
    struct
    {
        int alt;
        LED_color color;
    }alt_color[];
}COLOR_LIST;

void flashPatternChange(int pattern);
void flashPatternAdvance(void);
void HsvToLED(LED_color *dest,unsigned char brt,unsigned char hue,unsigned char saturation,unsigned char value);
void flashPatternNext(void);
int flashPatternGet(void);
void init_FlashPattern(void);
void flashPattern_setColor(LED_color color);
void flashPattern_setValue(unsigned int val);
void flashPattern_setList(const COLOR_LIST *list);
void flashPatternVC(int pattern,unsigned int val,LED_color color);
void panicPattern(void);

//flash patterns
enum{LED_PAT_OFF=0,LED_PAT_MAN,LED_PAT_COLORTRAIN,LED_PAT_HUE,LED_PAT_BURST,LED_PAT_SATURATION,
    LED_PAT_FLASH_GAP,LED_PAT_FLASH_NOGAP,LED_PAT_ST_LIST,LED_PAT_FLOW_LIST,LED_PAT_STR_ST,
    LED_PAT_BOOST,LED_PAT_GRAPH,LED_SOLID_ST,
    LED_PAT_EYES_H,
    //wave patterns
    LED_PAT_WAVE_BIG_U,LED_PAT_WAVE_BIG_D,LED_PAT_WAVE_SM_U,LED_PAT_WAVE_SM_D,LED_PAT_WAVE_HUE_D,LED_PAT_WAVE_HUE_U,LED_PAT_WAVE_SAT_D,LED_PAT_WAVE_SAT_U,
    //panic pattern
    LED_PAT_PANIC,
    //particle patterns
    LED_PAT_PARTICLE,LED_PAT_COLOR_PARTICLE,LED_PAT_UNIFORM_PARTICLE,LED_PAT_COLOR_UNIFORM_PARTICLE,LED_PAT_LIST_PARTICLE,LED_PAT_LIST_UNIFORM_PARTICLE,
    //end of list, get number of elements
    LED_NUM_PAT};

#define LED_PAT_MIN         LED_PAT_OFF
#define LED_PAT_MAX         (LED_NUM_PAT-1)

#endif /* FLASHPATTERN_H_ */

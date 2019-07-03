/*
 * flashPattern.h
 *
 *  Created on: Jan 6, 2019
 *      Author: jesse
 */

#ifndef FLASHPATTERN_H_
#define FLASHPATTERN_H_

#include "LEDs.h"

void flashPatternChange(int pattern);
void flashPatternAdvance(void);
void HsvToLED(LED_color *dest,unsigned char brt,unsigned char hue,unsigned char saturation,unsigned char value);
void flashPatternNext(void);
int flashPatternGet(void);
void init_FlashPattern(void);

//flash patterns
enum{LED_PAT_OFF=0,LED_PAT_ST_COLORS,LED_PAT_COLORTRAIN,LED_PAT_HUE,LED_PAT_BURST,LED_PAT_SATURATION,LED_NUM_PAT};

#define LED_PAT_MIN         LED_PAT_OFF
#define LED_PAT_MAX         (LED_NUM_PAT-1)

#endif /* FLASHPATTERN_H_ */

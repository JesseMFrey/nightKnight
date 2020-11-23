/*
 * settings.h
 *
 *  Created on: Nov 16, 2020
 *      Author: jesse
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "flashPattern.h"

#define SETTINGS_MAGIC  0xA5A3

typedef struct {
    LED_color color;
    int pattern;
    const COLOR_LIST *list;
    unsigned int value;
}SETTINGS;

typedef union{
    struct {
        unsigned short magic;
        SETTINGS set;
    };
    unsigned char dat[128];
} FL_SETTINGS;


extern const FL_SETTINGS fl_settings;

void write_settings(const SETTINGS *settings);
void init_settings(void);


#endif /* SETTINGS_H_ */

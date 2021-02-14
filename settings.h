/*
 * settings.h
 *
 *  Created on: Nov 16, 2020
 *      Author: jesse
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "flashPattern.h"
#include "flightPattern.h"

#define SETTINGS_MAGIC  0xA5A3

typedef struct {
    LED_color color;
    int pattern;
    const COLOR_LIST *list;
    unsigned int value;
    char flightp[FP_NAME_LEN];
}SETTINGS;

typedef union{
    struct {
        unsigned short magic;
        SETTINGS set;
    };
    unsigned char dat[128];
} FL_SETTINGS;


extern SETTINGS settings;
extern const FL_SETTINGS fl_settings;

void write_settings(void);
void init_settings(void);
int settings_valid(void);
void erase_settings(void);


#endif /* SETTINGS_H_ */

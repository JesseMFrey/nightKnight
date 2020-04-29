/*
 * flightPattern.h
 *
 *  Created on: Apr 25, 2020
 *      Author: jesse
 */

#ifndef FLIGHTPATTERN_H_
#define FLIGHTPATTERN_H_

#include "LEDs.h"
#include "flashPattern.h"
#include "Companion.h"

typedef void (*FLIGHT_UPDATE)(const struct ao_companion_command *cmd_dat,int new, void *data);

typedef struct{
    int type;
    union{
        struct{
            int pattern;
            int value;
            LED_color color;
        }basic;
        struct{
            FLIGHT_UPDATE callback;
            void *data;
        }cb;
    };
}FLIGHT_MODE_HANDLER;

typedef struct{
    const char *name;
    FLIGHT_MODE_HANDLER handlers[AO_FLIGHT_NUM];
}FLIGHT_PATTERN;

enum {FLIGHT_TYPE_INVALID=0,FLIGHT_TYPE_NO_CHANGE,FLIGHT_TYPE_PATTERN,FLIGHT_TYPE_PATTERN_VAL_COLOR,FLIGHT_TYPE_COLOR,FLIGHT_TYPE_PATTERN_COMPLEX};

//flight data for replay
extern const struct ao_companion_command flight_dat[];

//flight pattern data
extern const FLIGHT_PATTERN patterns[];

//process flight data
int proc_flightP(const struct ao_companion_command *new_dat,const FLIGHT_PATTERN *pattern,int last);

//functions used in flight modes
void fp_speedGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *data);

//sim interval in ms
extern unsigned int sim_int;

#endif /* FLIGHTPATTERN_H_ */

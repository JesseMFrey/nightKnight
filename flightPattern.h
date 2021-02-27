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
#include "Nosecone.h"

#define FP_NAME_LEN (16)

typedef void (*FLIGHT_UPDATE)(const struct ao_companion_command *cmd_dat,int new, void *data);

typedef struct{
    int type;
    union{
        struct{
            int pattern;
            int value;
            LED_color color;
            const COLOR_LIST *list;
        }basic;
        struct{
            FLIGHT_UPDATE callback;
            void *data;
        }cb;
    };
    LED_PARAMS NC_state,Chute_state;
}FLIGHT_MODE_HANDLER;

typedef struct{
    char name[FP_NAME_LEN];
    FLIGHT_MODE_HANDLER handlers[AO_FLIGHT_NUM];
}FLIGHT_PATTERN;

enum {FLIGHT_TYPE_INVALID=0,FLIGHT_TYPE_NO_CHANGE,FLIGHT_TYPE_PATTERN,FLIGHT_TYPE_PATTERN_VAL_COLOR,FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,FLIGHT_TYPE_COLOR,FLIGHT_TYPE_PATTERN_COMPLEX};

//flight data for replay
extern const struct ao_companion_command flight_dat[];

//flight pattern data
extern const FLIGHT_PATTERN flight_patterns[];

//find a flight pattern with a given name
int find_flightP(const char *name);

//process flight data
int proc_flightP(const struct ao_companion_command *new_dat,const FLIGHT_PATTERN *pattern,int last);

//functions used in flight modes
void fp_speedGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color);
void fp_heightGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color);
void fp_heightParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *color);
void fp_heightListParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *color);
void fp_colorHeight(const struct ao_companion_command *cmd_dat,int fm_new, void *list);

//sim interval in ms
extern unsigned int sim_int;

#endif /* FLIGHTPATTERN_H_ */

/*
 * events.h
 *
 *  Created on: Jul 1, 2019
 *      Author: jesse
 */

#ifndef EVENTS_H_
#define EVENTS_H_

enum{COMP_RX_CMD=1<<0,FM_SIM_ADVANCE=1<<1,FP_ADVANCE=1<<2};

typedef unsigned int e_type;

e_type e_get_clear(void);
void LPM0_check(void);

extern e_type e_flags;

#endif /* EVENTS_H_ */

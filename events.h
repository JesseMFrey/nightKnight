/*
 * events.h
 *
 *  Created on: Jul 1, 2019
 *      Author: jesse
 */

#ifndef EVENTS_H_
#define EVENTS_H_

enum{COMP_RX_CMD=0x01,FM_SIM_ADVANCE=0x02};

typedef unsigned int e_type;

e_type e_get_clear(void);

extern e_type e_flags;

#endif /* EVENTS_H_ */

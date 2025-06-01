/*
 * regulator.h
 *
 *  Created on: Feb 26, 2020
 *      Author: jesse
 */

#ifndef REGULATOR_H_
#define REGULATOR_H_

void init5Vreg(void);

void reg5V_on(void);

#define reg5V_off()     (P1OUT&=~BIT7)

#define reg5V_is_on()   (P1OUT&BIT7)

extern int reg_flags;

enum{REG_FLAGS_STARTUP=0x01,REG_FLAGS_ERROR=0x02,REG_FLAGS_DEGLITCH=0x04};

#define REG_STARTUP_TICKS        (800)        //about 24 ms
#define REG_DEGLITCH_TICKS       (660)        //about 20 ms

#endif /* REGULATOR_H_ */

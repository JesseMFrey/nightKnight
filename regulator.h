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

extern int reg_startup;

#endif /* REGULATOR_H_ */

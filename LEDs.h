/*
 * LEDs.h
 *
 *  Created on: Dec 27, 2018
 *      Author: jesse
 */

#ifndef LEDS_H_
#define LEDS_H_

void initLEDs(void);

#define NUM_LEDS    30

unsigned char LEDSendByte(const unsigned char data);

#endif /* LEDS_H_ */

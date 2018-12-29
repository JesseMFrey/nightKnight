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

typedef struct{
    unsigned char brt;
    unsigned char b,g,r;
}LED_color;

typedef struct{
    unsigned char header[4];
    LED_color colors[NUM_LEDS];
    unsigned char end[((NUM_LEDS-1)+8)/16];
}LED_array;

LED_array LED_stat[2];

unsigned char LEDSendByte(const unsigned char data);

void LEDs_send(LED_array *dat);

#endif /* LEDS_H_ */

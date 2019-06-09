/*
 * LEDs.h
 *
 *  Created on: Dec 27, 2018
 *      Author: jesse
 */

#ifndef LEDS_H_
#define LEDS_H_

void initLEDs(void);

//number of LEDs under the fins
#define FIN_LED     (11)

//number of LEDs on the outside of lower section
#define BOOST_LED   (6)

//number of LEDs on the outside of the upper section
#define UPR_LED     (33)

//lower section LED length
#define LWR_LED     (FIN_LED+BOOST_LED)

//11 under fins, 6 on lower section, 33 on upper section
#define LED_LEN     (FIN_LED+BOOST_LED+UPR_LED)


#define NUM_LEDS    (3*LED_LEN)

#define MAX_BRT     0x1F
#define LED_ST_BITS 0xE0

typedef struct{
    unsigned char brt;
    unsigned char b,g,r;
}LED_color;

typedef struct{
    unsigned char header[4];
    LED_color colors[NUM_LEDS];
    unsigned char end[4];
}LED_array;

LED_array LED_stat[2];

extern const int LED_lut[150][2];

unsigned char LEDSendByte(const unsigned char data);

void LEDs_send(LED_array *dat);

#endif /* LEDS_H_ */

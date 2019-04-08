/*
 * Companion.h
 *
 *  Created on: Apr 7, 2019
 *      Author: jesse
 */

#ifndef COMPANION_H_
#define COMPANION_H_

#include <stdint.h>

//from ALTOS ao_companion.h

#define AO_COMPANION_SETUP      1
#define AO_COMPANION_FETCH      2
#define AO_COMPANION_NOTIFY     3

struct ao_companion_command {
    uint8_t     command;
    uint8_t     flight_state;
    uint16_t    tick;
    uint16_t    serial;
    uint16_t    flight;
    int16_t     accel;
    int16_t     speed;
    int16_t     height;
    int16_t     motor_number;
};

struct ao_companion_setup {
    uint16_t    board_id;
    uint16_t    board_id_inverse;
    uint8_t     update_period;
    uint8_t     channels;
};

//from ALTOS ao_flight_state.h

enum ao_flight_state {
    ao_flight_startup = 0,
    ao_flight_idle = 1,
    ao_flight_pad = 2,
    ao_flight_boost = 3,
    ao_flight_fast = 4,
    ao_flight_coast = 5,
    ao_flight_drogue = 6,
    ao_flight_main = 7,
    ao_flight_landed = 8,
    ao_flight_invalid = 9,
    ao_flight_test = 10
};

typedef union{
    struct ao_companion_setup s;
    struct ao_companion_command c;
}COMPANION_BUF;

//from ao_host.h
#define AO_SEC_TO_TICKS(s)  ((s) * 100)

void init_Companion(void);

#endif /* COMPANION_H_ */

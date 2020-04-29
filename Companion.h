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
    ao_flight_test = 10,
    AO_FLIGHT_NUM
};

enum TLM_flags{TLM_ADC_OV=0x0001,TLM_PG=0x0002};

//structure for telemitry data
struct telemitry_dat {
    uint16_t flags;
    uint16_t Batt_V;
    uint16_t Batt_I;
    uint16_t LED_V;
    uint16_t LED_I;
    uint16_t MSP_V;
    uint16_t Temp;
};

#define TLM_ITEMS               (sizeof(struct telemitry_dat)/2)

//states for SPI
enum{CP_COMMAND_RX=0,CP_SETUP_TX,CP_TLM_TX};

enum{DMA_NO_INCR=0,DMA_INCR=1};

enum{DMA_NO_START=0,DMA_START=1};

#define AO_TICKS_TO_SEC(s)  ((s) / 100)

//from ao_host.h
#define AO_SEC_TO_TICKS(s)  ((s) * 100)

void init_Companion(void);

void companion_SPI_reset(void);

extern struct ao_companion_command cpCmd;

extern struct telemitry_dat cpTLM;

#endif /* COMPANION_H_ */

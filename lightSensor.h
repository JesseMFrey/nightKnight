/*
 * lightSensor.h
 *
 *  Created on: Sep 1, 2019
 *      Author: jesse
 */

#ifndef LIGHTSENSOR_H_
#define LIGHTSENSOR_H_

void init_light_sensor(void);
void light_sensor_start(void);
void light_sensor_stop(void);
unsigned short light_sensor_get(void);

#endif /* LIGHTSENSOR_H_ */

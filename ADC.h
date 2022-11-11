/*
 * ADC.h
 *
 *  Created on: Mar 12, 2020
 *      Author: jesse
 */

#ifndef ADC_H_
#define ADC_H_


void initADC(void);


#define adc2unit(val,scale,offset)  ((val)*(scale)*(1.2)/((float)((int)0x7FFF))+(offset))

#define unit2adc(val,scale,offset)  ( ((val)-(offset))*((float)((int)0x7FFF))/(1.2*(scale)) )

#define BATTERY_V_SCALE   (6.3667)
#define BATTERY_V_OFFSET  (0)
#define BATTERY_I_SCALE   (2)
#define BATTERY_I_OFFSET  (0)
#define LED_V_SCALE       (4.8802)
#define LED_V_OFFSET      (0)
#define LED_I_SCALE       (2)
#define LED_I_OFFSET      (0)
#define MSP_V_SCALE       (2.75)
#define MSP_V_OFFSET      (0)
#define REG_TEMP_SCALE    (51.282051)
#define REG_TEMP_OFFSET   (-20.641026)

#endif /* ADC_H_ */

/*
 * Nosecone.h
 *
 *  Created on: Apr 3, 2019
 *      Author: jesse
 */

#ifndef NOSECONE_H_
#define NOSECONE_H_

void init_Nosecone(void);
void set_nosecone(uint16_t cone);
void set_chute(uint16_t chute);


#define getNoseconeLED()        ((TA2CCTL2==(TA2CCTL2&OUTMOD_7)?TA2CCR2:0))
#define getChuteLED()           ((TA2CCTL1==(TA2CCTL1&OUTMOD_7)?TA2CCR1:0))

#define MAX_PWM     0XFFF

#endif /* NOSECONE_H_ */

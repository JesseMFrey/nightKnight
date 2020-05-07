/*
 * reset.h
 *
 *  Created on: May 6, 2020
 *      Author: jesse
 */

#ifndef RESET_H_
#define RESET_H_


extern unsigned int nr_resets;
extern unsigned int reset_reason;


enum{RST_SRC_INVALID=0,RST_SRC_BOR,RST_SRC_RSTNMI,RST_SRC_DOBOR,RST_SRC_LPM5WU,RST_SRC_SECYV,
    RST_SRC_SVSL,RST_SRC_SVSH,RST_SRC_SVML_OVP,RST_SRC_SVMH_OVP,RST_SRC_DOPOR,
    RST_SRC_WDTTO,RST_SRC_WDTKEY,RST_SRC_KEYV,RST_SRC_PERF,RST_SRC_PMMKEY,RST_SRC_UNKNOWN
};

void init_reset(void);
const char *reset_to_string(unsigned int reason);


#endif /* RESET_H_ */

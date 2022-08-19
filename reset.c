/*
 * reset.c
 *
 *  Created on: May 6, 2020
 *      Author: jesse
 */

#include "reset.h"
#include <msp430.h>

unsigned int __attribute__((noinit)) nr_resets;

unsigned int reset_reason;

int init_reset(void)
{
    int normal=0;
    switch(SYSRSTIV)
    {
        case SYSRSTIV_NONE:
			reset_reason = RST_SRC_UNKNOWN;
		break;
        case SYSRSTIV_BOR:
			reset_reason = RST_SRC_BOR;
			//clear number of resets
			nr_resets=0;
			normal=1;
		break;
        case SYSRSTIV_RSTNMI:
			reset_reason = RST_SRC_RSTNMI;
            //clear number of resets
            nr_resets=0;
            normal=1;
		break;
        case SYSRSTIV_DOBOR:
			reset_reason = RST_SRC_DOBOR;
			normal=1;
		break;
        case SYSRSTIV_LPM5WU:
			reset_reason = RST_SRC_LPM5WU;
		break;
        case SYSRSTIV_SECYV:
			reset_reason = RST_SRC_SECYV;
		break;
        case SYSRSTIV_SVSL:
			reset_reason = RST_SRC_SVSL;
		break;
        case SYSRSTIV_SVSH:
			reset_reason = RST_SRC_SVSH;
            //clear number of resets
            nr_resets=0;
            normal=1;
		break;
        case SYSRSTIV_SVML_OVP:
			reset_reason = RST_SRC_SVML_OVP;
		break;
        case SYSRSTIV_SVMH_OVP:
			reset_reason = RST_SRC_SVMH_OVP;
		break;
        case SYSRSTIV_DOPOR:
			reset_reason = RST_SRC_DOPOR;
            normal=1;
		break;
        case SYSRSTIV_WDTTO:
			reset_reason = RST_SRC_WDTTO;
		break;
        case SYSRSTIV_WDTKEY:
			reset_reason = RST_SRC_WDTKEY;
		break;
        case SYSRSTIV_KEYV:
			reset_reason = RST_SRC_KEYV;
		break;
        case SYSRSTIV_PERF:
			reset_reason = RST_SRC_PERF;
		break;
        case SYSRSTIV_PMMKEY:
			reset_reason = RST_SRC_PMMKEY;
		break;
        default:
            reset_reason = RST_SRC_INVALID;
        break;
    }
    //increment number of resets
    nr_resets++;
    //return true if reset was normal, false otherwise
    return normal;
}

const char *reset_to_string(unsigned int reason)
{
    switch(reason)
    {
    case RST_SRC_INVALID:
        return "Invalid";
    case RST_SRC_BOR:
        return "Brownout";
    case RST_SRC_RSTNMI:
        return "Reset Pin";
	case RST_SRC_DOBOR:
	    return "SW BOR";
	case RST_SRC_LPM5WU:
	    return "LPMx.5 wake up";
	case RST_SRC_SECYV:
	    return "Security violation";
	case RST_SRC_SVSL:
	    return "SVSL";
	case RST_SRC_SVSH:
	    return "SVSH";
	case RST_SRC_SVML_OVP:
	    return "SVML_OVP";
	case RST_SRC_SVMH_OVP:
	    return "SVMH_OVP";
	case RST_SRC_DOPOR:
	    return "Software POR";
	case RST_SRC_WDTTO:
	    return "Watchdog timeout";
	case RST_SRC_WDTKEY:
	    return "Watchdog password violation";
	case RST_SRC_KEYV:
	    return "Flash password violation";
	case RST_SRC_PERF:
	    return "Peripheral fetch";
	case RST_SRC_PMMKEY:
	    return "PMM password violation";
	case RST_SRC_UNKNOWN:
	    return "Unknown";
	default:
	    return "internal error";
    }
}


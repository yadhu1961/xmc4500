/*

    Program is written as part of assignment part A for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/


#ifndef __FUNC_DEF_H__
#define __FUNC_DEF_H__

#include <xmc_common.h>
#include "blinkenlights.h"

extern uint32_t prescaler;
extern uint32_t counter_value;
extern uint32_t def_ccu4_clock_freq; //This is the default value of clock frequency
extern double def_ccu4_clock_period; //Default value CCU4 clock period.
extern double led_period;


uint32_t power_val(int,int);
void calculate_prescaler();
void calculate_counter_value();


# endif /* FUNC_DEF_H_ */

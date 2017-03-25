/*

    Program is written as part of assignment part B for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/


#ifndef __FUNC_DEF_H__
#define __FUNC_DEF_H__

#include <xmc_common.h>
#include "blinkenlights.h"

enum LED_STATE
{
	NO_LED,
	LED1,
	LED2,
	NUM_LEDS
};

enum SWITCH_STATE
{
	SWITCH_BOTH = 0,
	SWITCH_TWO,
	SWITCH_ONE,
	SWITCH_NONE
};

 
extern uint32_t prescaler;
extern uint32_t counter_value;
extern uint32_t def_ccu4_clock_freq; 	//This is the default value of clock frequency
extern double def_ccu4_clock_period; 	//Default value CCU4 clock period.
extern double led_period;
extern double duty_cycle;
extern uint32_t switch_state;


uint32_t power_val(int,int);
void calculate_prescaler();
void calculate_counter_value();
void initButtons();
uint32_t get_switch_status();
void switch_off_LED1();
void switch_off_LED2();

# endif /* FUNC_DEF_H_ */
/*

    Program is written as part of assignment part A for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/

#include "func_def.h"

uint32_t prescaler = 0;
uint32_t counter_value = 0xFFFF;
uint32_t def_ccu4_clock_freq = 120000000; //This is the default value of clock frequency
double def_ccu4_clock_period = 1.0/120000000.0; //Default value CCU4 clock period.
double led_period =  1/LED_FREQ;


//Dynamically calculate the prescaler value
void calculate_prescaler()
{
	int max_prescaler = 15,i=0;
	double clock_period = def_ccu4_clock_period;
 	for(i = 0;i<max_prescaler;i++)
	{
		clock_period = power_val(2,prescaler)*def_ccu4_clock_period*65535.0;
		if(led_period > clock_period)
			prescaler++;
		else
			break;
	}
}

//Dynamically calculate the counter value
void calculate_counter_value()
{
	counter_value = (float)led_period/(float)(def_ccu4_clock_period*power_val(2,prescaler));
	counter_value = counter_value>0xFFFF?0xFFFF:counter_value;
}
 
 
//A small routine to calculate the power of value
uint32_t power_val(int base,int power)
{
	uint32_t result = 1;
	if(base < 0 || power < 0)
	{
	 //We will not worry about calculating the power of negative values, hence return zero.
		return -1;
	}
	if(power == 0)
		return result;
	else
	{
		while(power > 0)
		{
			result*=base;
			power--;
		}
	}
	return result;
 }
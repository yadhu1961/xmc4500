
/*

    Program is written as part of assignment part B for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/

#include "func_def.h"

//Definitions of the global variables
uint32_t prescaler = 0;
uint32_t counter_value = 0xFFFF;
uint32_t def_ccu4_clock_freq = 120000000;
double def_ccu4_clock_period = 1.0/120000000.0;
double led_period =  1/LED_FREQ;
double duty_cycle = ((LED_DUTY > 1)? 1:LED_DUTY);
uint32_t switch_state = SWITCH_NONE;

//Initializing buttons as outputs
void initButtons()
{
	static const uint8_t PU_INPUT = 0b00010;//Configuring as pull-up input
	//Configuring as push pull output
	PORT1->IOCR12 = (PORT1->IOCR12 & ~PORT1_IOCR12_PC14_Msk ) | ( PU_INPUT<<PORT1_IOCR12_PC14_Pos ); 
	PORT1->IOCR12 = (PORT1->IOCR12 & ~PORT1_IOCR12_PC15_Msk ) | ( PU_INPUT<<PORT1_IOCR12_PC15_Pos );
}

//Calculating the required value of the prescaler for the given prescaler
 void calculate_prescaler()
{
	int max_prescaler = 15;
	prescaler = 0;
	double clock_period = def_ccu4_clock_period;
 	for(int i = 0;i < max_prescaler;i++)
	{
		clock_period = power_val(2,prescaler)*def_ccu4_clock_period*65535.0;
		if(led_period > clock_period)
			prescaler++;
		else
			break;
	}
}

//Calculating the required value of the counter for the given counter
void calculate_counter_value()
{
	counter_value = 0xFFFF;
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

 
 
 //Getting status of the switches to control actions
 uint32_t get_switch_status()
 {	 
	uint32_t pre_state = SWITCH_NONE,prev_state = SWITCH_NONE;
	 bool is_both_buttons_pressed = false;
	 do
	 {
		pre_state = PORT1->IN & (PORT1_IN_P14_Msk | PORT1_IN_P15_Msk);
		pre_state = pre_state >> PORT1_IN_P14_Pos;
		
		if(pre_state == SWITCH_ONE || pre_state == SWITCH_TWO)
		{
			prev_state = pre_state;
		} else if(pre_state == SWITCH_BOTH)
		{
			prev_state = pre_state;
			is_both_buttons_pressed = true;
		} else
			break;
	 } while(1);
	 //Here giving more priority to both buttons pressed option
	 if(is_both_buttons_pressed)
		 return SWITCH_BOTH;
	 else
		 return prev_state;
 }
 
//Here making sure that the led is turned off after idle.
 void switch_off_LED1()
 {
 	 static const uint8_t PU_OUTPUT = 0b10000;
	 PORT1->IOCR0 = (PORT1->IOCR0 & ~PORT1_IOCR0_PC1_Msk ) | ( PU_OUTPUT<<PORT1_IOCR0_PC1_Pos );
	 PORT1->OUT = PORT1->OUT & ~PORT1_OUT_P1_Msk;
	 //connectLED1();
 }
 
 //Here making sure that the led is turned off after idle.
 void switch_off_LED2()
 {
 	static const uint8_t PU_OUTPUT = 0b10000;
	 PORT1->IOCR0 = (PORT1->IOCR0 & ~PORT1_IOCR0_PC0_Msk ) | ( PU_OUTPUT<<PORT1_IOCR0_PC0_Pos );
	 PORT1->OUT = PORT1->OUT & PORT1_OUT_P0_Msk;
 }

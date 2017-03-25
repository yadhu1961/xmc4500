/*

    Program is written as part of assignment part B for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/

#include <xmc_common.h>
#include "func_def.h"


void initCCU40_CC43(void);
void initCCU40_CC42(void);
void connectLED2(void);
void connectLED1 (void);

 int main(void) 
 {
	int present_led = NO_LED;
    
    //Initializing buttons as inputs
	initButtons();
    
    initCCU40_CC43();
	
    initCCU40_CC42();
	
    //Moving slice 3 to idle mode.
	CCU40->GIDLS |= CCU4_GIDLS_SS3I_Msk;
    
    //First case I am considering LED1.
	present_led = LED1;
	
    while(1)
	{
		switch_state = get_switch_status();
		switch(switch_state)
		{
			case SWITCH_BOTH: //When both the buttons are pressed
				if(present_led == LED2)
				{
                    //Connecting LED1 for CCU4 alternative function.
					connectLED1();
                    //Clearing the idle mode bit of Slice 2.
					CCU40->GIDLC |= CCU4_GIDLC_CS2I_Msk;
					//Moving slice 3 to idle mode.
					CCU40->GIDLS |= CCU4_GIDLS_SS3I_Msk;
					//Assigning new value to my LED state machine.
					present_led = LED1;
					//Applying the present duty cycle and period values to the LED1
					CCU40_CC42->PSC = prescaler;
					CCU40_CC42->CRS = (1-duty_cycle) * counter_value;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S2SE_Msk;
					switch_off_LED2();
				}
				else if(present_led == LED1)
				{
                    //Connecting LED2 for CCU4 alternative function.
					connectLED2();
                    //Clearing the idle mode bit of Slice 3.
					CCU40->GIDLC |= CCU4_GIDLC_CS3I_Msk;
                    //Moving slice 2 to idle mode.
					CCU40->GIDLS |= CCU4_GIDLS_SS2I_Msk;
                    //Assigning new value to my LED state machine.
					present_led = LED2;
                    //Applying the present duty cycle and period values to the LED2
					CCU40_CC43->PSC = prescaler;
					CCU40_CC43->CRS = (1-duty_cycle) * counter_value;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S3SE_Msk;
					switch_off_LED1();
				}
				break;
				
			case SWITCH_TWO: //When button two pressed
				if(present_led == LED2)
				{
                    //For increasing the frequency, decrementing the prescaler.
					prescaler--;
					CCU40_CC43->PSC = prescaler;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S3SE_Msk;
				} else if(present_led == LED1)
				{
					duty_cycle *= 2;
					duty_cycle = ((duty_cycle > 1)? 1:duty_cycle); // Bounding the duty cycle to 0 <= duty_cycle <= 1
					CCU40_CC42->CRS = (1-duty_cycle) * counter_value;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S2SE_Msk;
				}
				break;
			case SWITCH_ONE: //When Button one pressed
				if(present_led == LED2)
				{
                    //Increasing the prescaler for reducing the frequency
					prescaler++;
					CCU40_CC43->PSC = prescaler;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S3SE_Msk;
				} else if(present_led == LED1)
				{
					duty_cycle /= 2;
					CCU40_CC42->CRS = (1-duty_cycle) * counter_value ;
					/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
					CCU40->GCSS = CCU4_GCSS_S2SE_Msk;
				}
				break;
		}
	}
	return 0;
 }
 

 
void initCCU40_CC43() 
{
	/* Release CCU4 instance 0 from reset ( manual 22.6.1 step 2) */
	SCU_RESET->PRCLR0 = SCU_RESET_PRCLR0_CCU40RS_Msk ;
	/* Enable clock to CCUs ( manual 22.6.1 step 3) */
	SCU_CLK->CLKSET = SCU_CLK_CLKSET_CCUCEN_Msk ;
	//Calculating required prescaler value
	calculate_prescaler();
	////Calculating required counter value
	calculate_counter_value();
	//Yadhu Additions end here
	CCU40_CC43->PSC = prescaler;
	/* Enable prescaler in CCU4 instance 0 ( manual 22.6.1 step 4) */
	CCU40->GIDLC = CCU4_GIDLC_SPRB_Msk ;
	/* Configure period and compare in CCU4 instance 0 slice 3 ( manual 22.6.1 step 6) */
	CCU40_CC43->PRS = counter_value;
	CCU40_CC43->CRS = (1-duty_cycle) * counter_value ;
	/* Request shadow transfer for CCU4 instance 0 slice 3 for prescaler( manual 22.6.1 step 6) */
	CCU40->GCSS = CCU4_GCSS_S3SE_Msk;
	/* Connect LED1 to CCU40 . OUT3 */
	connectLED2();
	/* Enable timer slice 3 in CCU4 instance 0 ( manual 22.6.1 step 8) */
	CCU40->GIDLC = CCU4_GIDLC_CS3I_Msk ;
	/* Start slice 3 in CCU4 instance 0 by setting run bit ( manual 22.6.1 step 9) */
	CCU40_CC43->TCSET = CCU4_CC4_TCSET_TRBS_Msk ;
}
 
void initCCU40_CC42() 
{
	/* Release CCU4 instance 0 from reset ( manual 22.6.1 step 2) */
	SCU_RESET->PRCLR0 = SCU_RESET_PRCLR0_CCU40RS_Msk ;
	/* Enable clock to CCUs ( manual 22.6.1 step 3) */
	SCU_CLK->CLKSET = SCU_CLK_CLKSET_CCUCEN_Msk ;
	//Calculating prescaler here
	calculate_prescaler();
	//Calculating required counter value
	calculate_counter_value();
	//Yadhu Additions end here
	CCU40_CC42->PSC = prescaler;
	/* Enable prescaler in CCU4 instance 0 ( manual 22.6.1 step 4) */
	CCU40->GIDLC = CCU4_GIDLC_SPRB_Msk ;
	/* Configure period and compare in CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
	CCU40_CC42->PRS = counter_value;
	CCU40_CC42->CRS = (1-duty_cycle) * counter_value ;
	/* Request shadow transfer for CCU4 instance 0 slice 2 ( manual 22.6.1 step 6) */
	CCU40->GCSS = CCU4_GCSS_S2SE_Msk;
	/* Connect LED1 to CCU40 . OUT2 */
	connectLED1();
	/* Enable timer slice 2 in CCU4 instance 0 ( manual 22.6.1 step 8) */
	CCU40->GIDLC = CCU4_GIDLC_CS2I_Msk ;
	/* Start slice 3 in CCU4 instance 0 by setting run bit ( manual 22.6.1 step 9) */
	CCU40_CC42->TCSET = CCU4_CC4_TCSET_TRBS_Msk ;
}
 

 void connectLED2() 
 {
	/* Bit mask for alternate function 3 with push - pull output */
	static const uint8_t PP_ALT3 = 0b10011;
	PORT1->IOCR0 = (PORT1->IOCR0 & ~PORT1_IOCR0_PC0_Msk ) | ( PP_ALT3<<PORT1_IOCR0_PC0_Pos );
 }
  
 void connectLED1() 
 {
	/* Bit mask for alternate function 3 with push - pull output */
	static const uint8_t PP_ALT3 = 0b10011;
	PORT1->IOCR0 = (PORT1->IOCR0 & ~PORT1_IOCR0_PC1_Msk ) | ( PP_ALT3<<PORT1_IOCR0_PC1_Pos );
 }

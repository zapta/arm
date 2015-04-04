/*

 u8g_arm_pro_mini.cpp


 Implementation of ARM PRO MINI u8g driver.

 Universal 8bit Graphics Library

 Copyright (c) 2013, olikraus@gmail.com
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list
 of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// TODO: implement the SPI driver below using the standard LPC11Uxx API
// API from ssp_11xx.h. Currently workign directly against the 
// hardware.

#include "u8g_mbed.h"
//#include "system_time.h"
//#include "chip.h"
//#include "chip/gpio_11xx_1.h"
//#include "chip/iocon_11xx.h"
//#include "io_pins.h"

#include "mbed.h"

// 1 - use chip select pin. 0 - ignore chip select pin.
#define USE_CS 0

static SPI spi(P0_9, NC, P0_10); // mosi, miso, sclk

// These pins are used in additional to SPI0 clock and MOSI0.
static DigitalOut u8g_pin_rst(P0_18);  // Reset
static DigitalOut u8g_pin_a0(P0_19);  // D/C

// These pins are used in additional to SPI0 clock and MOSI0.
//static io_pins::OutputPin u8g_pin_rst(0, 18);  // Reset
//static io_pins::OutputPin u8g_pin_a0(0, 19);   // D/C

#if USE_CS
//static io_pins::OutputPin u8g_pin_cs(0, 16);   // Chip select
static DigitalOut u8g_pin_cs(P0_16);   // Chip select
#endif

// setup spi0
// ns is the clock cycle time between 0 and 20000
void spi_init(uint32_t ns) {

  // Setup the spi for 8 bit data, high steady state clock,
  // second edge capture, with a 1MHz clock rate
  spi.format(8,3);
  // TODO: compute actual speed.
  spi.frequency(1000000);


//  // SPI clock polarity [0, 1]. Specifies clock steady state between frame.
//  const uint32_t cpol = 1;
//
//  // SPI clock phase.
//  // 0: clock samples on change from steady state
//  // 1: clock samples on change to steady state.
//  const uint32_t cpha = 1;  // clock samples on return to stable state.
//
//  LPC_SYSCTL->PRESETCTRL |= 1 << 0; /* de-asserted reset SSP0 */
//  LPC_SYSCTL->SYSAHBCLKCTRL |= 1 << 16; /* enable IOCON clock */
//  LPC_SYSCTL->SYSAHBCLKCTRL |= 1 << 11; /* enable SSP0 clock  */
//  LPC_SYSCTL->SSP0CLKDIV = 1;
//
//  LPC_IOCON->PIO0[10] = 0x2;  // SCK0 at PIO0_10
//  LPC_IOCON->PIO0[9] = 0x1;   // MOSI0 at PIO0_9, MISO is not used.
//
//  LPC_SSP0->CR1 = 0; /* disable SPI, enable master mode */
//  LPC_SSP0->CR0 = 7 | (cpol << 6) | (cpha << 7); /* 8 bit, SPI mode, SCR = 1 (prescale) */
//  LPC_SSP0->CR0 = 7 | (cpol << 6) | (cpha << 7); /* 8 bit, SPI mode, SCR = 1 (prescale) */
//
//  // calculate CPSR
//  // SystemCoreClock / CPSR = 1000000000 / ns
//  // CPSR = SystemCoreClock * ns / 1000000000
//  // CPSR = (SystemCoreClock/10000) * ns / 100000
//  uint32_t cpsr = SystemCoreClock;  // 48000000
//  cpsr /= 10000UL;
//  cpsr *= ns;
//  cpsr += 100000UL - 1UL; /* round up */
//  cpsr /= 100000UL;
//  /* ensure that cpsr will be between 2 and 254 */
//  if (cpsr == 0)
//    cpsr = 1;
//  cpsr++;
//  cpsr &= 0x0feUL;
//  //cpsr = 2;
//  LPC_SSP0->CPSR = cpsr;
//  LPC_SSP0->CR1 = 2; /* enable SPI, (enable master mode) */
}

void spi_out(uint8_t data) {
  spi.write(data);
//  while ((LPC_SSP0->SR & 2) == 0) {
//  }
//  LPC_SSP0->DR = data;
}

void u8g_Delay(uint16_t ms) {
  wait_ms(ms);
//  system_time::delay_usec(1000UL * (uint32_t) ms);
}

void u8g_MicroDelay(void) {
  wait_us(1);
//  system_time::delay_usec(1);
//
}

void u8g_10MicroDelay(void) {
  wait_us(10);
//  system_time::delay_usec(10);
}

// U8G SPI abstrationc.
uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr) {

  switch (msg) {
  case U8G_COM_MSG_STOP:
    break;

  case U8G_COM_MSG_INIT:
    if (arg_val <= U8G_SPI_CLK_CYCLE_50NS) {
      spi_init(50);
    } else if (arg_val <= U8G_SPI_CLK_CYCLE_300NS) {
      spi_init(300);
    } else if (arg_val <= U8G_SPI_CLK_CYCLE_400NS) {
      spi_init(400);
    } else {
      spi_init(1200);
    }
    u8g_MicroDelay();
    break;

  case U8G_COM_MSG_ADDRESS:
    u8g_10MicroDelay();
    //u8g_pin_a0.set(arg_val);
    u8g_pin_a0.write(arg_val);

    u8g_10MicroDelay();
    break;

  case U8G_COM_MSG_CHIP_SELECT:
    // The Heltec SPI module we are using doesn't use chip select.
#if USE_CS
     if (arg_val == 0) {
       // disable
       uint8_t i;
       // this delay is required to avoid that the display is switched off
       // too early --> DOGS102 with LPC1114 */
       for (i = 0; i < 5; i++) {
         u8g_10MicroDelay();
       }
       u8g_pin_cs.set(true);
     } else {
       // enable
       u8g_pin_cs.set(false);
     }
     u8g_MicroDelay();
#endif
    break;

  case U8G_COM_MSG_RESET:
    //u8g_pin_rst.set(arg_val);
    u8g_pin_rst.write(arg_val);
    u8g_10MicroDelay();
    break;

  case U8G_COM_MSG_WRITE_BYTE:
    spi_out(arg_val);
    u8g_MicroDelay();
    break;

  case U8G_COM_MSG_WRITE_SEQ:
  case U8G_COM_MSG_WRITE_SEQ_P: {
    register uint8_t *ptr = (uint8_t*) arg_ptr;
    while (arg_val > 0) {
      spi_out(*ptr++);
      arg_val--;
    }
  }
    break;
  }
  return 1;
}


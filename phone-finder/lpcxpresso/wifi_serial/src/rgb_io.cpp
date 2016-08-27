#include "rgb_io.h"

#include "cmsis.h"
#include "pinmap.h"

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif

// RGB LED connections (active high)
//
// Red:   P0_8   PWM_1  CT16B0  MR0
// Green: P0_9   PWM_2  CT16B0  MR1
// Blue:  P0_22  PWM_5  CT16B1  MR1

#define TCR_OFF    0b00
#define TCR_EN     0b01
#define TCR_RESET  0b10

namespace rgb_io {

// The MSB byte is derived from the 8bit pwm value. Subtracting 1
// such that the min value of 0x00 which is translated to match
// count of 0xff00 is beyond the period size. If it's equal, we
// do get a ~0.2 usec pulses rather than constant low..
static const uint16_t kPeriodCounterTicks = 0xff00 - 1;

// PWM ~100 hz frequency. 0xff00 is the max count of the counters ((~0x00) << 8).
static const uint16_t kScalerRatio = (SystemCoreClock /  0xff00) / 100;

void initialize() {
  // Disable timers 0, 1
  LPC_CT16B0->TCR = TCR_OFF;
  LPC_CT16B1->TCR = TCR_OFF;

  //Power timers 0, 1
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 0);
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 1);

  // Enable PWM of timers 0, 1
  LPC_CT16B0->PWMC = 0b1111;
  LPC_CT16B1->PWMC = 0b1111;

  // Reset functionality on MR3 controlling the PWM period
  LPC_CT16B0->MCR = (1 << 10);
  LPC_CT16B1->MCR = (1 << 10);

  // TODO: should we set kScalerRatio - 1 ?
  LPC_CT16B0->PR = kScalerRatio;
  LPC_CT16B1->PR = kScalerRatio;

  LPC_CT16B0->TCR = TCR_RESET;
  LPC_CT16B1->TCR = TCR_RESET;

  LPC_CT16B0->MR3 = kPeriodCounterTicks;
  LPC_CT16B1->MR3 = kPeriodCounterTicks;

  // Set initial PWM off ticks.
  set(0, 0, 0);

  // Enable timers 0, 1
   LPC_CT16B0->TCR = TCR_EN;
   LPC_CT16B1->TCR = TCR_EN;

  // Pinout
  // TODO: define a const for the PWM pin function 2.
  pin_function(P0_8, 2);
  pin_mode(P0_8, PullNone);

  pin_function(P0_9, 2);
  pin_mode(P0_9, PullNone);

  pin_function(P0_22, 2);
  pin_mode(P0_22, PullNone);
}

void set(uint8_t r, uint8_t g, uint8_t b) {
  const uint16_t ticks_off_r = ((uint16_t)~r) << 8;
  const uint16_t ticks_off_g = ((uint16_t)~g) << 8;
  const uint16_t ticks_off_b = ((uint16_t)~b) << 8;

  LPC_CT16B0->MR0 = ticks_off_r;
  LPC_CT16B0->MR1 = ticks_off_g;
  LPC_CT16B1->MR1 = ticks_off_b;
}

}  // rgb_io


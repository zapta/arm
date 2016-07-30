#include "dtmf_io.h"

#include "cmsis.h"
#include "pinmap.h"

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif

//static const uint16_t kFreqLow1Hz = 697;
//static const uint16_t kFreqLow2Hz = 770;
//static const uint16_t kFreqLow3Hz = 852;
//static const uint16_t kFreqLow4Hz = 941;
//
//static const uint16_t kFreqHigh1Hz = 1209;
//static const uint16_t kFreqHigh2Hz = 1336;
//static const uint16_t kFreqHigh3Hz = 1477;

#define COUNT(f) ((const uint16_t)(SystemCoreClock / (f) / 2))

struct Code {
  const uint16_t timer0_count;
  const uint16_t timer1_count;
};

static const Code codes[] = {
    { COUNT(697), COUNT(1209) },
    { COUNT(697), COUNT(1336) },
    { COUNT(697), COUNT(1477) },
};



//
//static const uint16_t kFreqLow1Count = HZ_TO_COUNT(697);

//         1209Hz 1336Hz 1477Hz
//  697Hz    1      2       3
//  770Hz    4      5       6
//  852Hz    7      8       9
//  941Hz    *      0       #

// RGB LED connections (active high)
//
// Tone1 (low ):  P0_8   PWM_1  CT16B0  MR0
// Tone2 (high):  P0_22  PWM_4  CT16B1  MR1

#define TCR_OFF    0b00
#define TCR_EN     0b01
#define TCR_RESET  0b10

namespace dtmf_io {

// The MSB byte is derived from the 8bit pwm value. Subtracting 1
// such that the min value of 0x00 which is translated to match
// count of 0xff00 is beyond the period size. If it's equal, we
// do get a ~0.2 usec pulses rather than constant low..
//static const uint16_t kPeriodCounterTicks = 0xff00 - 1;

// PWM ~100 hz frequency. 0xff00 is the max count of the counters ((~0x00) << 8).

//static const uint16_t kScalerRatio = 1;

// Last rgb set in 0x00rrggbb format.
// Using an invalid value so the initialization setting will never match.
//static uint32_t last_rgb = 0xff000000;

void initialize() {
  // Disable timers 0, 1
  LPC_CT16B0->TCR = TCR_OFF;
  LPC_CT16B1->TCR = TCR_OFF;

  //Power timers 0, 1
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 0);
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 1);

  // Enable counter mode (non PWM) for timers 0, 1
  LPC_CT16B0->PWMC = 0b0000;
  LPC_CT16B1->PWMC = 0b0000;

  // Reset functionality on MR0 controlling the counter time period
  LPC_CT16B0->MCR = (1 << 1);  // reset counter 0 of MR0 match
  LPC_CT16B1->MCR = (1 << 4);  // reset counter 1 on MR1 match

  // Set prescalers to 1. Timer is incremented 'SystemCoreClock' times a second.
  LPC_CT16B0->PR = 0;
  LPC_CT16B1->PR = 0;

  // TODO: is this reset needed?
  LPC_CT16B0->TCR = TCR_RESET;
  LPC_CT16B1->TCR = TCR_RESET;

  // Half cycle period
  LPC_CT16B0->MR0 = COUNT(697);
  LPC_CT16B1->MR1 = COUNT(1477);

  // Toggle output on match
  LPC_CT16B0->EMR = (0b11 << 4);  // MR0 match
  LPC_CT16B1->EMR = (0b11 << 6);  // MR1 match

  // Set initial PWM off ticks. This also initializes last_rgb.
  // last_rgb never matches rgb in this call.
  //set(0x000000);

  // TODO: move this to end of function?
  // Enable timers 0, 1
   LPC_CT16B0->TCR = TCR_EN;
   LPC_CT16B1->TCR = TCR_EN;

  // Pinout
  // TODO: define a const for the PWM pin function 2.

  pin_function(P0_8, 2);  // CT16B0_MAT0
  pin_mode(P0_8, PullNone);

//  pin_function(P0_9, 2);
//  pin_mode(P0_9, PullNone);

  pin_function(P0_22, 2);  // CT16B1_MAT1
  pin_mode(P0_22, PullNone);
}

// rgb is in 0x00rrggbb format.
//void set(uint32_t rgb) {
//  // Do nothing if no change.
//  //
//  // NOTE: we compare the entire 32 bit value even though we care only
//  // about the last 24 bits.
//  if (rgb == last_rgb) {
//    return;
//  }
//
//  // Convert rgb to three independent match counter values.
//  const uint32_t n_rgb = ~rgb;
//  const uint16_t ticks_off_r = (n_rgb >> 8) & 0xff00;
//  const uint16_t ticks_off_g = (n_rgb & 0xff00);
//  const uint16_t ticks_off_b = (n_rgb << 8);
//
//  LPC_CT16B0->MR0 = ticks_off_r;
//  LPC_CT16B0->MR1 = ticks_off_g;
//  LPC_CT16B1->MR1 = ticks_off_b;
//
//  last_rgb = rgb;
//}

}  // dtmf_io


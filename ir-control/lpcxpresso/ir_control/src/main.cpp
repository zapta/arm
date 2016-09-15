// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include "mbed.h"
#include "USBSerial.h"

// For ir timer
#include "cmsis.h"
#include "pinmap.h"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 250;

// Timer for generating the delay between printed messages.
static Timer timer;

Ticker ticker;

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif

// P0_8   PWM_1  CT16B0  MR0

// Map tone frequency to timer half cycle count. Since we have enough timer.
// The -1 is from the timer specification.
#define COUNT(f) ((const uint16_t)((SystemCoreClock / (f) / 2) - 1))

#define TCR_OFF    0b00
#define TCR_EN     0b01
#define TCR_RESET  0b10

void ir_setup() {
  // Disable timers 0, 1
  LPC_CT16B0->TCR = TCR_OFF;
  //LPC_CT16B1->TCR = TCR_OFF;

  //Power timers 0, 1
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 0);
  //LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 1);

  // Enable counter mode (non PWM) for timers 0, 1
  LPC_CT16B0->PWMC = 0b0000;
  //LPC_CT16B1->PWMC = 0b0000;

  // Reset functionality on MR0 controlling the counter time period
  LPC_CT16B0->MCR = (1 << 1);  // Timer0: reset counter 0 o0 MR0 match
  //LPC_CT16B1->MCR = (1 << 4);  // Timer1: reset counter 1 on MR1 match

  // Set prescalers to 1. Timer is incremented 'SystemCoreClock' times a second.
  LPC_CT16B0->PR = 0;
  //LPC_CT16B1->PR = 0;

  // TODO: is this reset needed?
  LPC_CT16B0->TCR = TCR_RESET;
  //LPC_CT16B1->TCR = TCR_RESET;

  // Clear output on match (tone are off, keep outputs  low).
  LPC_CT16B0->EMR = (0b01 << 4);  // Timer0: output LOW on MR0 match
  //LPC_CT16B1->EMR = (0b10 << 6);  // Timer1: output HIGH on MR1 match

  // Set arbitrary cycle, just to have counter matches which sets
  // the outputs to desired values based on EMR setting.
  // Values don't matter much since we override latter when dialing
  // the tones.
  LPC_CT16B0->MR0 = COUNT(1000);
  //LPC_CT16B1->MR1 = COUNT(1300);

  LPC_CT16B0->TCR = TCR_EN;
  //LPC_CT16B1->TCR = TCR_EN;

  // Pinout
   // TODO: define a const for the PWM pin function 2.
   pin_function(P0_8, 2);  // CT16B0_MAT0
   pin_mode(P0_8, PullNone);

   //pin_function(P0_22, 2);  // CT16B1_MAT1
  // pin_mode(P0_22, PullNone);
 }

// dtmf_count is the timer count to set or 0 if to turn off.

//static inline void ir_off() {
//  // Handle the case of tone off
////  if (!is_on) {
//    // Force output LOW on MR0 match
//    LPC_CT16B0->EMR = (0b01 << 4);
////    return;
////  }
////
////  // Handle the case of an actual tone.
//////  LPC_CT16B0->TCR = TCR_RESET;
////  LPC_CT16B0->MR0 = COUNT(1000);   //dtmf_count;
////  // Toggle output on MR0 match.
////  LPC_CT16B0->EMR = (0b11 << 4);
//}


static inline void ir_on() {
  // Handle the case of tone off
//  if (!is_on) {
//    // Force output LOW on MR0 match
//    LPC_CT16B0->EMR = (0b01 << 4);
//    return;
//  }

  // Handle the case of an actual tone.
//  LPC_CT16B0->TCR = TCR_RESET;
  LPC_CT16B0->MR0 = COUNT(1000);   //dtmf_count;
  // Toggle output on MR0 match.
  LPC_CT16B0->EMR = (0b11 << 4);
}

static inline void ir_off() {
  // Handle the case of tone off
//  if (!is_on) {
    // Force output LOW on MR0 match
    LPC_CT16B0->EMR = (0b01 << 4);
//    return;
//  }
//
//  // Handle the case of an actual tone.
////  LPC_CT16B0->TCR = TCR_RESET;
//  LPC_CT16B0->MR0 = COUNT(1000);   //dtmf_count;
//  // Toggle output on MR0 match.
//  LPC_CT16B0->EMR = (0b11 << 4);
}

void ticker_func() {

}


// Time since program start.
//static Timer sys_time;

// Red LED is at GPIO0_20.
static DigitalOut led(P0_20, 0);

USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

//AnalogIn analog_in(P0_11);

static void setup() {
  timer.start();
  ir_setup();

  ticker.attach_us(&ticker_func, 600);

  ir_on();
  //sys_time.start();
}

volatile uint32_t i;

static void loop() {
  timer.reset();

  i = 0;
  while(i < 2166213L) {
    i++;
  }

  int32_t loop_time_usecs = timer.read_us();
  usb_serial.printf("Time, %u\r\n", loop_time_usecs);
  led = !led;
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

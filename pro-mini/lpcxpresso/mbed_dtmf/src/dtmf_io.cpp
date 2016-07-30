#include "dtmf_io.h"

#include "cmsis.h"
#include "pinmap.h"
#include "USBSerial.h"

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif

extern USBSerial usb_serial;

//static const uint16_t kFreqLow1Hz = 697;
//static const uint16_t kFreqLow2Hz = 770;
//static const uint16_t kFreqLow3Hz = 852;
//static const uint16_t kFreqLow4Hz = 941;
//
//static const uint16_t kFreqHigh1Hz = 1209;
//static const uint16_t kFreqHigh2Hz = 1336;
//static const uint16_t kFreqHigh3Hz = 1477;

// Map tone frequency to timer half cycle count. 0 indicates off.
#define COUNT(f) ((const uint16_t)(SystemCoreClock / (f) / 2))

struct DtmfCodeEntry {
  const char dtmf_ascii_code;
  const uint16_t timer0_count;
  const uint16_t timer1_count;
};

// Maps characters to low and high tone combinations. 0 indicates no tone.
// The first entry is also used for all unknown ascii codes.
static const DtmfCodeEntry kDtmfTable[] = {
    // Unknown code.
    { '?', COUNT(400), 0 },
    // Off
    { ' ', 0, 0 },
    // Decimal Digits.
    { '0', COUNT(941), COUNT(1336) },
    { '1', COUNT(697), COUNT(1209) },
    { '2', COUNT(697), COUNT(1336) },
    { '3', COUNT(697), COUNT(1477) },
    { '4', COUNT(770), COUNT(1209) },
    { '5', COUNT(770), COUNT(1336) },
    { '6', COUNT(770), COUNT(1477) },
    { '7', COUNT(852), COUNT(1209) },
    { '8', COUNT(852), COUNT(1336) },
    { '9', COUNT(852), COUNT(1477) },
    // Special codes.
    { '*', COUNT(941), COUNT(1209) },
    { '#', COUNT(941), COUNT(1477) },
    // Low tones only.
    { 'i', COUNT(697), 0 },
    { 'j', COUNT(770), 0 },
    { 'k', COUNT(852), 0 },
    // High tones only.
    { 'x', 0, COUNT(1209) },
    { 'y', 0, COUNT(1336) },
    { 'z', 0, COUNT(1447) },
};

// Number of table entries.
static const uint8_t kDtmfTableSize = sizeof(kDtmfTable)
    / sizeof(kDtmfTable[0]);

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

  // Half cycle period to max.
  //LPC_CT16B0->MR0 = COUNT(697);
  //LPC_CT16B1->MR1 = COUNT(1477);

  // Set longest cycle. Doesn't really matter since both channels
  // are not active yet.
  LPC_CT16B0->MR0 = 0xffff;
  LPC_CT16B1->MR1 = 0xffff;

  // Toggle output on match
  LPC_CT16B0->EMR = (0b11 << 4);  // MR0 match
  LPC_CT16B1->EMR = (0b11 << 6);  // MR1 match

  // Set initial PWM off ticks. This also initializes last_rgb.
  // last_rgb never matches rgb in this call.
  //set(0x000000);

  // TODO: move this to end of function?
  // Enable timers 0, 1
//   LPC_CT16B0->TCR = TCR_EN;
//   LPC_CT16B1->TCR = TCR_EN;

  // Pinout
  // TODO: define a const for the PWM pin function 2.

  pin_function(P0_8, 2);  // CT16B0_MAT0
  pin_mode(P0_8, PullNone);

//  pin_function(P0_9, 2);
//  pin_mode(P0_9, PullNone);

  pin_function(P0_22, 2);  // CT16B1_MAT1
  pin_mode(P0_22, PullNone);
}

// Return a pointer to the entry in dtmf table that matches the given
// dtmf ascii code. If not found, return a pointer to the first entry
// in the table.
static const DtmfCodeEntry* find_dtmf_code_entry(char dtmf_ascii_code) {
  for (int i = 0; i < kDtmfTableSize; i++) {
    const DtmfCodeEntry* const entry = &kDtmfTable[i];
    if (entry->dtmf_ascii_code == dtmf_ascii_code) {
      return entry;
    }
  }
  // If unknown code, return the first table entry.
  return &kDtmfTable[0];
}

// channel_index is 0 or 1.
// dtmf_count is the timer count to set or 0 if to turn off.
static void set_channel_tone(int channel_index, uint16_t dtmf_count) {
  //usb_serial.printf("xset: %d, %d\r\n", channel_index, dtmf_count);

  // Select the timer for this channel
  LPC_CTxxBx_Type* const timer = channel_index ? LPC_CT16B1 : LPC_CT16B0;

  // On timer0 we use MR0 and on timer1 we use MR1 so we need to select this
  // as well.
  __IO uint32_t* const timer_mr = channel_index ? &LPC_CT16B1->MR1 : &LPC_CT16B0->MR0;

  // Handle the case of an actual tone.
  if (dtmf_count) {
    timer->TCR = TCR_RESET;
    *timer_mr = dtmf_count;
    timer->TCR = TCR_EN;
    return;
  }

  // Handle the case of no tone. Turn the channel off.
  timer->TCR = TCR_OFF;
  timer->TCR = TCR_RESET;
  *timer_mr = 0xffff;
}

// See dtmf_io.h
void set_dtmf_code(char dtmf_ascii_code) {
  const DtmfCodeEntry* const dtmf_code_entry = find_dtmf_code_entry(
      dtmf_ascii_code);

  usb_serial.printf("\r\nDTMF: %c, %d, %d\r\n", dtmf_code_entry->dtmf_ascii_code,
      dtmf_code_entry->timer0_count, dtmf_code_entry->timer1_count);

  set_channel_tone(0, dtmf_code_entry->timer0_count);
  set_channel_tone(1, dtmf_code_entry->timer1_count);
}

}  // dtmf_io


// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include "mbed.h"
//#include "USBSerial.h"

#include <src/dialer/dialer.h>
// For testing only
#include <src/dialer/dtmf.h>

// This consts file is not checked in to keep the actual test phone
//  number private. It includes a definition like this:
// #define TEST_PHONE_NUMBER "18004377950"
#include "_phone_numbers.i"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 1000;

// Timer for generating the delay between printed messages.
static Timer timer;

// Time since program start.
static Timer sys_time;

// Red LED is at GPIO0_20.
DigitalOut led1(P0_20, 0);

//USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

AnalogIn analog_in(P0_11);

static void setup() {
  timer.start();
  sys_time.start();
  dialer::initialize();

  // For testing
  // dtmf::force_continuous_code('z');
}

static void loop() {
  dialer::loop();

  const uint32_t time_now_in_cycle_msecs = timer.read_ms();

  // Generates a blink at the beginning of each cycle.
  const bool led_state = time_now_in_cycle_msecs <= kCycleTimeMsecs / 50;
  led1 = led_state;
  //legacy_led = led_state;

  if (time_now_in_cycle_msecs >= kCycleTimeMsecs) {
    timer.reset();

    if (!dialer::is_call_in_progress()) {
       dialer::call(TEST_PHONE_NUMBER);
     }
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

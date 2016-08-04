// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include <src/dialer.h>
#include "mbed.h"
#include "USBSerial.h"

// This consts file is not checked in to keep the numbers private.
#include "_phone_numbers.i"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 3000;

// Timer for generating the delay between printed messages.
static Timer timer;

// Time since program start.
static Timer sys_time;

// Red LED is at GPIO0_20.
DigitalOut led1(P0_20, 0);

// Early versions of ARM PRO MINI had the led at GPIO0_7.
static DigitalOut legacy_led(P0_7, 0);

USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

AnalogIn analog_in(P0_11);

static void setup() {
  timer.start();
  sys_time.start();
  dialer::initialize();

  //dtmf::force_continuous_code('k');

}

static void loop() {
  dialer::loop();

  static int message_count = 0;
  const uint32_t time_now_in_cycle_msecs = timer.read_ms();

  // Generates a blink at the beginning of each cycle.
  const bool led_state = time_now_in_cycle_msecs <= kCycleTimeMsecs / 50;
  led1 = led_state;
  legacy_led = led_state;

  if (time_now_in_cycle_msecs >= kCycleTimeMsecs) {
    message_count++;
    timer.reset();

    if (!dialer::is_call_in_progress()) {
       dialer::call(TEST_PHONE_NUMBER);
     }

 //   dtmf::force_continuous_code('0');
//    if (dtmf::is_dialing_in_progress()) {
//      dtmf::start_dialing("0123456789");
//    }
  }


}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

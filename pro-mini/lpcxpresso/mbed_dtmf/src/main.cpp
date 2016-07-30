// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include <src/dtmf.h>
#include "mbed.h"
#include "USBSerial.h"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 3000;

// Timer for generating the delay between printed messages.
static Timer timer;

// Time since program start.
static Timer sys_time;

// Red LED is at GPIO0_20.
static DigitalOut led1(P0_20, 0);

// Early versions of ARM PRO MINI had the led at GPIO0_7.
DigitalOut legacy_led(P0_7, 0);

USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

AnalogIn analog_in(P0_11);

static void setup() {
  timer.start();
  sys_time.start();
  dtmf::initialize();
}

static void loop() {
  dtmf::loop();

  static int message_count = 0;
  const uint32_t time_now_in_cycle_msecs = timer.read_ms();

  // Generates a blink at the beginning of each cycle.
  const bool led_state = time_now_in_cycle_msecs <= kCycleTimeMsecs / 3;
  led1 = led_state;
  legacy_led = led_state;

  if (time_now_in_cycle_msecs >= kCycleTimeMsecs) {
    message_count++;
    timer.reset();

    if (dtmf::is_idle()) {
      dtmf::start_dialing("0123456789");
    }
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

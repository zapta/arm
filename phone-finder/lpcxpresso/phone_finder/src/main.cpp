// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include "mbed.h"
//#include "USBSerial.h"

#include <src/dialer/dialer.h>
// For testing only
#include <src/dialer/dtmf.h>
#include "util/system_memory.h"
#include "util/status_led.h"
#include "wifi/wifi.h"


// This consts file is not checked in to keep the actual test phone
//  number private. It includes a definition like this:
// #define PHONE_NUMBER1 "18004377950"
// #define PHONE_NUMBER2 "16501234567"
#include "_phone_numbers.i"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 1000;

// Timer for generating the delay between printed messages.
static Timer heatbeat_timer;

static void setup() {
  heatbeat_timer.start();

  dialer::initialize();
  wifi::initialize();

  // For testing
  // dtmf::force_continuous_code('z');
}

// Called periodically to dump state for debugging.
static void dumpInternalState() {
  wifi::dumpInternalState();
  //protocol_impl::dumpInternalState();
  PRINTF("MAIN mem=%u\n", system_memory::estimateStackFreeSize());
}

static void loop() {
  dialer::loop();
  wifi::polling();

  const uint32_t time_now_in_cycle_msecs = heatbeat_timer.read_ms();

  // Generates a blink at the beginning of each cycle.
  const bool led_state = time_now_in_cycle_msecs <= kCycleTimeMsecs / 50;
  status_led::led_pin = led_state;

  if (time_now_in_cycle_msecs >= kCycleTimeMsecs) {
    heatbeat_timer.reset();
    PRINTF("\n");
    dumpInternalState();
    return;
  }

  if (!dialer::is_call_in_progress()) {
    const int pressed_buttons = wifi::getPressedButtonSet();
    if (pressed_buttons & (1 << 1)) {
       dialer::call(PHONE_NUMBER1);
       return;
    }
    if (pressed_buttons & (1 << 2)) {
       dialer::call(PHONE_NUMBER2);
       return;
    }
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

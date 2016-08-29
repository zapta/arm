// A simple mbed program for Arm Pro Mini.
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include <phone/dialer.h>
#include <phone/dtmf.h>
#include "mbed.h"

#include "util/system_memory.h"
#include "wifi/wifi.h"


// This consts file is not checked in to keep the actual test phone
//  number private. It includes a definition like this:
// #define PHONE_NUMBER1 "18004377950"
// #define PHONE_NUMBER2 "16501234567"
#include "_phone_numbers.i"

// LED, active high.
static DigitalOut led(P0_20, 0);

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

  const uint32_t time_now_in_heatbeat_cycle_msecs = heatbeat_timer.read_ms();

  led = (dialer::is_call_in_progress())
      ? dialer::led_control()
      : time_now_in_heatbeat_cycle_msecs <= kCycleTimeMsecs / 100;

  if (time_now_in_heatbeat_cycle_msecs >= kCycleTimeMsecs) {
    heatbeat_timer.reset();
    PRINTF("\n");
    dumpInternalState();
    return;
  }

  // If not active and a button is pressed than dial that number.
  if (!dialer::is_call_in_progress()) {
    const int pressed_buttons = wifi::getPressedButtonSet();
    // Number 1
    if (pressed_buttons & (1 << 1)) {
       dialer::call(PHONE_NUMBER1);
       return;
    }
    // Number 2
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

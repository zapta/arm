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
// #define TEST_PHONE_NUMBER "18004377950"
#include "_phone_numbers.i"

// LED blink cycle.
static const uint32_t kCycleTimeMsecs = 1000;

// Timer for generating the delay between printed messages.
static Timer heatbeat_timer;

// Time since program start.
//static Timer sys_time;

// Red LED is at GPIO0_20.
//DigitalOut led1(P0_20, 0);

//USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

//AnalogIn analog_in(P0_11);

// Time since last state change.
static Timer time_in_current_state;

enum State {
  POWER_UP,
  DOWN,
  UP,
  ERROR,
};

static State state = POWER_UP;

// Return display name for given state.
static const char* stateName(State state) {
  switch (state) {
    case POWER_UP:
      return "POWERED";
    case DOWN:
      return "DOWN";
    case UP:
      return "UP";
    case ERROR:
      return "ERROR";
    default:
      return "???";
  }
}

//static void setState(State new_state) {
//  DEBUGF("main %s -> %s\n", stateName(state), stateName(new_state));
//  if (new_state != state) {
//    state = new_state;
//    time_in_current_state.reset();
//  }
//}

static void setup() {
  heatbeat_timer.start();
  time_in_current_state.start();
 // sys_time.start();

  dialer::initialize();
  wifi::initialize();
  //protocol_impl::initialize();


  // For testing
  // dtmf::force_continuous_code('z');
}

// Called periodically to dump state for debugging.
static void dumpInternalState() {
  wifi::dumpInternalState();
  //protocol_impl::dumpInternalState();
  PRINTF("MAIN s=%s t=%u fm=%u\n", stateName(state),
      time_in_current_state.read_ms()/1000, system_memory::estimateStackFreeSize());
}

static void loop() {
  dialer::loop();
  wifi::polling();

  const uint32_t time_now_in_cycle_msecs = heatbeat_timer.read_ms();

  // Generates a blink at the beginning of each cycle.
  const bool led_state = time_now_in_cycle_msecs <= kCycleTimeMsecs / 50;
  status_led::led_pin = led_state;
  //legacy_led = led_state;

  if (time_now_in_cycle_msecs >= kCycleTimeMsecs) {
    heatbeat_timer.reset();
    PRINTF("\n");
    dumpInternalState();
    return;
//    if (!dialer::is_call_in_progress()) {
//       dialer::call(TEST_PHONE_NUMBER);
//     }
  }

  if (!dialer::is_call_in_progress() && wifi::getPressedButtonSet()) {
    dialer::call(TEST_PHONE_NUMBER);
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}

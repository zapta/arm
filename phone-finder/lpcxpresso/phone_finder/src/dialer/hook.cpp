#include "dialer/hook.h"

//#include "USBSerial.h"
#include "util/common.h"

//extern USBSerial usb_serial;

//extern DigitalOut led1;

namespace hook {

// Phone hook. 1 = line active.
static DigitalOut hook(P0_12, 0);

// Max time in millis from activating the hook until we can start dialing.
const int kHookOnStableTimeMillis = 1000;

// Max time in millis from deactivating the hook until it can be activated
// again for the next call.
const int kHookOffStableTimeMillis = 1000;

enum State {
  OFF,
  OFF_STABLE,
  ON,
  ON_STABLE
};

static State state = OFF;

// Time in current state. Not valid in stable states since they
// can last long time and experience timer overflow.
static Timer time_in_state;

static void change_state(State new_state) {
  state = new_state;
  time_in_state.reset();
  hook = (state == ON || state == ON_STABLE);
}

// See hook.h
void initialize() {
  time_in_state.start();
  change_state(OFF);
}

// Call from main loop.
void loop() {
  switch (state) {
  case OFF:
    if (time_in_state.read_ms() >= kHookOnStableTimeMillis) {
      change_state(OFF_STABLE);
    }
    break;

  case OFF_STABLE:
    // Do nothing.
    break;

  case ON:
    if (time_in_state.read_ms() >= kHookOnStableTimeMillis) {
      change_state(ON_STABLE);
    }
    break;

  case ON_STABLE:
    // Do nothing.
    break;

  default:
    PRINTF("hook error: %d\r\n", state);
  }
}

// See hook.h
bool is_hook_stable_on() {
  return state == ON_STABLE;
}

// See hook.h
bool is_hook_stable_off() {
  return state == OFF_STABLE;
}

void set_hook(bool is_on) {
  // Turn on
  if (is_on) {
    if (state == OFF || state == OFF_STABLE) {
      change_state(ON);
    }
    return;
  }

  // Turn off
  if (state == ON || state == ON_STABLE) {
    change_state(OFF);
  }
}

}  // hook


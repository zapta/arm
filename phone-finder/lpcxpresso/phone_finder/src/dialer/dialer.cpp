#include "dialer/dialer.h"

//#include "USBSerial.h"
#include "util/common.h"

#include "dialer/dtmf.h"
#include "dialer/hook.h"

//extern USBSerial usb_serial;

namespace dialer {

// NOTE: my cell phone stop ringing after ~25 secs.
const int kCallHoldTimeMillis   = 25000;

enum State {
  //
  IDLE,
  //
  ACTIVATE,
  //
  DIALING,
  //
  CALL_DELAY,
  //
  DEACTIVATE
};

static State state = DEACTIVATE;

// For debugging.
static const char* state_name(State state) {
  switch (state) {
    case IDLE: return "IDLE";
    case ACTIVATE: return "ACTIVATE";
    case DIALING: return "DIALING";
    case CALL_DELAY: return "CALL_DELAY";
    case DEACTIVATE: return "DEACTIVATE";
    default: return "???";
  }
}

// Ignored in IDLE state.
static const char* number_to_dial = "";

// Time in current state. Invalid in IDLE state due to possible
// timer overflow.
static Timer time_in_state;

// See dtmf.h
void initialize() {
  time_in_state.start();
  dtmf::initialize();
  hook::initialize();
}

static void change_state(State new_state) {
  PRINTF("%s -> %s\r\n", state_name(state), state_name(new_state));
  time_in_state.reset();
  state = new_state;
}

// See dtmf.h
void loop() {
  dtmf::loop();
  hook::loop();

  switch (state) {
    case IDLE:
      hook::set_hook(false);
      return;
      break;

    case ACTIVATE:
      hook::set_hook(true);
      if (hook::is_hook_stable_on()) {
        change_state(DIALING);
        dtmf::start_dialing(number_to_dial);
      }
      break;

    case DIALING:
      if (!dtmf::is_dialing_in_progress()) {
        change_state(CALL_DELAY);
      }
      break;

    case CALL_DELAY:
      if (time_in_state.read_ms() > kCallHoldTimeMillis) {
        change_state(DEACTIVATE);
      }
      break;

    case DEACTIVATE:
      hook::set_hook(false);
      if (hook::is_hook_stable_off()) {
        change_state(IDLE);
      }
      return;

    default:
      PRINTF("Unknown dialer state: %d\r\n", state);
      change_state(IDLE);
  }
}

// See dialer.h.
void call(const char* number) {
  if (state != IDLE) {
    PRINTF("A call already in progress\r\n");
    return;
  }
  number_to_dial = number;
  change_state(ACTIVATE);
}

// See dialer.h.
bool is_call_in_progress() {
  return state != IDLE;
}

}  // dialer

